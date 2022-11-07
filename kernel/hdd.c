// hard disk low lovel i/o

#include "bellona2.h"

typedef struct  {
	DWORD dwCylinder;
	DWORD dwHead;
	DWORD dwSector;
	DWORD dwTotalSector;
	DWORD dwBasePort;
	DWORD dwMasterSlave;
}CHSStt;

_declspec(naked) void primary_hdd_irq14()
{
	_asm { 
		PUSHAD
	    PUSHFD
	    CLI
		PUSH DS				
		PUSH ES				
		PUSH FS				
		PUSH GS				
		MOV AX, GSEL_DATA32
		MOV DS, AX
		MOV ES, AX
		MOV FS, AX
		MOV GS, AX
	}
	
	inc_event_count( &primary_ide_event );
	
	vSendEOI( 14 );

	_asm {
		POP GS
		POP FS
		POP ES
		POP DS
		POPFD
		POPAD
		IRETD
	}
}

_declspec(naked) void secondary_hdd_irq15()
{
	_asm { 
		PUSHAD
	    PUSHFD
	    CLI
		PUSH DS				
		PUSH ES				
		PUSH FS				
		PUSH GS				
		MOV AX, GSEL_DATA32
		MOV DS, AX
		MOV ES, AX
		MOV FS, AX
		MOV GS, AX
	}
	
	inc_event_count( &secondary_ide_event );
	vSendEOI( 15 );

	_asm {
		POP GS
		POP FS
		POP ES
		POP DS
		POPFD
		POPAD
		IRETD
	}
}

static int is_hdd_busy( DWORD dwBasePort )
{
	int 	nI;
	BYTE 	byTe;
	// check busy bit
	for( nI = 0; nI < 3; nI++ )
	{	
		vReadPort( (DWORD)( HDD_STATUS + dwBasePort ), &byTe );
		// busy bit must be 0
		if((  byTe & (UCHAR)0x80 ) == 0 )
			break;		
		
		kdelay( 30 );
	}
	if( nI == 3 )
	{
		kdbg_printf( "IDE device is busy.\n" );
		return( -1 );
	}
	return( 0 );
}

// initialize hdd
static int ide_hdd_recalibrate( DWORD dwBasePort, DWORD dwMasterSlave )
{
	int		nR;
	DWORD	dwHddDrv;

	// clear event count
	if( dwBasePort == PRIMARY_HDD_BASE )
		clear_event_count( &primary_ide_event );
	else
		clear_event_count( &secondary_ide_event );

	// send recalibrate command
	dwHddDrv = (DWORD)( ( dwMasterSlave << 4 ) | (DWORD)0xE0 );  // 1110 0000  (LBA Mode)
	vWritePort( (DWORD)( dwBasePort + HDD_DRIVE_HEAD), dwHddDrv );
	vWritePort( (DWORD)( dwBasePort + HDD_COMMAND), HDD_RECALIBRATE_COMMAND );

	// wait interrupt
	if( dwBasePort == PRIMARY_HDD_BASE )
		nR = wait_event( &primary_ide_event, 250 );
	else
		nR = wait_event( &secondary_ide_event, 250 );

	if( nR < 0 )
		kdbg_printf( "hdd recalibrate error (timeout) : base_port( %X), master_slave( %d )\n", dwBasePort, dwMasterSlave );
	else
		kdbg_printf( "hdd recalibrate - ok\n" );
	return( nR );
}					

/** 2003-05-15
 *
 * VMWare에서 돌리면 완료 인터럽트가 걸리지 않고 timeout이 발생한다.
 * 지원하지 않는 명령이기 때문에 그런것인가? (지원할 필요가 없을수도 있겠지.)
 * 일단 사용하지 않고 놔두자.
**/
static int seek_hdd( CHSStt *pCHS )
{
	BYTE 	byTe;
	DWORD	dwTemp;
	int		nI, nR;
	
	// check busy bit
	for( nI = 0; ; nI++ )
	{	
		if( nI >= 3 )
		{
			kdbg_printf( "HDD seek failed : port is busy.\n" );
			return( -1 );
		}
		vReadPort( (DWORD)(HDD_STATUS + pCHS->dwBasePort), &byTe );
		// busy bit must be 0
		if((  byTe & (UCHAR)0x80 ) == 0 )
			break;		
		
		kdelay( 30 );
	}

	// send seek parameter
    //vWritePort( (DWORD)( pCHS->dwBasePort + HDD_SECTOR_COUNT),  (DWORD)pCHS->dwTotalSector );
    //vWritePort( (DWORD)( pCHS->dwBasePort + HDD_SECTOR_NUMBER), (DWORD)pCHS->dwSector );
	dwTemp    = (DWORD)( pCHS->dwCylinder & (DWORD)0xFF);
    vWritePort( (DWORD)( pCHS->dwBasePort + HDD_CYLINDER_L), dwTemp );
	dwTemp    = (DWORD)( pCHS->dwCylinder >> 8 );
    vWritePort( (DWORD)( pCHS->dwBasePort + HDD_CYLINDER_H), dwTemp );	
	dwTemp    = (DWORD)( pCHS->dwHead & 0x0F ) | (DWORD)( pCHS->dwMasterSlave << 4 ) | (DWORD)0xE0;  // 1110 0000  (LBA Mode)
	vWritePort( (DWORD)( pCHS->dwBasePort + HDD_DRIVE_HEAD), dwTemp );

	// clear event count
	if( pCHS->dwBasePort == PRIMARY_HDD_BASE )
		clear_event_count( &primary_ide_event );
	else
		clear_event_count( &secondary_ide_event );

	// send seek command
	vWritePort( (DWORD)( pCHS->dwBasePort + HDD_COMMAND), HDD_SEEK_COMMAND );
	
	// wait irq
	if( pCHS->dwBasePort == PRIMARY_HDD_BASE )
		nR = wait_event( &primary_ide_event, 1000 );
	else
		nR = wait_event( &secondary_ide_event, 1000 );
	if( nR < 0 )
	{	// time out
		kdbg_printf( "HDD seek failed : timeout!\n" );
		return( -1 );
	}									 	

	return( 0 );
}							   

static int scatter_read_sectors_by_lba( CHSStt *pCHS, BCacheEntStt **ppCEntArray )
{
	int 	nR, nI;
	DWORD	dwTemp;
	
	//nR = seek_hdd( pCHS );  VMWare에서 동작하지 않는다.
	
	// send read parameter
	vWritePort( (DWORD)( pCHS->dwBasePort + HDD_SECTOR_COUNT),	(DWORD)pCHS->dwTotalSector );
	vWritePort( (DWORD)( pCHS->dwBasePort + HDD_SECTOR_NUMBER), (DWORD)pCHS->dwSector );
	dwTemp	  = (DWORD)( pCHS->dwCylinder & (DWORD)0xFF);  // Low cylinder
	vWritePort( (DWORD)( pCHS->dwBasePort + HDD_CYLINDER_L), dwTemp );
	dwTemp	  = (DWORD)( pCHS->dwCylinder >> 8 );
	vWritePort( (DWORD)( pCHS->dwBasePort + HDD_CYLINDER_H ), dwTemp & (DWORD)0xFF );	
	dwTemp	  = (DWORD)( pCHS->dwHead & 0x0F ) | (DWORD)( pCHS->dwMasterSlave << 4 ) | (DWORD)0xE0;  // 1110 0000  (LBA Mode)
	vWritePort( (DWORD)( pCHS->dwBasePort + HDD_DRIVE_HEAD), dwTemp );
		
	// clear event count
	if( pCHS->dwBasePort == PRIMARY_HDD_BASE )
		clear_event_count( &primary_ide_event );
	else
		clear_event_count( &secondary_ide_event );
	
	// Send Read Command
	vWritePort( (DWORD)( pCHS->dwBasePort + HDD_COMMAND), HDD_READ_COMMAND );
	
	// wait irq
	if( pCHS->dwBasePort == PRIMARY_HDD_BASE )
		nR = wait_event( &primary_ide_event, 3000 );		// 여러 섹터를 읽는 것이므로 좀 더 기다리자...
	else
		nR = wait_event( &secondary_ide_event, 3000 );
	
	/*	결과 상태 값을 읽는 부분인데.. (필요없나?)
		for( nI = 0; nI < 6; nI++ )
		{
			BYTE byTe;
			vReadPort( (DWORD)(pCHS->dwBasePort+HDD_STATUS), &byTe );
		}
	*/
	
	// read data from port
	if( nR < 0 )
	{
		kdbg_printf( "hdd scatter read command(0x%X) failed!\n", HDD_READ_COMMAND );
		return( -1 );
	}

	// 섹터들을 홀라당 읽어들인다.
	for( nI = 0; nI < (int)pCHS->dwTotalSector; nI++ )		
	{
		// 섹터 버퍼가 Ready로 되기를 기다린다.
		for( ;; )
		{
			BYTE byTe;
			byTe = 0;
			vReadPort( (DWORD)(pCHS->dwBasePort+HDD_STATUS), &byTe );
			if( byTe & 8 )
				break;
		}
		
		vReadPortMultiWord( (DWORD)( pCHS->dwBasePort+HDD_DATA ), (UINT16*)ppCEntArray[nI]->pBuff, 512 / 2 );
	}

	return( 0 );
}

// read sector by lba
static int read_sectors_by_lba( CHSStt *pCHS, char *pB )
{
	int		nR;
	DWORD	dwTemp;

	//nR = seek_hdd( pCHS );  VMWare에서 동작하지 않는다.

	// send read parameter
    vWritePort( (DWORD)( pCHS->dwBasePort + HDD_SECTOR_COUNT),  (DWORD)pCHS->dwTotalSector );
    vWritePort( (DWORD)( pCHS->dwBasePort + HDD_SECTOR_NUMBER), (DWORD)pCHS->dwSector );
	dwTemp    = (DWORD)( pCHS->dwCylinder & (DWORD)0xFF);  // Low cylinder
    vWritePort( (DWORD)( pCHS->dwBasePort + HDD_CYLINDER_L), dwTemp );
	dwTemp    = (DWORD)( pCHS->dwCylinder >> 8 );
    vWritePort( (DWORD)( pCHS->dwBasePort + HDD_CYLINDER_H ), dwTemp & (DWORD)0xFF );	
	dwTemp    = (DWORD)( pCHS->dwHead & 0x0F ) | (DWORD)( pCHS->dwMasterSlave << 4 ) | (DWORD)0xE0;  // 1110 0000  (LBA Mode)
	vWritePort( (DWORD)( pCHS->dwBasePort + HDD_DRIVE_HEAD), dwTemp );
	
	// clear event count
	if( pCHS->dwBasePort == PRIMARY_HDD_BASE )
		clear_event_count( &primary_ide_event );
	else
		clear_event_count( &secondary_ide_event );

	// Send Read Command
	vWritePort( (DWORD)( pCHS->dwBasePort + HDD_COMMAND), HDD_READ_COMMAND );

	// wait irq
	if( pCHS->dwBasePort == PRIMARY_HDD_BASE )
		nR = wait_event( &primary_ide_event, 1000 );
	else
		nR = wait_event( &secondary_ide_event, 1000 );

/*  결과 상태 값을 읽는 부분인데.. (필요없나?)
	for( nI = 0; nI < 6; nI++ )
	{
		BYTE byTe;
		vReadPort( (DWORD)(pCHS->dwBasePort+HDD_STATUS), &byTe );
	}
*/

	// read data from port
	if( nR < 0 )
	{
		kdbg_printf( "hdd read command(0x%X) failed!\n", HDD_READ_COMMAND );
		return( -1 );
	}
	
	// read data  (2003-07-26)	한 워드씩 읽다가 vReadPortMultiWord를 이용하여 한 번에 모든 데이터를 읽음.
	//for( nI = 0; nI < (int)pCHS->dwTotalSector * 512 / 2; nI++ )
	//	vReadPortWord( (DWORD)( pCHS->dwBasePort+HDD_DATA ), (UINT16*)&pB[nI*2] );

	// 섹터 버퍼가 Ready로 되기를 기다린다.
	for( ;; )
	{
		BYTE byTe;
		byTe = 0;
		vReadPort( (DWORD)(pCHS->dwBasePort+HDD_STATUS), &byTe );
		if( byTe & 8 )
			break;
	}

	vReadPortMultiWord( (DWORD)( pCHS->dwBasePort+HDD_DATA ), (UINT16*)pB, pCHS->dwTotalSector * 512 / 2 );
	
	return( 0 );
}	

void vSwapUINT16Array( UINT16 *pW, int nTotal )
{
	int		nI;
	UCHAR	*pB, byTemp;

	for( nI = 0; nI < nTotal ; nI++ )
	{
		pB     = (UCHAR*)&pW[nI];
		byTemp = pB[0];
		pB[0]  = pB[1];
		pB[1]  = byTemp;
	}		   
}
// 스트링의 뒤에 붙어있는 스페이스를 없앤다.
void vCutAppendedSpace( char *pS )
{
	int nI;

	for( nI = strlen( pS ) -1; nI > 0; nI-- )
	{
		if( pS[nI] == ' ' )
			pS[nI] = 0;
		else 
			break;
	}		
}

static int check_ide_hdd_busy( DWORD dwBasePort )
{
	int 	nI;
	BYTE	byTe;

	// check busy bit
	for( nI = 0; ; nI++ )
	{	
		if( nI >= 3 )
			return( -1 );

		vReadPort( (DWORD)(HDD_STATUS + dwBasePort), &byTe );
		// busy bit must be 0
		if((  byTe & (UCHAR)0x80 ) == 0 )
			break;		
		
		kdelay(30);
	}
	return( 0 );
}

// read geometry information
static int  read_hdd_geometry_info(  IDEGeometryStt *pGeo, DWORD dwBasePort, DWORD dwMasterSlave  )
{
	UINT16  *pB;
	UCHAR	byTe;
	int		nI, nR;
	DWORD	dwTemp;
	
	pB = (UINT16*)pGeo;

	memset( pGeo, 0, sizeof( IDEGeometryStt ) );

	check_ide_hdd_busy( dwBasePort );

	dwTemp    = (DWORD)( dwMasterSlave << 4 ) | (DWORD)0xE0;  // 1110 0000  LBA Mode
	vWritePort( (DWORD)( dwBasePort + HDD_DRIVE_HEAD), dwTemp );
	// Send Identify Command
	vWritePort( (DWORD)( dwBasePort + HDD_COMMAND), HDD_IDENTIFY_COMMAND );

	// wait irq
	if( dwBasePort == PRIMARY_HDD_BASE )
		nR = wait_event( &primary_ide_event, 1000 );
	else
		nR = wait_event( &secondary_ide_event, 1000 );
	if( nR < 0 )
		return( -1 );		// time out

	// read 256 words
	for( nI = 0; nI < 512/2; nI++ )
		vReadPortWord( (DWORD)(dwBasePort+HDD_DATA), (UINT16*)&pB[nI] );

	if( pGeo->wConfig == 0 )
		return( -1 );		// failed!

	// 상태 바이트 하나를 읽는다. (2003-04-25)
	//for( nI = 0; nI < 5; nI++ )
	vReadPort( (DWORD)(dwBasePort+HDD_STATUS), &byTe );
	//kdbg_printf( "HDD-GEO Status Byte : 0x%X\n", byTe );

	// swap upper and lower	byte
	vSwapUINT16Array( pGeo->manufacturer, 3 );
	vSwapUINT16Array( pGeo->model,  20 );

	memcpy( pGeo->szManufacturer, pGeo->manufacturer, 6  );
	memcpy( pGeo->szModel,        pGeo->model,       40 );
	pGeo->szManufacturer[6] = pGeo->szModel[40] = 0;
	// 문자로 시작하지 않으면 아닌 것으로 본다.
	if( is_char( pGeo->szModel[0] ) == 0 || pGeo->szModel[1] == 127 )
		return( -1 );

	vCutAppendedSpace( pGeo->szManufacturer );
	vCutAppendedSpace( pGeo->szModel );
	
	// calculate size by MB.
	//pGeo->dwSize = (DWORD)( pGeo->wCylinder * pGeo->wHead * pGeo->wSector ) / (2*1024);
	pGeo->dwSize = (DWORD)( (UINT16)(pGeo->wLBASectors1 >> 11) | (UINT16)(pGeo->wLBASectors2 << 5) );// * 1024 / 1000;
		
	return( 0 );		
}	

// LBA-> CHS = xHCCCCSS
static void vLBA2CHS( CHSStt *pChs, DWORD dwLBA )
{
	pChs->dwSector   = (DWORD)( dwLBA & 0xFF );
	pChs->dwCylinder = (DWORD)( (DWORD)(dwLBA >>  8  ) & 0xFFFF );
	pChs->dwHead	 = (DWORD)( (DWORD)(dwLBA >>  24 ) & 0x0F );
}	

// read one sector
int read_sectors( int nDrv, DWORD dwIndex, int nTotalSector, char *pBuff )
{
	CHSStt		chs;
	int 		nR;
	DWORD		dwBasePort;

	if( nDrv <= 1 )
		dwBasePort = PRIMARY_HDD_BASE;
	else if( nDrv <= 3 )
		dwBasePort = SECONDARY_HDD_BASE;
	else
		return( -1 );

	vLBA2CHS( &chs, dwIndex );
	chs.dwTotalSector = nTotalSector;
	chs.dwMasterSlave = nDrv % 2;
	chs.dwBasePort    = dwBasePort;

	nR = read_sectors_by_lba( &chs, pBuff );  

	return( nR );
}

static IDEGeometryStt G_ide_geo[4];

IDEGeometryStt *get_ide_geo_ptr( int nI )
{
	if( nI < 0 || nI >= 4 )
		return( NULL );
		
	return( &G_ide_geo[nI] );
}

static int get_udma_support_number( IDEGeometryStt *pGeo )
{
	if( pGeo == NULL )
		return( -1 );
	
	if( ( pGeo->wAvailableBit & 4 ) == 0 )		// 88 word is not available
		return( -2 );

	if( pGeo->wUDMAFlag & UDMA_MODE_4_SUPPORTED )
		return( 4 );
	else if( pGeo->wUDMAFlag & UDMA_MODE_3_SUPPORTED )
		return( 3 );
	else if( pGeo->wUDMAFlag & UDMA_MODE_2_SUPPORTED )
		return( 2 );
	else if( pGeo->wUDMAFlag & UDMA_MODE_1_SUPPORTED )
		return( 1 );
	else if( pGeo->wUDMAFlag & UDMA_MODE_0_SUPPORTED )
		return( 0 );

	return( -1 );	
}

static int get_udma_set_number( IDEGeometryStt *pGeo )
{
	if( pGeo == NULL )
		return( -1 );
	
	if( ( pGeo->wAvailableBit & 4 ) == 0 )		// 88 word is not available
		return( -2 );

	if( pGeo->wUDMAFlag & UDMA_MODE_4_SET )
		return( 4 );
	else if( pGeo->wUDMAFlag & UDMA_MODE_3_SET )
		return( 3 );
	else if( pGeo->wUDMAFlag & UDMA_MODE_2_SET )
		return( 2 );
	else if( pGeo->wUDMAFlag & UDMA_MODE_1_SET )
		return( 1 );
	else if( pGeo->wUDMAFlag & UDMA_MODE_0_SET )
		return( 0 );

	return( -1 );	
}

int scatter_read_sectors( int nDrv, BCacheEntStt **ppCEntArray, DWORD dwBlock, int nSectors )
{
	int 		nR;
	CHSStt		chs;
	DWORD		dwBasePort;
	
	if( nDrv <= 1 )
		dwBasePort = PRIMARY_HDD_BASE;
	else if( nDrv <= 3 )
		dwBasePort = SECONDARY_HDD_BASE;
	else
		return( -1 );
	
	vLBA2CHS( &chs, dwBlock );
	chs.dwTotalSector = nSectors;
	chs.dwMasterSlave = nDrv % 2;
	chs.dwBasePort	  = dwBasePort;
	
	// 여러 섹터를 캐시 엔트리에 직접 읽어들인다.
 	nR = scatter_read_sectors_by_lba( &chs, ppCEntArray );  
	
	return( nR );
}

int dump_hdd( int nDrv, DWORD dwIndex )
{
	int		nR;
	UCHAR	tempBuff[512];

	kdbg_printf( "dump_hdd( %d, %d )\n", nDrv, dwIndex );

	// read specified sector
	nR = read_sectors( nDrv, dwIndex, 1, tempBuff );
	if( nR < 0 )
	{
		kdbg_printf( "dump_hdd() - failed!\n" );
		return( -1 );
	}

	// dump content
	dump_memory( (DWORD)tempBuff, 256 );

	return( 0 );
}

int ide_auto_detection()
{
	IDEGeometryStt	*pGeo;
	CDGeometryStt	cd_geo;
	char			*title[4];
	int				nR, nI;
	DWORD			base_port[4];
	DWORD			master_slave[4];
	
	kdbg_printf( "IDE HDD Auto detection.\n" );

	title[0] = "Primary   Master";
	title[1] = "Primary   Slave ";
	title[2] = "Secondary Master";
	title[3] = "Secondary Slave ";

	base_port[0] = PRIMARY_HDD_BASE;
	base_port[1] = PRIMARY_HDD_BASE;
	base_port[2] = SECONDARY_HDD_BASE;
	base_port[3] = SECONDARY_HDD_BASE;

	master_slave[0] = 0;
	master_slave[1] = 1;
	master_slave[2] = 0;
	master_slave[3] = 1;
	
	for( nI = 0; nI < 1; nI++ )
	{
		kdbg_printf( "%s: ", title[nI] );

		pGeo = &G_ide_geo[nI];
			
		// find hdd
		nR = read_hdd_geometry_info( pGeo, base_port[nI], master_slave[nI] );
		if( nR >= 0 )
		{
			int nDma, nDmaSupport;

			// 현재 UDMA 모드 설정 상태를 출력한다.
			nDma = get_udma_set_number( pGeo );
			nDmaSupport = get_udma_support_number( pGeo );
			kdbg_printf( "%s UDMA(cur=%d/%d support)\n", pGeo->szModel, nDma, nDmaSupport );
		}
		else
		{// find cd-rom
			nR = read_cd_geometry_info( &cd_geo, base_port[nI], master_slave[nI] );
			if( nR >= 0 )
				kdbg_printf( "%s (%s)\n", cd_geo.szModel, cd_geo.szSerial );
			else
				kdbg_printf( "None\n" );
		}
	}

	return( 0 );
}



