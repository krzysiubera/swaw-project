#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <kernel.h>
#include <sys/ring_buffer.h>
#include <drivers/sensor.h>
#include <drivers/uart.h>
#include <drivers/counter.h>
#include <storage/disk_access.h>
#include <fs/fs.h>
#include <ff.h>
#include "rtc_app.h"
#include "bme280_app.h"
#include <logging/log.h>
LOG_MODULE_REGISTER(main);

#define DISK_WRITE_REQ		(0x0101)
#define DISK_READ_REQ		(0x0100)
#define DISK_DMA_READ_REQ	(0x0110)

/* ring buffer */
#define DATA_BUF_SIZE		(1024)
static uint8_t bme280_data_buf[DATA_BUF_SIZE];
static struct ring_buf bme280_data_ring_buf;

/* message queue */
static struct bme280_state data_msgq_buf[4]__attribute__((aligned(4)));
static struct k_msgq data_msgq;

/* k_poll_signal object */
static struct k_poll_signal disk_sig;
static struct k_poll_event disk_event;

static struct bme280_context bme280_ctx;

void bme280_worker(struct k_work *item);


void disk_worker(void *a, void *b, void *c);
void ui_worker(void *a, void *b, void *c);

/*
 *Define and initialize threads.
 */
K_THREAD_STACK_DEFINE(disk_stack, 800);
K_THREAD_STACK_DEFINE(ui_stack, 800);
static struct k_thread disk_thr, ui_thr;

static int disk_init(const char *pdrv);

void rtc_alarm_handler(const struct device *dev, uint8_t chan_id,
				uint32_t ticks, void *user_data);

static struct counter_alarm_cfg rtc_alarm_cfg = {
	.callback	= rtc_alarm_handler,
	.ticks		= 2U,
	.user_data	= (struct counter_alarm_cfg*) &rtc_alarm_cfg,
	.flags		= 0,
};

static const char *disk = "SD";

void main(void)
{	
	int status;
	const struct device *rtc = NULL, *bme280 = NULL;
	
	if (rtc_get_device(&rtc)) {
		for (;;);
	}

	if (bme280_get_device(&bme280_ctx.dev)) {
		for (;;);
	}

	if (disk_init(disk)) {
		for (;;);
	}

	k_msgq_init(&data_msgq, (char *)data_msgq_buf, sizeof(*data_msgq_buf), 4);

	k_poll_signal_init(&disk_sig);

	ring_buf_init(&bme280_data_ring_buf, DATA_BUF_SIZE, bme280_data_buf); 

	k_work_init(&bme280_ctx.wq_item, bme280_worker);

	status = k_thread_create(&disk_thr, disk_stack, 800, disk_worker, NULL, NULL,
			NULL, -1, 0, K_NO_WAIT); 

	counter_set_channel_alarm(rtc, 0, &rtc_alarm_cfg); 

	while (1) {
	
	}
}

void disk_worker(void *a, void *b, void *c)
{
	unsigned int msgs_num;
	struct k_poll_event disk_events[1] = {
		K_POLL_EVENT_INITIALIZER(K_POLL_TYPE_SIGNAL,
					K_POLL_MODE_NOTIFY_ONLY,
					&disk_sig),
	};
	struct bme280_state curr_sensor_data[4];

	for (;;) {
		k_poll(disk_events, 1, K_FOREVER);

		switch (disk_events[0].signal->result) {
		case DISK_WRITE_REQ:
			msgs_num = k_msgq_num_used_get(&data_msgq);
			for (unsigned int i = 0; i < msgs_num; ++i) {
				k_msgq_get(&data_msgq, curr_sensor_data, K_FOREVER); 
			}
			printk("welcome from disk_worker()\n");
			break;
		case DISK_READ_REQ:
			break;
		case DISK_DMA_READ_REQ:
			break;
		default:
			break;
		}
		
		/* reset the disk_events state */
		disk_events[0].signal->signaled = 0;
		disk_events[0].state = K_POLL_STATE_NOT_READY;
	}
}

void ui_worker(void *a, void *b, void *c)
{
	for (;;) {
	
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

	k_poll_signal_raise(&disk_sig, DISK_WRITE_REQ);

	printk("[timestamp: %u] temp: %d.%06d; press: %d.%06d; humidity: %d.%06d;\n",
		ctx->state.timestamp, ctx->state.temp.val1,
		ctx->state.temp.val2,
		ctx->state.press.val1, ctx->state.press.val2,
		ctx->state.humidity.val1, ctx->state.humidity.val2);
}
