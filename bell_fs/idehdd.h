#ifndef VFS_IDE_HDD_BLK_DEV_HEADER_jj
#define VFS_IDE_HDD_BLK_DEV_HEADER_jj

#include "hddpart.h"

typedef struct MBSTag{	// 하드디스크의 마스터부트 섹터
    char			rsv[0x1BE];
    PartitionStt    part[4];
    unsigned short  wSignature;
};
typedef struct MBSTag MBSStt; 

typedef struct IDEHddTag {
	int				nDrive;
	MBSStt			mbs;
	BlkDevObjStt	part_dev_obj[4];
};
typedef struct IDEHddTag IDEHddStt;
					
extern int init_ide_hdd_driver();
extern int close_ide_hdd_driver();

#endif