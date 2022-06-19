#include <drivers/counter.h>
#include <devicetree.h>
#include "rtc_app.h"

int rtc_get_device(const struct device **dev)
{
	*dev = DEVICE_DT_GET_ANY(st_stm32_rtc);
	printk("\n\"%x\"", (const int) *dev);
	
	if (*dev == NULL) {
		printk("\nError: no device found.\n");
		return -ENODEV;
	}

	if (!device_is_ready(*dev)) {
		printk("\nError: Device \"%s\" is not ready; "
		       "check the driver initialization logs for errors.\n",
		       (*dev)->name);
		return -EINVAL;
	}

	printk("Found device \"%s\"\n", (*dev)->name);
	return 0;
}
