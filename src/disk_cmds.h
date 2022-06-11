#ifndef _DISK_CMDS_
#define _DISK_CMDS_

enum disk_cmds {
	LIST_FILES,
	FILE_READ,
	FILE_WRITE,
	FILE_CLOSE,
	FILE_CREATE,
	NOP,
};

enum file_type {
	READ_DESC,
	WRITE_DESC,
};


#endif
