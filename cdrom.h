#ifndef BELLONA2_CDROM_HEADER_jj
#define BELLONA2_CDROM_HEADER_jj

#define CD_DATA			0x0
#define CD_ERROR		0x01
#define CD_FEATURE		0x01
#define CD_CAUSE_OF_INT	0x02
#define CD_COUNT_L		0x04
#define CD_COUNT_H		0x05
#define CD_DRV_SELECT	0x06
#define CD_STATUS		0x07
#define CD_COMMAND		0x07

typedef struct {
	UINT16		wConfig;
	UINT16		rsv0[9];
	UINT16		serial[10];
	UINT16		rsv2[3];
	UINT16		firmware[4];
	UINT16		model[20];
	UINT16		rsv3[512 - 47];
	
	//=======================//
	char		szSerial[21];
	char		szModel[41];
} CDGeometryStt;


extern int read_cd_capacity( int nDrv, DWORD *pTotalBlock, DWORD *pBlockSize );
extern int read_cd_geometry_info( CDGeometryStt *pGeo, DWORD dwBasePort, DWORD dwMasterSlave );

#endif
