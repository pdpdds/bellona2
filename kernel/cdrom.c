#include "bellona2.h"

static int chk_cd_busy( DWORD dwBasePort )
{
	int		nI;
	UCHAR	byTe;

	// check busy bit
	for( nI = 0; nI < 3; nI++ )
	{	
		vReadPort( (DWORD)( CD_STATUS + dwBasePort ), &byTe );
		// busy bit must be 0
		if((  byTe & (UCHAR)0x80 ) == 0 )
			return( 0 );
		
		kdelay(30);
	}

	return( -1 );
}

int read_cd_geometry_info( CDGeometryStt *pGeo, DWORD dwBasePort, DWORD dwMasterSlave )
{
	int		nI, nR;
	UCHAR  *pB;
	DWORD	dwTemp;
		
	pB = (UCHAR*)pGeo;

	memset( pGeo, 0, sizeof( CDGeometryStt ) );

	// check busy bit
	nR = chk_cd_busy( dwBasePort );
	if( nR < 0 )
		return( -1 );

	// clear event count
	if( dwBasePort == PRIMARY_HDD_BASE )
		clear_event_count( &primary_ide_event );
	else
		clear_event_count( &secondary_ide_event );

	// send IDENTIFY Command
	dwTemp    = (DWORD)( dwMasterSlave << 4 ) | (DWORD)0xE0;  // 1110 0000  LBA Mode
	vWritePort( (DWORD)( dwBasePort + CD_DRV_SELECT ), dwTemp );
	vWritePort( (DWORD)( CD_COMMAND + dwBasePort ), (DWORD)0xA1 );

	// wait interrupt
	if( dwBasePort == PRIMARY_HDD_BASE )
		nR = wait_event( &primary_ide_event, 250 );
	else
		nR = wait_event( &secondary_ide_event, 250 );
	
	// read 512 bytes
	for( nI = 0; nI < 512/2; nI++ )		
		vReadPortWord( (DWORD)( dwBasePort+CD_DATA), (UINT16*)&pB[nI*2] );

	// check config word  (bit8-12 must be 01001)
	if( (pGeo->wConfig & (UINT16)0x1F00 ) != (UINT16)0x0500 )
		return( -1 );
	
	// swap upper and lower	byte
	vSwapUINT16Array( pGeo->model,  20 );
	vSwapUINT16Array( pGeo->serial, 10 );

	memcpy( pGeo->szSerial, pGeo->serial, 20  );
	memcpy( pGeo->szModel,  pGeo->model,  40 );
	pGeo->szSerial[20] = pGeo->szModel[40] = 0;

	vCutAppendedSpace( pGeo->szSerial );
	vCutAppendedSpace( pGeo->szModel );

	return( 0 );
}

// read CD-ROM capacity
int read_cd_capacity( int nDrv, DWORD *pTotalBlock, DWORD *pBlockSize )
{
	int		nI, nR;
	DWORD	dwTemp;
	DWORD	dwBasePort, dwMasterSlave;
	UCHAR	cmd[12], byStatus;
	UINT16	*pTB, *pBS;
	
	pTotalBlock[0] = pBlockSize[0] = 0;

	switch( nDrv )	
	{
	case 0 : dwBasePort = (DWORD)PRIMARY_HDD_BASE;	 dwMasterSlave = 0; break;
	case 1 : dwBasePort = (DWORD)PRIMARY_HDD_BASE;   dwMasterSlave = 1; break;
	case 2 : dwBasePort = (DWORD)SECONDARY_HDD_BASE; dwMasterSlave = 0; break;
	case 3 : dwBasePort = (DWORD)SECONDARY_HDD_BASE; dwMasterSlave = 1; break;	
	}
	
	// check busy bit
	nR = chk_cd_busy( dwBasePort );
	if( nR < 0 )
		return( -1 );

	// clear event count
	if( dwBasePort == PRIMARY_HDD_BASE )
		clear_event_count( &primary_ide_event );
	else
		clear_event_count( &secondary_ide_event );
	
	// send Command
	dwTemp    = (DWORD)( dwMasterSlave << 4 ) | (DWORD)0xE0;  // 1110 0000  LBA Mode
	vWritePort( (DWORD)( dwBasePort + CD_DRV_SELECT ), dwTemp );
	vWritePort( (DWORD)( CD_COMMAND + dwBasePort ), (DWORD)0xA0 );

	// send command packet
	memset( cmd, 0, sizeof( cmd ) );
	cmd[0] = (UCHAR)0x25;
	for( nI = 0; nI < sizeof( cmd ); nI++ )
		vWritePort( (DWORD)( CD_DATA + dwBasePort ), (DWORD)cmd[nI] );
	
	// wait interrupt
	if( dwBasePort == PRIMARY_HDD_BASE )
		nR = wait_event( &primary_ide_event, 250 );
	else
		nR = wait_event( &secondary_ide_event, 250 );
	if( nR < 0 )
	{
		kdbg_printf( "error : cd time out\n" );
		return( -1 );
	}

	// read status !!!  (this command line is indispensable)
	vReadPort( (DWORD)( dwBasePort+CD_STATUS), &byStatus );
  	
	pTB = (UINT16*)pTotalBlock;
	pBS = (UINT16*)pBlockSize;
	vReadPortWord( (DWORD)( dwBasePort+CD_DATA), &pTB[1] );	// Total Block H
	vReadPortWord( (DWORD)( dwBasePort+CD_DATA), &pTB[0] );	// Total Block L
	vReadPortWord( (DWORD)( dwBasePort+CD_DATA), &pBS[1] );	// Block Size  H
	vReadPortWord( (DWORD)( dwBasePort+CD_DATA), &pBS[0] ); // Block Size  L

	vSwapUINT16Array( pTB,  2 );
	vSwapUINT16Array( pBS,  2 );

	// read status
	vReadPort( (DWORD)( dwBasePort+CD_STATUS), &byStatus );
	
	return( 0 );
}
