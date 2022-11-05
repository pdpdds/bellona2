#include "bellona2.h"

//-------------------------------//
// Function to access GDTR Value //
//-------------------------------//
void   vClearGDTR   ( GDTRStt *pG )
{
    memset( pG, 0, sizeof( GDTRStt ) );
}
UINT16 wSetGDTRSize ( GDTRStt *pG, UINT16 wSize )
{
    UINT16 wOrg;

    wOrg = wGetGDTRSize( pG );
    pG->wSize = wSize;
    return( wOrg );
}
ULONG  dwSetGDTRAddr( GDTRStt *pG, ULONG  dwAddr )
{
    ULONG dwOrg;

    dwOrg = dwGetGDTRAddr( pG );
    pG->dwAddr = dwAddr;

    return( dwOrg );
}
UINT16 wGetGDTRSize ( GDTRStt *pG )
{
    return( pG->wSize );
}

ULONG  dwGetGDTRAddr( GDTRStt *pG )
{
    return( pG->dwAddr );
}

//-------------------------------------------------//
/////////  Function to access Descriptors  //////////
//-------------------------------------------------//
void vClearDescriptor    ( DescriptorStt *pD )
{
    memset( pD, 0, sizeof( DescriptorStt ) );
}
void vCopyDescriptor     ( DescriptorStt *pDest, DescriptorStt *pSrc )
{
    memcpy( pDest, pSrc, sizeof( DescriptorStt ) );
}

ULONG dwSetDescriptorSize  ( DescriptorStt *pD, ULONG dwSize )
{
    UCHAR  s[4];
    ULONG  dwOrg;

    dwOrg = dwGetDescriptorSize( pD );

    memcpy( s, &dwSize, 4 );

    s[2] &= 0x0F;
    pD->size_0 = s[0];
    pD->size_1 = s[1];
    pD->gdou_size |= s[2];

    return( dwOrg );
}

ULONG dwSetDescriptorAddr  ( DescriptorStt *pD, ULONG dwAddr )
{
    UCHAR  s[4];
    ULONG  dwOrg;

    dwOrg = dwGetDescriptorAddr( pD );

    memcpy( s, &dwAddr, 4 );

    pD->addr_0 = s[0];
    pD->addr_1 = s[1];
    pD->addr_2 = s[2];
    pD->addr_3 = s[3];

    return( dwOrg );
}

// ACCESS Byte의 1번 비트가 TSS의 BUSY Bit이다.
void vSetDescriptorBusyBit( DescriptorStt *pD, UCHAR byBusy )
{	
	UCHAR	byT;

	// 일단 BusyBit를 Clear한다.
	byT = 2;
	pD->byAccess |= byT;
	pD->byAccess ^= byT;	
	
	if( byBusy == 0 )
		byT = 0;
	// 설정된 비트로  OR한다.
	pD->byAccess |= byT;
}	

UCHAR bySetDescriptorAccess( DescriptorStt *pD, UCHAR byAccess )
{
    UCHAR byR;
    byR = byGetDescriptorAccess( pD );
    pD->byAccess = byAccess;
    return( byR );
}
UCHAR bySetDescriptorGDOU  ( DescriptorStt *pD, UCHAR byGDOU )
{
    UCHAR byR;

    byR = byGetDescriptorGDOU( pD );
    byGDOU &= 0xF0;
    pD->gdou_size &= 0x0F;  // Clear Org GDOU Value
    pD->gdou_size |= byGDOU;

    return( byR );
}

ULONG dwGetDescriptorSize  ( DescriptorStt *pD )
{
    ULONG dwR;
    UCHAR  s[4];

    s[3] = 0;
    s[2] = pD->gdou_size;
    s[2] &= 0x0F;
    s[1] = pD->size_1;
    s[0] = pD->size_0;

    memcpy( &dwR, s, 4 );

    return( dwR );
}
ULONG dwGetDescriptorAddr  ( DescriptorStt *pD )
{
    ULONG dwR;
    UCHAR  s[4];

    s[0] = pD->addr_0;
    s[1] = pD->addr_1;
    s[2] = pD->addr_2;
    s[3] = pD->addr_3;

    memcpy( &dwR, s, 4 );

    return( dwR );
}
UCHAR byGetDescriptorAccess( DescriptorStt *pD )
{
    return( pD->byAccess );
}
UCHAR byGetDescriptorGDOU  ( DescriptorStt *pD )
{
    UCHAR byR;

    byR = pD->gdou_size;
    byR &= 0xF0;

    return( byR );
}

//----------------------------------------------------------//
/////////  Function to access Callgate Descriptors  //////////
//----------------------------------------------------------//
ULONG dwSetCallGeteOffset( CallGate32Stt *pC, DWORD dwOffset )
{
    ULONG dwR;
    UCHAR offs[4];

    dwR = dwGetCallGate32Offset( pC );

    memcpy( offs, &dwOffset, 4 );

    pC->offset_0 = offs[0];
    pC->offset_1 = offs[1];
    pC->offset_2 = offs[2];
    pC->offset_3 = offs[3];

    return( dwR );
}
ULONG dwGetCallGate32Offset( CallGate32Stt *pC )
{
    return( 0 );
}

void vMakeGDTDescriptor( int nIndex, DWORD dwAddr, DWORD dwSize, UCHAR byAccess, UCHAR byGDOU )
{
	DescriptorStt *pD;

	pD = (DescriptorStt*)( (DWORD)&gdt + (DWORD)nIndex );
	vClearDescriptor( pD );

	dwSetDescriptorSize  ( pD, dwSize   );
	dwSetDescriptorAddr  ( pD, dwAddr   );
	bySetDescriptorAccess( pD, byAccess );
	bySetDescriptorGDOU  ( pD, byGDOU   );
}

void vMakeTaskGate_in_GDT( int nGateOffset, int nSelectorOffset )
{
	CallGate32Stt *pD;

	pD = (CallGate32Stt*)( (DWORD)&gdt + (DWORD)nGateOffset );
	vClearDescriptor( (DescriptorStt*)pD );

	// 셀렉터 설정.
	pD->wSelector = (UINT16)( nSelectorOffset );
	pD->type = (UCHAR)0xE5;								// P DPL 0  0101
}

void vMakeTaskGate_in_IDT( int nGateOffset, int nSelectorOffset )
{
	CallGate32Stt *pD;

	pD = (CallGate32Stt*)( (DWORD)&idt + (DWORD)nGateOffset );
	vClearDescriptor( (DescriptorStt*)pD );

	// 셀렉터 설정.
	pD->wSelector = (UINT16)( nSelectorOffset );
	pD->type = (UCHAR)0xE5;								// P DPL 0  0101
}
////////////////////////////////////////////////////////////////////////////////////////////////
static char *pGetGDOUStr( char *pS, UCHAR byGDOU )
{
	pS[0] = 0;
	
	if( byGDOU & (UCHAR)0x80 )	strcat( pS, " G" ); else strcat( pS, " -" );
	if( byGDOU & (UCHAR)0x40 )	strcat( pS, "D" ); else strcat( pS, "-" );
	if( byGDOU & (UCHAR)0x20 )	strcat( pS, "O" ); else strcat( pS, "-" );
	if( byGDOU & (UCHAR)0x10 )	strcat( pS, "U" ); else strcat( pS, "-" );

	return( pS );
}

char *pGetDescriptorTypeStr( char *pS, UCHAR byAccess, UCHAR byGDOU )
{
	char	szGDOU[32];
	pS[0] = 0;
	// S비트가 1(CODE,DATA Segment)인지 확인한다.
	if( byAccess & (UCHAR)0x10 )
	{	
		if( byAccess & (UCHAR)0x08 )	// C/D비트가 1 이면 CODE Segment이다.
		{
			strcat( pS, "CODE" );
			strcat( pS, pGetGDOUStr( szGDOU, byGDOU ) );		// CODE, DATA일 때에만 GDOU를 출력한다.
			if( byAccess & (UCHAR)0x04 )
				strcat( pS,  " Conforming" );
			if( byAccess & (UCHAR)0x02 )
				strcat( pS, " Readable" );
			else
				strcat( pS, " No-Read" );
		}
		else							// DATA Segment
		{
			strcat( pS, "DATA" );
			strcat( pS, pGetGDOUStr( szGDOU, byGDOU ) );		// CODE, DATA일 때에만 GDOU를 출력한다.
			if( byAccess & (UCHAR)0x04 )
				strcat( pS, " Expand Down" );
			else
				strcat( pS, " Expand Up" );
			if( byAccess & (UCHAR)0x02 )
				strcat( pS,  " Writable" );
		}

		if( byAccess & (UCHAR)0x01 )
			strcat(  pS, " Accessed" );
	}							   
	else		// SYSTEM Segment
	{
		switch( (UCHAR)( byAccess & (UCHAR)0x0F ) )
		{
		case  1 : strcat( pS, "TSS16"		);	break;
		case  2 : strcat( pS, "LDT"			);	break;
		case  3 : strcat( pS, "BUSY TSS16"	);	break;
		case  4 : strcat( pS, "CALLGATE16"	);	break;
		case  5 : strcat( pS, "TASKGATE"	);	break;
		case  6 : strcat( pS, "INTGATE16"	);	break;
		case  7 : strcat( pS, "TRAPGATE616"	);	break;
		case  8 : strcat( pS, "Reserved"	);	break;
		case  9 : strcat( pS, "TSS32"		);	break;
		case  11: strcat( pS, "BUSY TSS32"	);	break;
		case  12: strcat( pS, "CALLGATE32"	);	break;
		case  14: strcat( pS, "INTGTATE32"	);  break;
		case  15: strcat( pS, "TRAPGATE32"	);  break;
		default : strcpy( pS, "UNKNOWN"		);	break;
		}	
	}	

	return( pS );
}

// GDT Table을 출력한다.
void vDispGDT( DescriptorStt *pD, int nTotal )
{
	int		nI;
	DWORD	dwSize, dwAddr;
	UCHAR	byAccess, byDPL, byP, byGDOU;
	char	szT[128];

	kdbg_printf( "      Address    Size    P DPL Type\n" );

	for( nI = 0; nI < nTotal; nI++ )
	{
		dwAddr   = dwGetDescriptorAddr  ( &pD[nI] );
		dwSize   = dwGetDescriptorSize  ( &pD[nI] );
		byAccess = byGetDescriptorAccess( &pD[nI] );
		byGDOU	 = byGetDescriptorGDOU  ( &pD[nI] );
		byDPL	 = (UCHAR)(byAccess >> 5 ) & 3;
		byP		 = (UCHAR)(byAccess >> 7 );
		kdbg_printf( "[0x%02X] %08X %08X  %d  %d  %s\n", nI*8, dwAddr, dwSize, byP, byDPL, pGetDescriptorTypeStr( szT, byAccess, byGDOU ) );
	}
}