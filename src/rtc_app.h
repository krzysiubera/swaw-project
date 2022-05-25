#ifndef _RTC_APP_H_
#define _RTC_APP_H_

#include <zephyr.h>
#include <device.h>

int rtc_get_device(const struct device **dev);

#endif
