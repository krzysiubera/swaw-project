#include <devicetree.h>
#include "bme280_app.h"

int bme280_get_device(const struct device **dev)
{
	*dev = DEVICE_DT_GET_ANY(bosch_bme280);
	printk("\n\"%x\"", (const int) *dev);
	
	if (*dev == NULL) {
		/* No such node, or the node does not have status "okay". */
		printk("\nError: no device found.\n");
		return -ENODEV;
	}

	if (!device_is_ready(*dev)) {
		printk("\nError: Device \"%s\" is not ready; "
		       "check the driver initialization logs for errors.\n",
		       (*dev)->name);
		return -EINVAL;
	}

	printk("Found device \"%s\", getting sensor data\n", (*dev)->name);
	return 0;
}
