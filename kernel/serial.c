#include "bellona2.h"

static SerialPortStt	*pIrq3Sp, *pIrq4Sp;

static EventStt *get_com_port_event( SerialPortStt *pSR )
{
	if( pSR->dwBase == COM1_BASE )
		return( &com1_event );
	else if( pSR->dwBase == COM2_BASE )
		return( &com2_event );
	else if( pSR->dwBase == COM3_BASE )
		return( &com3_event );
	else
		return( &com4_event );
}

static int save_received_byte_to_sp( SerialPortStt *pSP, UCHAR byTe )
{
	if( pSP->nTotalI >= SERIAL_PORT_BUFF_SIZE )
	{
		kdbg_printf( "save_received_byte_to_sp() - buffer overflow!\n" );
		return( -1 );
	}

	_asm PUSHFD
	_asm CLI

	pSP->i_buff[ pSP->nI2++ ] = byTe;
	if( pSP->nI2 >= SERIAL_PORT_BUFF_SIZE )
		pSP->nI2 = 0;

	pSP->nTotalI++;

	_asm POPFD

	return( 0 );
}

static int ser_irq_handling( SerialPortStt *pSP )
{
	UCHAR		byTe;
	EventStt	*pEvent;

	vReadPort( pSP->dwBase + _IIR, &byTe );

	pEvent = get_com_port_event( pSP );
	switch( byTe & (UCHAR)6 )
	{
	case 0 :	// 00x		RS-232 incoming signal changed.
		break;
	case 2 :	// 01x		buffer empty
		break;
	case 4 :	// 10x		data has been arrived
		// read LSR
		vReadPort( pSP->dwBase + _LSR, &byTe );
		if( byTe & 1 )
		{
			vReadPort( pSP->dwBase + _RBR, &byTe );

			// send key to debugger
			if( is_debugger_active() )
			{
				BKeyStt key;

				memset( &key, 0, sizeof( key ) );
				key.byCode = byTe;

				kdbg_key_input( &key );
			}
			else
			{
				save_received_byte_to_sp( pSP, byTe );
				// wake up the first waiting thread
				awake_the_first_waiting_thread( pEvent, NULL );
			}
		}
		break;
	case 6 :	// 11x	sync error or BREAK
		break;
	}

	return( 0 );
}

int serial_irq( int nIRQ )
{
	int nR;

	if( nIRQ == 3 )
		nR = ser_irq_handling( pIrq3Sp );
	else
		nR = ser_irq_handling( pIrq4Sp );
	
	return( 0 );
}

static int serial_port_open( int nMinor, void *pVoidObj, void *pVDev )
{
	SerialPortStt	*pSR;
	CharDevObjStt	*pObj;
	UCHAR			byTe;
	int				nDivisor;

	if( nMinor <= 0 || nMinor > 4 )
		return( -1 );

	pObj = (CharDevObjStt*)pVoidObj;

	pSR = (SerialPortStt*)MALLOC( sizeof( SerialPortStt ) );
	if( pSR == NULL )
		return( -1 );	// memory allocation failed

	memset( pSR, 0, sizeof( SerialPortStt ) );

	pObj->pPtr	 = pSR;
	pObj->pDev	 = pVDev;
	pObj->nMinor = nMinor;

	// get Base Port
	if( nMinor == 1 )
	{
		pSR->nIrq = 4;
		pIrq4Sp   = pSR;
		pSR->dwBase = COM1_BASE;
	}
	else if( nMinor == 2 )
	{
		pSR->nIrq = 3;
		pIrq3Sp   = pSR;
		pSR->dwBase = COM2_BASE;
	}
	else if( nMinor == 3 )
	{
		pSR->nIrq = 4;
		pIrq4Sp   = pSR;
		pSR->dwBase = COM3_BASE;
	}
	else if( nMinor == 4 )
	{
		pSR->nIrq = 3;
		pIrq3Sp   = pSR;
		pSR->dwBase = COM4_BASE;
	}

	// Stop(1), Parity(None), Data(8)
	vWritePort( pSR->dwBase + _LCR, 3 );

	// SET DTR,RTS,OUT2
	vWritePort( pSR->dwBase + _MCR, 0x0B );

	// Trigger Interrupt when data arrives only.
	vWritePort( pSR->dwBase + _IER, 1 );

	nDivisor = (int)( 115200 / 57600 );

	// set DLAB
	vReadPort( pSR->dwBase + _LCR, &byTe );
	byTe = (UCHAR)( byTe | (UCHAR)0x80 );	
	vWritePort( pSR->dwBase + _LCR, byTe );

	// set baud rate to 9600
	vWritePort( pSR->dwBase + _DLL, (UCHAR)nDivisor );
	vWritePort( pSR->dwBase + _DLM, (UCHAR)( nDivisor >> 8 ) );

	// clear DLAB
	vReadPort( pSR->dwBase + _LCR, &byTe );
	byTe = (UCHAR)( byTe & (UCHAR)0x7F );	
	vWritePort( pSR->dwBase + _LCR, byTe );

	return( 0 );
}

static int serial_port_close( void *pCharDevObj )
{
	CharDevObjStt *pObj;

	pObj = pCharDevObj;

	if( pObj == NULL || pObj->pPtr == NULL )
		return( -1 );

	FREE( pObj->pPtr );
	
		return( 0 );
}

static int serial_port_read( void *pCharDevObj )
{
	SerialPortStt	*pSR;
	CharDevObjStt	*pObj;
	int				nChar;
	EventStt		*pEvent;

	pObj = pCharDevObj;
	pSR  = pObj->pPtr;

	pEvent = get_com_port_event( pSR );

	// endless loop for waiting data to be received.
	for( ;  pSR->nTotalI <= 0;  )
	{
		wait_event( pEvent, 0 );
	}
	
	_asm PUSHFD
	_asm CLI

	// retrieve one char from serial port input buffer
	nChar = (int)pSR->i_buff[ pSR->nI1++ ];
	if( pSR->nI1 >= SERIAL_PORT_BUFF_SIZE )
		pSR->nI1 = 0;
	pSR->nTotalI--;

	_asm POPFD
	
	return( nChar );
}

static int serial_port_write( void *pCharDevObj, UCHAR *pB, int nSize )
{
	int				nI;
	SerialPortStt	*pSR;
	BYTE			byTe;
	CharDevObjStt	*pObj;
	int				nRetry;

	pObj = pCharDevObj;
	pSR  = pObj->pPtr;
	if( pSR == NULL || pObj->nMinor == 0 )
		return( -1 );

	// buffering
	for( nI = 0; nI < nSize && pSR->nTotalO < SERIAL_PORT_BUFF_SIZE; )
	{
		pSR->o_buff[ pSR->nO2++ ] = pB[nI++];
		if( pSR->nO2 >= SERIAL_PORT_BUFF_SIZE )
			pSR->nO2 = 0;

		pSR->nTotalO++;
		if( pSR->nTotalO >= SERIAL_PORT_BUFF_SIZE )
			break;
	}

	// sending
	nRetry = 0;
	for(  ; pSR->nTotalO > 0; nI++ )
	{
		vReadPort( pSR->dwBase + _LSR, &byTe );
		if( byTe & 0x20 )		// THRE?
		{
			// get one char
			byTe = (UCHAR)pSR->o_buff[ pSR->nO1++ ];
			if( pSR->nO1 >= SERIAL_PORT_BUFF_SIZE )
				pSR->nO1 = 0;
			pSR->nTotalO--;

			// send char
			vWritePort( pSR->dwBase + _THR, byTe );
			nRetry = 0;
		}
		else
		{
			nRetry++;
		//	if( nRetry > 20 )
		//		break;
		}			   
	}				   
	
	return( 0 );
}

static int serial_port_ioctl( void *pCharDevObj, void *pV )
{
	return( 0 );
}

int init_serial_port_driver ( int nMajor )
{
	int			nR;
	CharDevStt	serial_port;

	memset( &serial_port, 0, sizeof( serial_port ) );

	strcpy( serial_port.szName, "SERIAL PORT" );
	serial_port.nMajor		= nMajor;
	serial_port.op.open		= serial_port_open ;
	serial_port.op.close	= serial_port_close;
	serial_port.op.read		= serial_port_read ;
	serial_port.op.write	= serial_port_write;
	serial_port.op.ioctl	= serial_port_ioctl;

	// register
	nR = register_chardev( &serial_port );
	
	return( nR );
}
