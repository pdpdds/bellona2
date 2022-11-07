#include "bellona2.h"

// set iopl
int set_iopl( int nIOPL )
{
	_asm {
		PUSH EBX
		
		PUSHFD
		POP EAX
		
		OR  EAX, MASK_IOPL
		XOR EAX, MASK_IOPL	// clear IOPL
		
		MOV EBX, nIOPL
		AND EBX, 3
		SHL EBX, 12
		
		OR  EAX,EBX
		
		PUSH EAX
		POPFD

		POP EBX
	}
	return( 0 );
}

void vInitDebugRegister()
{
	DWORD dwT;

	dwT = (DWORD)DR_SET | (DWORD)DR_GE | (DWORD)DR_LE;
	_asm {
		MOV EAX, dwT
		MOV DR7, EAX
		XOR EAX, EAX
		MOV DR0, EAX
		MOV DR1, EAX
		MOV DR2, EAX
		MOV DR3, EAX
	}			  
}

// 지정된 비트를 On, Off한다.
static int nOnOffDwordBit( DWORD *pDWORD, int nSize, int nIndex, DWORD dwBit )
{
	DWORD	dwT;
	int		nI;


	// 클리어하기 위한 비트를 만든다.
	dwT = 1;
	for( nI = 1; nI < nSize; nI++ )
	{
		dwT = (DWORD)( dwT << 1 );
		dwT |= (DWORD)1;
	}

	// 설정할 비트를 위치로 옮긴다.
	dwT   = (DWORD)( dwT   << nIndex );
	dwBit = (DWORD)( dwBit << nIndex );

	// 비트가 클리어된다.
	pDWORD[0] |=dwT;	  
	pDWORD[0] ^=dwT;	  

	// 새로 비트를 설정한다.
	pDWORD[0] |=dwBit;

	return( 0 );
}

static DWORD dwGetDwordBit( DWORD dwX, int nSize, int nIndex )
{
	DWORD dwT;
	int   nI;

	dwT = 1;
	for( nI = 1; nI < nSize; nI++ )
	{
		dwT  = (DWORD)( dwT << 1 );
		dwT |= (DWORD)1;
	}

	dwT  = (DWORD)( dwT   << nIndex );
	dwT &= dwX;
	dwT  = (DWORD)( dwT   >> nIndex );

	return( dwT );
}	

// set breakpoint
// nNo(0,1,2,3), nRW(Read Write Bit)
int nSetBreakpoint( int nNo, DWORD dwAddr, int nLen, int nRW, int nLocal, int nGlobal )
{
	DWORD dwT;

	if( nNo > 3 )
	{
		kdbg_printf( "nSetBreakpoint: Invalid debug register index(%d)\n", nNo );
		return( -1 );
	}

	// DR7을 얻어온다. 
	_asm {
		MOV EAX, DR7;
		MOV dwT, EAX;
	}
	
	// 비트를 클리어한다.
	nOnOffDwordBit( &dwT, 2, DR_RW0_INDEX  + nNo*4, (DWORD)0 );
	nOnOffDwordBit( &dwT, 2, DR_LEN0_INDEX + nNo*4, (DWORD)0 );
	nOnOffDwordBit( &dwT, 1, DR_L0_INDEX   + nNo*2, (DWORD)0 );
	nOnOffDwordBit( &dwT, 1, DR_G0_INDEX   + nNo*2, (DWORD)0 );	
	
	switch( nNo )
	{
	case 0 : { _asm MOV ECX, dwAddr 
			   _asm MOV DR0, ECX } break;
	case 1 : { _asm MOV ECX, dwAddr 
			   _asm MOV DR1, ECX } break;
	case 2 : { _asm MOV ECX, dwAddr 
			   _asm MOV DR2, ECX } break;
	case 3 : { _asm MOV ECX, dwAddr 
			   _asm MOV DR3, ECX } break;
	}

	nLen &= 3;
	nRW  &= 3;
	// 비트를 설정한다.
	nOnOffDwordBit( &dwT, 2, DR_RW0_INDEX  + nNo*4, (DWORD)nRW     );
	nOnOffDwordBit( &dwT, 2, DR_LEN0_INDEX + nNo*4, (DWORD)nLen    );
	nOnOffDwordBit( &dwT, 1, DR_L0_INDEX   + nNo*2, (DWORD)nLocal  );
	nOnOffDwordBit( &dwT, 1, DR_G0_INDEX   + nNo*2, (DWORD)nGlobal );	

	// DR7에 값을 설정한다.
	_asm {
		PUSH EAX
		MOV  EAX, dwT;
		MOV  DR7, EAX;
		POP  EAX
	}

	return( 0 );
}

// reset breakpoint
int nResetBreakpoint( int nNo )
{
	DWORD dwT, dwAddr;

	if( nNo > 3 )
	{
		kdbg_printf( "error : Invalid debug register index (%d)\n", nNo );
		return( -1 );
	}

	// get DR7
	_asm {
		MOV EAX, DR7;
		MOV dwT, EAX;
	}
	
	dwAddr = 0;
	// clear breakpoint address
	nOnOffDwordBit( &dwT, 2, DR_RW0_INDEX  + nNo*4, (DWORD)0 );
	nOnOffDwordBit( &dwT, 2, DR_LEN0_INDEX + nNo*4, (DWORD)0 );
	nOnOffDwordBit( &dwT, 1, DR_L0_INDEX   + nNo*2, (DWORD)0 );
	nOnOffDwordBit( &dwT, 1, DR_G0_INDEX   + nNo*2, (DWORD)0 );	

	// change DR7
	_asm {
		PUSH EAX
		MOV  EAX, dwT;
		MOV  DR7, EAX;
		POP  EAX
	}

	switch( nNo )
	{
	case 0 : { _asm MOV ECX, dwAddr 
			   _asm MOV DR0, ECX } break;
	case 1 : { _asm MOV ECX, dwAddr 
			   _asm MOV DR1, ECX } break;
	case 2 : { _asm MOV ECX, dwAddr 
			   _asm MOV DR2, ECX } break;
	case 3 : { _asm MOV ECX, dwAddr 
			   _asm MOV DR3, ECX } break;
	}

	return( 0 );
}

void vDispDebugRegister()
{
	DWORD dwDR0, dwDR1, dwDR2, dwDR3, dwDR6, dwDR7;
	DWORD dwA, dwB;

	_asm {
		MOV EAX,     DR7;
		MOV dwDR7,   EAX;
		MOV EAX,     DR6;
		MOV dwDR6,   EAX;
		MOV EAX,     DR3;
		MOV dwDR3,   EAX;
		MOV EAX,     DR2;
		MOV dwDR2,   EAX;
		MOV EAX,     DR1;
		MOV dwDR1,   EAX;
		MOV EAX,     DR0;
		MOV dwDR0,   EAX;
	}
	
	// DR7
	dwA = dwGetDwordBit( dwDR7, 2, 18 );		// LEN3
	dwB = dwGetDwordBit( dwDR7, 2, 16 );		// RW3
	kdbg_printf( "LEN/RW0(%d/%d) ", dwA, dwB );
	dwA = dwGetDwordBit( dwDR7, 2, 22 );		// LEN3
	dwB = dwGetDwordBit( dwDR7, 2, 20 );		// RW3
	kdbg_printf( "1(%d/%d) ", dwA, dwB );
	dwA = dwGetDwordBit( dwDR7, 2, 26 );		// LEN3
	dwB = dwGetDwordBit( dwDR7, 2, 24 );		// RW3
	kdbg_printf( "2(%d/%d) ", dwA, dwB );
	dwA = dwGetDwordBit( dwDR7, 2, 30 );		// LEN3
	dwB = dwGetDwordBit( dwDR7, 2, 28 );		// RW3
	kdbg_printf( "3(%d/%d) ", dwA, dwB );

	dwA = dwGetDwordBit( dwDR7, 1, 13 );		// GD
	kdbg_printf( "GD(%d) ", dwA );

	dwA = dwGetDwordBit( dwDR7, 1, 9 );		// GE
	dwB = dwGetDwordBit( dwDR7, 1, 8 );		// LE
	kdbg_printf( "GE(%d) LE(%d)\n", dwA, dwB );

	dwA = dwGetDwordBit( dwDR7, 1, 1 );		// G0
	dwB = dwGetDwordBit( dwDR7, 1, 0 );		// L0
	kdbg_printf( "GL0(%d/%d) ", dwA, dwB );
	dwA = dwGetDwordBit( dwDR7, 1, 3 );		// G1
	dwB = dwGetDwordBit( dwDR7, 1, 2 );		// L1
	kdbg_printf( "1(%d/%d) ", dwA, dwB );
	dwA = dwGetDwordBit( dwDR7, 1, 5 );		// G2
	dwB = dwGetDwordBit( dwDR7, 1, 4 );		// L2
	kdbg_printf( "2(%d/%d) ", dwA, dwB );
	dwA = dwGetDwordBit( dwDR7, 1, 7 );		// G3
	dwB = dwGetDwordBit( dwDR7, 1, 6 );		// L3
	kdbg_printf( "3(%d/%d) ", dwA, dwB );

	// DR6
	kdbg_printf( "BT(%d) BS(%d) BD(%d) B0(%d) B1(%d) B2(%d) B3(%d)\n",
		(DWORD)( dwDR6 & DR_BT ), 
		(DWORD)( dwDR6 & DR_BS ), 
		(DWORD)( dwDR6 & DR_BD ), 
		(DWORD)( dwDR6 & DR_B0 ), 
		(DWORD)( dwDR6 & DR_B1 ), 
		(DWORD)( dwDR6 & DR_B2 ), 
		(DWORD)( dwDR6 & DR_B3 ) ); 
		
	// DR0,1,2,3
	kdbg_printf( "DR0=%08X DR1=%08X DR2=%08X DR3=%08X DR6=%08X DR7=%08X\n", dwDR0, dwDR1, dwDR2, dwDR3, dwDR6, dwDR7 );
}