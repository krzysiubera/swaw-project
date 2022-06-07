#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <kernel.h>
#include <sys/ring_buffer.h>
#include <drivers/sensor.h>
#include <drivers/uart.h>
#include <drivers/counter.h>
#include <drivers/console/uart_pipe.h>
#include <storage/disk_access.h>
#include <fs/fs.h>
#include <ff.h>
#include "rtc_app.h"
#include "bme280_app.h"
#include <logging/log.h>
#include "console_cmds.h"
#include "disk_cmds.h"
LOG_MODULE_REGISTER(main);

#define DISK_WRITE_REQ		(0x0101)
#define DISK_READ_REQ		(0x0100)
#define DISK_DMA_READ_REQ	(0x0110)

#define CONSOLE_BUF_SIZE	64

/* file system ops */
#define DISK_NAME	"SD"
#define FATFS_MOUNT_PT	"/SD:"

/* fat_fs data */
static FATFS fat_fs;
/* mounting info */
static struct fs_mount_t mp = {
	.type = FS_FATFS,
	.fs_data = &fat_fs,
	.mnt_point = FATFS_MOUNT_PT, 
};

static struct k_pipe fs_ops_path_pipe;

/* ring buffer */
#define DATA_BUF_SIZE		(1024)
static uint8_t bme280_data_buf[DATA_BUF_SIZE];
static struct ring_buf bme280_data_ring_buf;

/* message queue */
static struct bme280_state data_msgq_buf[4]__attribute__((aligned(4)));
static struct k_msgq data_msgq;

/* k_poll_signal object */
static struct k_poll_signal disk_sig;

/* pipe uart buffer */
static uint8_t console_rx_buf[CONSOLE_BUF_SIZE];
static uint8_t *console_rx_handler(uint8_t *buf, size_t *off); 
static void console_worker(struct k_work *item);

struct console_context {
	const struct device *dev;
	struct k_work wq_item;
	uint8_t * const rx_buf;
	const uint8_t rx_buf_size;
	uint8_t rx_buf_len;
	uint8_t cont_flag;
};

static struct console_context console_ctx = {
	//.dev		= DEVICE_DT_GET(DT_NODELABEL(usart2)),
	.dev		= DEVICE_DT_GET(DT_CHOSEN(zephyr_console)),
	.rx_buf		= console_rx_buf,
	.rx_buf_size	= CONSOLE_BUF_SIZE,
	.rx_buf_len	= 0,
};

static struct bme280_context bme280_ctx;

void bme280_worker(struct k_work *item);

void disk_worker(void *a, void *b, void *c);

/*
 *Define and initialize threads.
 */
K_THREAD_STACK_DEFINE(disk_stack, 4096);
static struct k_thread disk_thr;

static int disk_init(const char *pdrv);

void rtc_alarm_handler(const struct device *dev, uint8_t chan_id,
				uint32_t ticks, void *user_data);

static struct counter_alarm_cfg rtc_alarm_cfg = {
	.callback	= rtc_alarm_handler,
	.ticks		= 2U,
	.user_data	= (struct counter_alarm_cfg*) &rtc_alarm_cfg,
	.flags		= 0,
};

const struct device *rtc = NULL;

void main(void)
{	
	if (rtc_get_device(&rtc)) {
		for (;;);
	}

	if (bme280_get_device(&bme280_ctx.dev)) {
		for (;;);
	}

	if (disk_init(DISK_NAME)) {
		for (;;);
	}

	if (fs_mount(&mp)) {
		for (;;);
	}

	uart_pipe_register(console_rx_buf, CONSOLE_BUF_SIZE, console_rx_handler);

	k_msgq_init(&data_msgq, (char *)data_msgq_buf, sizeof(*data_msgq_buf), 4);
	//k_pipe_init(&fs_ops_path_pipe, fs_ops_path_buf, CONSOLE_BUF_SIZE);
	k_pipe_init(&fs_ops_path_pipe, NULL, 0);

	k_poll_signal_init(&disk_sig);

	ring_buf_init(&bme280_data_ring_buf, DATA_BUF_SIZE, bme280_data_buf); 

	k_work_init(&bme280_ctx.wq_item, bme280_worker);
	k_work_init(&console_ctx.wq_item, console_worker);

	k_thread_create(&disk_thr, disk_stack, 4096,disk_worker, NULL, NULL,
						NULL, -2, 0, K_NO_WAIT); 

	while (1) {
	
	}
}

static int lsdir(const char *path)
{
	int res;
	struct fs_dir_t dirp;
	static struct fs_dirent entry;

	fs_dir_t_init(&dirp);

	/* Verify fs_opendir() */
	res = fs_opendir(&dirp, path);
	if (res) {
		printk("Error opening dir %s [%d]\n", path, res);
		return res;
	}

	printk("\nListing dir %s ...\n", path);
	for (;;) {
		/* Verify fs_readdir() */
		res = fs_readdir(&dirp, &entry);

		/* entry.name[0] == 0 means end-of-dir */
		if (res || entry.name[0] == 0) {
			break;
		}

		if (entry.type == FS_DIR_ENTRY_DIR) {
			printk("[DIR ] %s\n", entry.name);
		} else {
			printk("[FILE] %s (size = %zu)\n",
				entry.name, entry.size);
		}
	}

	/* Verify fs_closedir() */
	fs_closedir(&dirp);

	return res;
}
void disk_worker(void *a, void *b, void *c)
{
	int status = 10;
	size_t bytes_read = 10;
	unsigned int msgs_num;
	struct k_poll_event disk_events[1] = {
		K_POLL_EVENT_INITIALIZER(K_POLL_TYPE_SIGNAL,
					K_POLL_MODE_NOTIFY_ONLY,
					&disk_sig),
	};
	struct bme280_state curr_sensor_data[4];
	const char read_req_info[] = "disk_worker: READ_REQ\n";
	static char fs_ops_buf[CONSOLE_BUF_SIZE]; 

	/* allow only one entry to be opened */
	struct fs_dir_t dir;
	struct fs_file_t file;

	for (;;) {
		k_poll(disk_events, 1, K_FOREVER);

		status = k_pipe_get(&fs_ops_path_pipe, fs_ops_buf, console_ctx.rx_buf_len - 2, &bytes_read,
								console_ctx.rx_buf_len - 2, K_FOREVER);

		switch (disk_events[0].signal->result) {
		case CREATE_DIR:
			printk("CREATE_DIR\n");
			status = fs_mkdir((const char*) fs_ops_buf);	
			if (status) {
				printk("fs_mkdir() fail: %d\n", status);
				break;
			}
		case OPEN_DIR:
			fs_dir_t_init(&dir);
			printk("init ok\n");
			status = fs_opendir(&dir, fs_ops_buf);
			if (status) {
				printk("fs_opendir() fail: %d\n", status);
				break;
			}
			
			printk("opendir ok\n");
			status = lsdir(fs_ops_buf);
			if (status) {
				printk("lsdir() fail: %d\n", status);
				break;
			}
			printk("lsdir ok\n");
			break;
		case CLOSE_DIR:
			status = fs_closedir(&dir);
			if (status) {
				printk("fs_closedir() fail: %d\n", status);
			}
			break;
		case FILE_WRITE:
			msgs_num = k_msgq_num_used_get(&data_msgq);
			for (unsigned int i = 0; i < msgs_num; ++i) {
				k_msgq_get(&data_msgq, &curr_sensor_data[i], K_FOREVER); 
			}
			break;
		case FILE_READ:
			uart_pipe_send((const uint8_t*) read_req_info,
						sizeof(read_req_info));
			break;
		default:
			break;
		}
		
		/* reset the disk_events state */
		disk_events[0].signal->signaled = 0;
		disk_events[0].state = K_POLL_STATE_NOT_READY;
	}
}

static int disk_init(const char *pdrv)
{
	uint32_t block_count;
	uint32_t block_size;
	uint64_t memory_size_mb;

	if (disk_access_init(pdrv) != 0) {
		LOG_ERR("Storage init ERROR!");
		return -ENODEV;
	}

	if (disk_access_ioctl(pdrv, DISK_IOCTL_GET_SECTOR_COUNT,
						&block_count)) {
		LOG_ERR("Unable to get sector count");
		return -ENODEV;
	}
	LOG_INF("Block count %u", block_count);

	if (disk_access_ioctl(pdrv, DISK_IOCTL_GET_SECTOR_SIZE,
						&block_size)) {
		LOG_ERR("Unable to get sector size");
		return -ENODEV;
	}
	printk("Sector size %u\n", block_size);

	memory_size_mb = (uint64_t)block_count * block_size;
	printk("Memory Size(MB) %u\n", (uint32_t)(memory_size_mb >> 20));

	return 0;
}

void rtc_alarm_handler(const struct device *dev, uint8_t chan_id,
					uint32_t ticks, void *user_data)
{
	int status;
	const struct counter_alarm_cfg *cfg;
	
	cfg = user_data;
	bme280_ctx.state.timestamp = ticks;

	status = k_work_submit(&bme280_ctx.wq_item);
	if (status < 0) {
		LOG_ERR("submission to workqueue failed: %d", status);
		return;
	}

	status = counter_set_channel_alarm(dev, 0, cfg);
	if (status) {
		LOG_ERR("Unable to set channel alarm: %d", status);	
		return;
	}
}

void bme280_worker(struct k_work *item)
{
	struct bme280_context *ctx;

	ctx = CONTAINER_OF(item, struct bme280_context, wq_item);

	sensor_sample_fetch(ctx->dev);
	sensor_channel_get(ctx->dev, SENSOR_CHAN_AMBIENT_TEMP, &ctx->state.temp);
	sensor_channel_get(ctx->dev, SENSOR_CHAN_PRESS, &ctx->state.press);
	sensor_channel_get(ctx->dev, SENSOR_CHAN_HUMIDITY, &ctx->state.humidity);
	sensor_channel_get(ctx->dev, SENSOR_CHAN_GAS_RES, &ctx->state.gas);

	if (k_msgq_put(&data_msgq, &ctx->state, K_NO_WAIT)) {
	//	k_msgq_purge(&data_msgq);
	}

	k_poll_signal_raise(&disk_sig, FILE_WRITE);

	if (console_ctx.cont_flag) {
		printk("[timestamp: %u] temp: %d.%06d; press: %d.%06d; humidity: %d.%06d;\n",
			ctx->state.timestamp, ctx->state.temp.val1,
			ctx->state.temp.val2,
			ctx->state.press.val1, ctx->state.press.val2,
			ctx->state.humidity.val1, ctx->state.humidity.val2);
	}
}

static uint8_t *console_rx_handler(uint8_t *buf, size_t *off)
{
	int status;
	char *const c = &buf[(*off) - 1];

	if (*c == '\r' || *c == '\n') {
		if (*off > 2) {
			/* simple workaround to buffer sharing, stupid as hell */
			uart_irq_rx_disable(console_ctx.dev);

			*c = '\0';

			status = k_work_submit(&console_ctx.wq_item);
			if (status < 0) {
				LOG_ERR("submission to workqueue failed: %d",
								status);
			}
			console_ctx.rx_buf_len = *off;
		}
		*off = 0;
	} else if (*off >= console_ctx.rx_buf_size) {
		*off = 0;		
	}	

	return buf;
}

/* ASSUMPTION: the command consists of two bytes */
static inline uint8_t console_get_command_id(const uint8_t *buf)
{

	return (*(buf + 1) - 48) + ((*buf) - 48) * 10;
}

static void console_worker(struct k_work *item)
{
	int status;
	size_t bytes_written;
	enum disk_cmds disk_cmd = NOP;
	const uint8_t cmd_id = console_get_command_id(console_ctx.rx_buf);	

	printk("command_id: %hu\n", cmd_id);
	switch (cmd_id) {
	case START_MEASUREMENTS:
		disk_cmd = CREATE_DIR;
		counter_set_channel_alarm(rtc, 0, &rtc_alarm_cfg); 
		break;	
	case STOP_MEASUREMENTS:
		counter_cancel_channel_alarm(rtc, 0);
		disk_cmd = CLOSE_DIR;
		break;
	case GET_MEASUREMENTS:
		disk_cmd = FILE_READ;
		break;
	case SET_SAMPLING_INTERVAL:
		rtc_alarm_cfg.ticks = atoi(&console_ctx.rx_buf[2]);
		break;
	case GET_CONTINUOUS_MEASUREMENTS:
		console_ctx.cont_flag ^= 1;
		break;
	case LIST_MEASUREMENTS:
		disk_cmd = LIST_DIRS;
		break;
	default:
		break;
	}

	/* find more elegant way for checking this flags e.g. make proper ascending fields in enum */
	if (disk_cmd != NOP && disk_cmd != LIST_DIRS && disk_cmd != CLOSE_DIR) {
		k_poll_signal_raise(&disk_sig, disk_cmd);
		status = k_pipe_put(&fs_ops_path_pipe, &console_ctx.rx_buf[2], console_ctx.rx_buf_len - 2,
					&bytes_written, console_ctx.rx_buf_len - 2, K_FOREVER);	
		if (status) {
			printk("k_pipe_put() failed!\n");
		}
	}

	/* enable the rx interrupts again */
	uart_irq_rx_enable(console_ctx.dev);
}
