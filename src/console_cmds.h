#ifndef _CONSOLE_CMDS_
#define _CONSOLE_CMDS_

enum console_cmds {
	START_MEASUREMENTS, /*FORMAT: "<00>;<interval>;<filename-path>" e.g "00;05;/SD:/11.05.2020" */
	STOP_MEASUREMENTS, /* "01" */
	GET_MEASUREMENTS, /* "<02>;<filename-path>" e.g. "02;/SD:/11.05.2020" */ 
	GET_CONTINUOUS_MEASUREMENTS, /* "03" */
	LIST_MEASUREMENTS, /* "04" */
};

#endif
