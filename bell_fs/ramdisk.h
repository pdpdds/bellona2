#ifndef RAM_DISK_HEADER_jj
#define RAM_DISK_HEADER_jj

typedef struct RamDiskTag {
	char	*pBuff;			// ������� ����ϱ� ���� �Ҵ�� �޸𸮿� ���� ������	
};
typedef struct RamDiskTag RamDiskStt;				// ram disk specific data

extern int init_ram_disk_driver();
extern int close_ram_disk_driver();

#endif
