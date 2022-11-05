/*
�÷��� ��ũ�� �� ���͸� �д� ����� 0x66�̴�.  0x66�� �̿��Ͽ� �� ���;� �÷��Ǹ� �о���� ������
�� Ʈ���� �д� �������� 18�� ����� �����ؾ� �Ѵ�.  �̷������� 1.44M ��ũ �� ���� ��� ���� ���
36�� ������ �ҿ�ȴ�.  ������ �볳�� �� ���� �ð��̴�. �׷��ٸ� ������ DISKCOPY ����� ��� ������
�ð��� �ɸ��°� üũ�� ���Ҵ��� �� ���� �дµ� 35�� ������ �ɷȴ�.  �ᱹ �÷��� ��ũ�� ���� ��
�� ���;� �������� �� Ʈ���� �о�� �Ѵٴ� ����� �����.  (�̰� ���� �𸣰� �־�����?)
��ũ�� �� Ʈ���� �д� ��ɿ��� 0x42(Read Track)�� 0xE6(Read Normal Data)�� �־���.
ó���� 0x42�� �̿��ߴµ� ù ��° ���¹���Ʈ�� ���� 2 ��Ʈ���� 01�� ����ϰ� ������ �߻��Ǿ���.
0x42�� 0xE6�� �ٸ��� ������ ID�ʵ忡 ����Ǿ� �ִ� ���� ���� ��ȣ(Actual Sector Number)�� �˻�����
�ʴ´ٰ� ����Ǿ� �ִ�.  ���ٿ� PC BIOS������ 0x42��� 0xE6�� ����Ѵٰ� ��޵Ǿ� �ִ�.  
������ 0xE6�� �̿��Ͽ����� ����ϰ� �߻��Ǵ� ������ ���� �߻����� �ʾҰ� ��ũ�� ���� �� 
���� �Ҹ��� DISKCOPY�� �װͰ� ���� ������ '�簢�簢~~' �Ҹ��� ����.  
*/

#include "bellona2.h"

static int nPrevTrack = -1;
static int nPrevSide  = -1;

// fdd irq 6 handler
_declspec(naked) void fdd_handler()
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

	inc_event_count( &fdd_event );
	vSendEOI( 6 );

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

// FDD ���͸� ����.  (����) IRQ6 Handler�� ������ �ʰ� Motor�� Off ��Ű�� �ȵȴ�.
// Motor�� Off���ѵ� IRQ6�� �߻��Ѵ�.
int fdd_motor_off()
{
	vWritePort( (DWORD)FDD_DIGITAL_OUTPUT, (DWORD)0x00 );
	return( 0 );
}


// FDC DATA��Ʈ�� ����
static int write_fdc_data_port( DWORD dwX )
{
	UCHAR byTe = 3;
	int nI;

	// FDC�� ���� ��Ʈ�� �о �����غ� �Ǿ��� ������ ����Ѵ�.
	for( nI = 0; nI < 100; nI++)
	{
		vReadPort( (DWORD)FDD_STATUS_PORT, &byTe );
		byTe = (BYTE)( byTe & 0xC0 );   // 2002-05-24
		if( byTe & 0x80 )
			break;
		
		kdelay( 300 );
	}
	if( nI == 100 )
	{
		kdbg_printf( "FDD port write error!\n" );
		return( -1 );
	}			  

	// FDC ����Ÿ ��Ʈ�� ���� ������ �ϴ� ����Ʈ�� ����Ѵ�.
	vWritePort( (DWORD)FDD_DATA_PORT, dwX );					

	return( 0 );
}
// read fdc data port
static int read_fdc_data_port( UCHAR *pByte )
{
	UCHAR byTe = 0;
	int   nI;

	pByte[0] = 0;
	
	// FDC�� ���� ��Ʈ�� �о �����غ� �Ǿ��� ������ ����Ѵ�.
	for( nI = 0; ( (UCHAR)byTe & (UCHAR)0x80 ) == 0  && nI < 100; nI++)
	{
		vReadPort( (DWORD)FDD_STATUS_PORT, &byTe );
		
		kdelay( 300 );
	}
	if( nI == 100 )
	{
		kdbg_printf( "FDD Read port error.\n" );
		return( -1 );
	}

	// FDC ����Ÿ ��Ʈ���� �д´�.
	vReadPort( (DWORD)FDD_DATA_PORT, pByte );					

	return( 0 );
}

// reset fdd
int fdd_reset()
{
	UCHAR r[2];

	write_fdc_data_port( 0x80 );
	
	read_fdc_data_port( &r[0] );
	read_fdc_data_port( &r[1] );

	return( 0 );
}

// recalibrate fdd
static int fdd_recalibrate( int nDrv )
{
	int nR;

	clear_event_count( &fdd_event );

	write_fdc_data_port( 0x07 );
	write_fdc_data_port( nDrv );

	// this guy invokes irq 6
	nR = wait_event( &fdd_event, 2000 );

 	return( nR );
}

// turn off fdd motor
static int fdd_motor_on( int nDrv, int nDelayTick )
{
	if( nDrv  ==  0 )				// A 
		vWritePort( (DWORD)FDD_DIGITAL_OUTPUT, (DWORD)0x1C );	 // 0001 11 00
    else							// B 
		vWritePort( (DWORD)FDD_DIGITAL_OUTPUT, (DWORD)0x2D );    // 0010 11 01
					  	
	// delay just a moment
	kdelay( nDelayTick );
	
	return( 0 );
}

// read the result of operation
static int read_fdd_state( char *pState, int nX )
{
	int nI, nR;

	write_fdc_data_port( 0x08 );  // send Sense Command
	for( nI = 0; nI < nX; nI++ )
		nR = read_fdc_data_port( &pState[nI] );

	return( 0 );
}

// initialize DMA
static int init_dma_for_fdd( DWORD dwAddr, int nBytes, DWORD dwMode )
{	
	DWORD dwTemp = dwAddr;
																		  
	vWritePort( (DWORD)0x0C, dwMode );  														    //	10(write)
	vWritePort( (DWORD)0x0B, dwMode ); // 01(Single mode select) 0(addr inc select) 0(Auto init enable) 01(read) 10(channel 2) )  

	// ��ũ ������ �޸� �ּ� 20��Ʈ�� ������.
	dwTemp = (DWORD)dwAddr;
	vWritePort( (DWORD)4, dwTemp );  	   // ���� 8��Ʈ���� �����Ѵ�.
	dwTemp = (DWORD)(dwTemp >> 8 );   
	vWritePort( (DWORD)4, dwTemp );		   // ���� 8��Ʈ�� �����Ѵ�.
	dwTemp = (DWORD)(dwTemp >> 8 );   
	vWritePort( (DWORD)0x81, dwTemp );	   // ���� 4��Ʈ�� �����Ѵ�.

	// ������ ����Ʈ��
	dwTemp = (DWORD)(nBytes -1);		  // ������ ����Ʈ �� -1 
	vWritePort( (DWORD)5, dwTemp );       // (����)
	dwTemp = (DWORD)(dwTemp >> 8 );
	vWritePort( (DWORD)5, dwTemp );       // (����)
	// DMA Ȱ��ȭ
	vWritePort( (DWORD)10, (DWORD)2 );

	return( 0 );
}	

// set fdd to ready for io operation
int fdd_set_ready( int nDrv )
{
	int nR;

	// turn on fdd motor
	fdd_motor_on( nDrv, 1000 );

	// recalibrate
	nR = fdd_recalibrate( nDrv );

	kdelay( 500 );

	nPrevTrack = -1;

	return( nR );
}

typedef struct {
	int		nByte;
	int		nIndex;
	char	*pStr;
} FDDStatusStt;

static FDDStatusStt fdd_status[] = {
	{ 0, 5, "Seek End."					 },
	{ 0, 4, "Drive Faults."				 },
	{ 0, 3, "Drive Not Ready."			 },
	{ 1, 7, "End of Cylinder."			 },
	{ 1, 5, "Data Error"				 },
	{ 1, 4, "Time-out."					 },
	{ 1, 2, "No Data,"					 },
	{ 1, 1, "Not Writable."				 },
	{ 1, 0, "No Address Mark."			 },
	{ 2, 6, "Deleted Address Mark."		 },
	{ 2, 5, "CRC Error."				 },
	{ 2, 4, "Wrong Cylinder."			 },
	{ 2, 3, "Seek Equal."				 },
	{ 2, 2, "Seek Error."				 },
	{ 2, 1, "Bad Cylinder."				 },
	{ 2, 0, "Invalid Data Address Mark." },

	{ -1, -1, NULL }
};

static int read_fdd_status( BYTE *pR )
{
	int		nI;
	BYTE	byTe;

	// ���� 7����Ʈ�� �д´�. 
	for( nI = 0; nI < 7; nI++ )
		read_fdc_data_port( &pR[nI] );

	// ��� ��Ʈ���� ����Ѵ�. 
	for( nI = 0; fdd_status[nI].nByte != -1; nI++ )
	{
		byTe = 1;

		// FDD ���� ��Ʈ���� ����Ѵ�. 
		byTe = (BYTE)( byTe << fdd_status[nI].nIndex );
		if( byTe & pR[ fdd_status[nI].nByte ] )
			kdbg_printf( "FDD_STATUS : %s\n", fdd_status[nI].pStr );
	}	

	// Interrupt Code B6,B7
	if( pR[0] & (UCHAR)0xC0 )
		return( -1 );
			 
	return( 0 );
}				


// fdd track i/o (����)
static int fdd_internal_track_io( int nDrv, int nTrack, int nSide, char *pBuff, char chType )
{
	UCHAR result[7];
	int   nR, nRetry;
	DWORD dwHeadDrv, dwDMAInit, dwFDDCmd;

	dwHeadDrv =  (DWORD)nDrv;
	dwHeadDrv += (DWORD)( nSide << 2 );

	nRetry = 0;

RETRY:
//	if( nPrevTrack != nTrack || nPrevSide != nSide )
	{
		// 2002-06-13 (motor on�� ���� �� ���� recalibrate�ϵ��� ����.)
		//fdd_recalibrate( nDrv );
		
		// clear fdd event
		clear_event_count( &fdd_event );

		// seek track
		write_fdc_data_port( (DWORD)0x0F	);		// seek command
		write_fdc_data_port( dwHeadDrv		);		// head, drive
		write_fdc_data_port( (DWORD)nTrack	);		// track

		// wait event
		nR = wait_event( &fdd_event, 2000 );
		if( nR < 0 )
		{
			//kdbg_printf( "fdd_internal_track_io( %c : %d, %d ) - seek track time out!\n", chType, nSide, nTrack );
			nPrevSide = nPrevTrack = -1;
			return( -1 );
		}
		nPrevTrack = nTrack;
		nPrevSide  = nSide;
	}

	if( chType == 'o' || chType == 'O' )
	{	// FDD write
		dwFDDCmd  = 0xC5;		
		dwDMAInit = 0x4A;
		// copy data to dma buffer if the operation type is writing.
		memcpy( (UCHAR*)FDD_DMA_BUFF_ADDR, pBuff, 512*18 ); 
	}
	else
	{	// FDD Track read
		dwFDDCmd  = 0xE6;		
		dwDMAInit = 0x46;
	}

	// initialize DMA
	init_dma_for_fdd( (DWORD)FDD_DMA_BUFF_ADDR, 18 * 512, dwDMAInit );
				 	
	// clear fdd event
	clear_event_count( &fdd_event );

	// command sequence
	write_fdc_data_port( dwFDDCmd		);	// command
	write_fdc_data_port( dwHeadDrv		);	// head, drive
	write_fdc_data_port( (DWORD)nTrack	);	// track
	write_fdc_data_port( (DWORD)nSide	);	// head
	write_fdc_data_port( (DWORD)1		);	// sector
	write_fdc_data_port( (DWORD)2		);	// bytes per sector ( 512 / 256 = 2 )
	//write_fdc_data_port( (DWORD)0x3F	);	// sectors per track
	write_fdc_data_port( (DWORD)0x12	);	// sectors per track
	write_fdc_data_port( (DWORD)0x1B	);	// inter sector gap
	write_fdc_data_port( (DWORD)0xFF	);	// data length

	// wait for irq 6 
	nR = wait_event( &fdd_event, 2000 );
	if( nR < 0 )
	{
		//kdbg_printf( "fdd_internal_track_io( %c : %d, %d ) - read track time out!\n", chType, nSide, nTrack );
		return( -1 );
	}

	// ���°� 7����Ʈ�� �д´�. 
	nR = read_fdd_status( result );
	if( nR < 0 )
	{
		nRetry++;

		if( nRetry >= 3 )
			return( -1 );
		else
		{
			nPrevSide = nPrevTrack = -1;
			// FDD�� ���͸� �� �� �ʱ�ȭ�Ѵ�.
			fdd_motor_off();
			fdd_set_ready( nDrv );
			goto RETRY;
		}
	}

	// copy data from dma buffer if the operation type is reading.
	if( chType == 'I' || chType == 'i' )
		memcpy( pBuff, (UCHAR*)FDD_DMA_BUFF_ADDR, 512*18 ); 

	return( 0 );
}


static TimeOutStt	*pFddMotorTimeout = NULL;
static int			fdd_motor_flag    = 0;
// it turns off the fdd motor after 5 seconds
int	fdd_motor_off_callback( DWORD dwDrv )
{
	fdd_motor_off( (int)dwDrv );

	if( fdd_motor_flag != 0 )
	{
		fdd_motor_flag = 0;
		if( pFddMotorTimeout != NULL )
		{
			sub_from_timeout_list( pFddMotorTimeout );
			pFddMotorTimeout = NULL;
		}
	}	

	return(0);
}

// fdd track i/o
static int fdd_track_io( int nDrv, int nTrack, int nSide, char *pBuff, char chType )
{
	int nR;

	if( fdd_motor_flag == 0 )
	{	// set fdd to ready
		fdd_set_ready( nDrv );

		// allocate timeout object
		pFddMotorTimeout = alloc_timeout();
		if( pFddMotorTimeout != NULL )
		{
			pFddMotorTimeout->dwTick          = 8000;
			pFddMotorTimeout->pCallBack		  = fdd_motor_off_callback;
			pFddMotorTimeout->dwCallBackParam = (DWORD)nDrv;
			pFddMotorTimeout->nReady          = 1;
		
			// add timeout to sys_timer list
			add_to_timeout_list( pFddMotorTimeout );

			fdd_motor_flag = 1;
		}

		// recalibrate fdd
		fdd_recalibrate( nDrv );
	}
	else
	{	// init counter
		if( pFddMotorTimeout != NULL )
		{
			pFddMotorTimeout->dwTick    = 3000;
			pFddMotorTimeout->dwCurTick = 0;
		}
	}

	// call internal function
	nR = fdd_internal_track_io( nDrv, nTrack, nSide, pBuff, chType );

	// maybe timeout object allocation failed.
	if( fdd_motor_flag == 0 )
		fdd_motor_off();

	return( nR );
}				 

// read fdd track
int fdd_read_track( int nDrv, int nTrack, int nSide, char *pBuff )
{
	int nR;
	nR = fdd_track_io( nDrv, nTrack, nSide, pBuff, 'I' );
	return( nR );
}

// write fdd track
int fdd_write_track( int nDrv, int nTrack, int nSide, char *pBuff )
{
	int nR;

	//nR = fdd_track_io( nDrv, nTrack, nSide, pBuff, 'O' );
	return( -1 );

	
	return( nR );
}

// fdd read test
static char	fdd_test_buff[18*512];

int fdd_write_test()
{
	int nR;

	memset( fdd_test_buff, 3, sizeof( fdd_test_buff ) );

	nR = fdd_write_track( 0, 79, 1, fdd_test_buff );

	return( 0 );
}

int fdd_read_test()
{
	int		nR, nTrack;
	
	nR = fdd_set_ready( 0 );	// 0 - A
	if( nR < 0 )
	{
		kdbg_printf( "fdd_read_test() - fdd_set_ready() returned failed!\n" );
		return( -1 );
	}

	for( nTrack = 0; nTrack < 80; nTrack++ )
	{
		// side 0 
		kdbg_printf( "track( %2d ) side( 0 )" );
		nR = fdd_read_track( 0, nTrack, 0, fdd_test_buff );
		if( nR < 0 )
			kdbg_printf( "error\n" );
		else
			kdbg_printf( "\r                              \r" );

		// side 1
		kdbg_printf( "track( %2d ) side( 1 ) - " );
		nR = fdd_read_track( 0, nTrack, 1, fdd_test_buff );
		if( nR < 0 )
			kdbg_printf( "error\n" );
		else
			kdbg_printf( "\r                              \r" );
	}

	nR = fdd_motor_off();

	return( 0 );
}
