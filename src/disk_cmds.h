#ifndef _DISK_CMDS_
#define _DISK_CMDS_

enum disk_cmds {
	CREATE_DIR,
	OPEN_DIR,
	CLOSE_DIR,
	LIST_DIRS,
	FILE_READ,
	FILE_WRITE,
	FILE_CLOSE,
	FILE_CREATE,
	FILE_OPEN,
	DATA_WRITE,
	NOP,
};

enum file_type {
	READ_DESC,
	WRITE_DESC,
};


#endif
