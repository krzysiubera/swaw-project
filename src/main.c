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

#define CONSOLE_BUF_SIZE	(255U)

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
static struct k_pipe sensor_data_pipe;

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
static int ls(const char *path);

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
	printk("ok\n");

	uart_pipe_register(console_rx_buf, CONSOLE_BUF_SIZE, console_rx_handler);

	k_pipe_init(&fs_ops_path_pipe, NULL, 0);
	k_pipe_init(&sensor_data_pipe, NULL, 0);

	k_poll_signal_init(&disk_sig);

	k_work_init(&bme280_ctx.wq_item, bme280_worker);
	k_work_init(&console_ctx.wq_item, console_worker);

	k_thread_create(&disk_thr, disk_stack, 4096,disk_worker, NULL, NULL,
						NULL, -2, 0, K_NO_WAIT); 

	while (1) {
	
	}
}

void disk_worker(void *a, void *b, void *c)
{
	int status = 10;
	ssize_t bytes_read = 10;
	ssize_t bytes_written;
	struct k_poll_event disk_events[1] = {
		K_POLL_EVENT_INITIALIZER(K_POLL_TYPE_SIGNAL,
					K_POLL_MODE_NOTIFY_ONLY,
					&disk_sig),
	};

	struct bme280_state sensor_data[2];
	char fs_ops_buf[CONSOLE_BUF_SIZE]; 
	char sensor_data_stream[512]; //number completely out of blue 
	char file_header[32];
	uint32_t ticks;
	unsigned int off = 0;
	

	struct fs_file_t file[2];

	for (;;) {
		k_poll(disk_events, 1, K_FOREVER);

		if (disk_events[0].signal->result == FILE_CREATE) {
			off = 6;
		} else if (disk_events[0].signal->result == FILE_READ) {
			off = 3;
		}

		status = k_pipe_get(&fs_ops_path_pipe, fs_ops_buf, console_ctx.rx_buf_len - off, &bytes_read,
								console_ctx.rx_buf_len - off, K_NO_WAIT);

		switch (disk_events[0].signal->result) {
		case LIST_FILES:
			ls(FATFS_MOUNT_PT); 
			break;	
		case FILE_CREATE:
			printk("FILE CREATE\n");
			fs_file_t_init(&file[WRITE_DESC]);
			status = fs_open(&file[WRITE_DESC], (const char*) fs_ops_buf,
					FS_O_CREATE | FS_O_WRITE | FS_O_APPEND);
			if (status) {
				printk("fs_open error: %d\n", status);
			}

			fs_write(&file[WRITE_DESC], &rtc_alarm_cfg.ticks,
					sizeof(rtc_alarm_cfg.ticks)); 
			break;
		case FILE_WRITE:
			printk("FILE_WRITE!\n");
			status = k_pipe_get(&sensor_data_pipe, &sensor_data[WRITE_DESC],
				sizeof(*sensor_data), &bytes_read, sizeof(*sensor_data),
									K_FOREVER);	
			bytes_written = fs_write(&file[WRITE_DESC], &sensor_data[WRITE_DESC],
									sizeof(*sensor_data));
			printk("bytes_written: %d\n", bytes_written);
			if (bytes_written < 0) {
				printk("fs_write fail: %d\n", bytes_written);
			}
			break;
		case FILE_READ:
			printk("FILE READ\n");
			fs_file_t_init(&file[READ_DESC]);
			status = fs_open(&file[READ_DESC], (const char*) fs_ops_buf,
									FS_O_READ);
			if (status < 0) {
				printk("fs_open error: %d\n", status);
			}

			fs_read(&file[READ_DESC], &ticks, sizeof(ticks));
			snprintk(file_header, 32, "interval: %u\n", ticks);
			uart_pipe_send((const uint8_t*) file_header, strlen(file_header));

			for (;;) {
				bytes_read = fs_read(&file[READ_DESC], &sensor_data[READ_DESC],
									sizeof(*sensor_data));
				if (bytes_read <= 0) {
					break;
				}

				printk("bytes_read %d\n", bytes_read);
				snprintk(sensor_data_stream, 512, 
					"temp: %d.%06d; press: %d.%06d; humidity: %d.%06d;\n",
					sensor_data[READ_DESC].temp.val1,
					sensor_data[READ_DESC].temp.val2,
					sensor_data[READ_DESC].press.val1 * 10,
					sensor_data[READ_DESC].press.val2,
					sensor_data[READ_DESC].humidity.val1,
					sensor_data[READ_DESC].humidity.val2);

				uart_pipe_send((const uint8_t*) sensor_data_stream,
							strlen(sensor_data_stream));
					
			}

			fs_close(&file[READ_DESC]);
			break;
		case FILE_CLOSE:
			printk("FILE CLOSE\n");
			fs_close(&file[WRITE_DESC]);
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
	int status;
	size_t bytes_written;
	struct bme280_context *ctx;
	struct bme280_state state;
	char buf[512];

	ctx = CONTAINER_OF(item, struct bme280_context, wq_item);

	sensor_sample_fetch(ctx->dev);
	sensor_channel_get(ctx->dev, SENSOR_CHAN_AMBIENT_TEMP, &state.temp);
	sensor_channel_get(ctx->dev, SENSOR_CHAN_PRESS, &state.press);
	sensor_channel_get(ctx->dev, SENSOR_CHAN_HUMIDITY, &state.humidity);

	k_poll_signal_raise(&disk_sig, FILE_WRITE);
	status = k_pipe_put(&sensor_data_pipe, &state, sizeof(struct bme280_state),
				&bytes_written, sizeof(struct bme280_state), K_FOREVER);	
	if (status) {
		printk("k_pipe_put() failed!\n");
	}

	if (console_ctx.cont_flag) {
		snprintk(buf, 512, "temp: %d.%06d; press: %d.%06d; humidity: %d.%06d;\n",
			state.temp.val1, state.temp.val2,
			state.press.val1, state.press.val2,
			state.humidity.val1, state.humidity.val2);

		uart_pipe_send((const char*) buf, strlen(buf));
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

static inline uint8_t console_get_num(const uint8_t *buf)
{
	return (*(buf + 1) - 48) + ((*buf) - 48) * 10;
}

/* ASSUMPTION: the command consists of two bytes */
static inline uint8_t console_get_command_id(const uint8_t *buf)
{
	return console_get_num(buf);
}

static void console_worker(struct k_work *item)
{
	int status;
	unsigned int off = 0;
	size_t bytes_written;
	enum disk_cmds disk_cmd = NOP;
	const uint8_t cmd_id = console_get_command_id(console_ctx.rx_buf);	

	printk("command_id: %hu\n", cmd_id);
	switch (cmd_id) {
	case START_MEASUREMENTS:
		disk_cmd = FILE_CREATE;
		/* <cmd-id(2 bytes)>:<sample-rate(2 bytes)>:<file-name> */
		off = 6;
		rtc_alarm_cfg.ticks = console_get_num(&console_ctx.rx_buf[3]);
		counter_set_channel_alarm(rtc, 0, &rtc_alarm_cfg); 
		break;	
	case STOP_MEASUREMENTS:
		counter_cancel_channel_alarm(rtc, 0);
		disk_cmd = FILE_CLOSE;
		break;
	case GET_MEASUREMENTS:
		off = 3;
		disk_cmd = FILE_READ;
		break;
	case SET_SAMPLING_INTERVAL:
		rtc_alarm_cfg.ticks = atoi(&console_ctx.rx_buf[2]);
		break;
	case GET_CONTINUOUS_MEASUREMENTS:
		console_ctx.cont_flag ^= 1;
		break;
	case LIST_MEASUREMENTS:
		disk_cmd = LIST_FILES;
		break;
	default:
		break;
	}

	if (disk_cmd != NOP) {
		k_poll_signal_raise(&disk_sig, disk_cmd);
		if (disk_cmd == FILE_READ || disk_cmd == FILE_CREATE) {
			status = k_pipe_put(&fs_ops_path_pipe, &console_ctx.rx_buf[off],
					console_ctx.rx_buf_len - off, &bytes_written, 
					console_ctx.rx_buf_len - off, K_FOREVER);	
			if (status) {
				printk("k_pipe_put() failed!\n");
			}
		}
	}

	/* enable the rx interrupts again */
	uart_irq_rx_enable(console_ctx.dev);
}

static int ls(const char *path)
{
	int status;
	struct fs_dir_t dir;
	struct fs_dirent dir_entry;
	char buf[128];

	/* initialize directory instance */	
	fs_dir_t_init(&dir);

	status = fs_opendir(&dir, path);
	if (status < 0) {
		printk("fs_opendir() error: %d\n", status);
		return -EINVAL;	
	}

	for (;;) {
		status = fs_readdir(&dir, &dir_entry);
		if ((status < 0) || (dir_entry.name[0] == 0)) {
			break;
		}
				
		if (dir_entry.type == FS_DIR_ENTRY_FILE) {
			snprintk(buf, 128, "FILE: %s (%d bytes)\n", dir_entry.name, 
								dir_entry.size);
		} else {
			snprintk(buf, 128, "DIR: %s\n", dir_entry.name);
		}

		uart_pipe_send((const uint8_t*) buf, strlen(buf));
	}

	status = fs_closedir(&dir);
	if (status < 0) {
		printk("fs_closedir() error: %d\n", status);	
	}
	
	return 0;
}
