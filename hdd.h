#ifndef BELLONA2_HDD_IO_HEADER_jj
#define BELLONA2_HDD_IO_HEADER_jj

#include <types.h>

#define PRIMARY_HDD_BASE			0x1F0
#define SECONDARY_HDD_BASE			0x170

#define HDD_DATA					0x0
#define HDD_FEATURE 				0x1
#define HDD_ERROR 					0x1
#define HDD_SECTOR_COUNT			0x2
#define HDD_SECTOR_NUMBER			0x3
#define HDD_CYLINDER_L				0x4
#define HDD_CYLINDER_H				0x5
#define HDD_DRIVE_HEAD				0x6
#define HDD_STATUS					0x7
#define HDD_COMMAND					0x7
#define HDD_ALT_STATUS				0x206
#define HDD_DRIVE_HEAD_STATUS		0x207

#define HDD_RECALIBRATE_COMMAND		0x10
//#define HDD_READ_COMMAND			0x21  // 0x21을 사용하면 VMWare에서 작동하지 않는다.
#define HDD_READ_COMMAND			0x20  // 2003-04-30  READ_SECTOR(S)
#define HDD_READ_DMA_COMMAND		0xC8  
#define HDD_SEEK_COMMAND			0x70
#define HDD_IDENTIFY_COMMAND		0xEC
#define HDD_SET_FEATURE_COMMAND		0xEF

typedef struct IDGeometryTag{
    UINT16  wConfig;
    UINT16  wCylinder;
    UINT16  wRsv1;
    UINT16  wHead;
    UINT16  wUfTrack;
    UINT16  wUfSector;
    UINT16  wSector;			// 6
    UINT16  manufacturer[3];	// 7
    UINT16  serial[10];			// 10
    UINT16  retired[3];			// 20
    UINT16  firm_ver[4];		// 23
    UINT16  model[20];			// 27

	UINT16  byRsv2;				// 47
    UINT16  byRsv3;				// 48
    UINT16  byMode;		  		// 49
	UINT16  rsv_a[3];	  		// 50
	UINT16  wAvailableBit;		// 53
	UINT16  rsv_b[6];	  		// 54
	UINT16  wLBASectors1; 		// 60
	UINT16  wLBASectors2; 		// 61

    UINT16  rsv_c[26];	  		// 62
	UINT16	wUDMAFlag;    		// 88
	UINT16	rsv_d[68];	  		// 89
		
	UINT16  rsv4[100];	  		// 156 (256 Words = 512 bytes)

	// 내가 그냥 야매로 만든 것.
	////////////////////////////////
	char	szManufacturer[7];	  
	char    szModel[41];		  
	DWORD   dwSize;				  
	////////////////////////////////
} IDEGeometryStt;

#define UDMA_MODE_0_SUPPORTED	0x0001
#define UDMA_MODE_1_SUPPORTED	0x0002
#define UDMA_MODE_2_SUPPORTED	0x0004
#define UDMA_MODE_3_SUPPORTED	0x0008
#define UDMA_MODE_4_SUPPORTED	0x0010

#define UDMA_MODE_0_SET			0x0100
#define UDMA_MODE_1_SET 		0x0200
#define UDMA_MODE_2_SET 		0x0400
#define UDMA_MODE_3_SET 		0x0800
#define UDMA_MODE_4_SET 		0x1000

extern int 		ide_auto_detection		();
extern int 		dump_hdd				( int nDrv, DWORD dwIndex );

extern void 	primary_hdd_irq14		();
extern void 	secondary_hdd_irq15		();
extern void 	vCutAppendedSpace		( char *pS );
extern void 	vSwapUINT16Array		( UINT16 *pW, int nTotal );

extern IDEGeometryStt *get_ide_geo_ptr	( int nI );

#endif
