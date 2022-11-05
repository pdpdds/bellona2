#ifndef RAM_DISK_HEADER_jj
#define RAM_DISK_HEADER_jj

typedef struct RamDiskTag {
	char	*pBuff;			// 블록으로 사용하기 위해 할당된 메모리에 대한 포인터	
};
typedef struct RamDiskTag RamDiskStt;				// ram disk specific data

extern int init_ram_disk_driver();
extern int close_ram_disk_driver();

#endif
