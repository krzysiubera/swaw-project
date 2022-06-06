#ifndef _CONSOLE_CMDS_
#define _CONSOLE_CMDS_

enum console_cmds {
	START_MEASUREMENTS,
	STOP_MEASUREMENTS,
	GET_MEASUREMENTS, /* <CMD_ID><date in string format i.e. "24.09.2022"> */
	SET_SAMPLING_INTERVAL, /* <CMD_ID><sampling interval value in string format i.e. "13" */
	GET_CONTINUOUS_MEASUREMENTS,
	LIST_MEASUREMENTS,
};

#endif
