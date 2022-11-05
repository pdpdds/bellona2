#include "gdt.h"


//-------------------------------//
// Function to access GDTR Value //
//-------------------------------//
void* B_memcpy( void* pD, void *pS, long lSize )
{
	long lX;
	UCHAR *pDest = (UCHAR*)pD;
	UCHAR *pSrc  = (UCHAR*)pS;

	for( lX = 0; lX < lSize; lX++ )
		pDest[lX] = pSrc[lX];

	return( pD );
}
void *B_memset( void *pD, UCHAR byTe, long lSize )
{
    UCHAR *pDest = (UCHAR*)pD;
    long  lX;
    for( lX = 0; lX < lSize; lX++ )
        pDest[lX] = byTe;

     return( pD );
}

int B_memcmp( void* pD, void *pS, long lSize )
{
	long lX;
	int  nR;
	UCHAR *pDest = (UCHAR*)pD;
	UCHAR *pSrc  = (UCHAR*)pS;


	nR = 0;
	for( lX = 0; lX < lSize; lX++ )
	{
		if( pDest[lX] > pSrc[lX] )
		{
			nR = 1;
			break;
		}
		else if( pDest[lX] < pSrc[lX] )
		{
			nR = -1;
			break;
		}
	}

	return( nR );
}

void   vClearGDTR   ( GDTRStt *pG )
{
    B_memset( pG, 0, sizeof( GDTRStt ) );
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
    B_memset( pD, 0, sizeof( DescriptorStt ) );
}
void vCopyDescriptor     ( DescriptorStt *pDest, DescriptorStt *pSrc )
{
    B_memcpy( pDest, pSrc, sizeof( DescriptorStt ) );
}

ULONG dwSetDescriptorSize  ( DescriptorStt *pD, ULONG dwSize )
{
    UCHAR  s[4];
    ULONG  dwOrg;

    dwOrg = dwGetDescriptorSize( pD );

    B_memcpy( s, &dwSize, 4 );

    s[1] &= 0x0F;
    pD->size_0 = s[2];
    pD->size_1 = s[3];
    pD->gdou_size |= s[1];

    return( dwOrg );
}

ULONG dwSetDescriptorAddr  ( DescriptorStt *pD, ULONG dwAddr )
{
    UCHAR  s[4];
    ULONG  dwOrg;

    dwOrg = dwGetDescriptorAddr( pD );

    B_memcpy( s, &dwAddr, 4 );

    pD->addr_0 = s[0];
    pD->addr_1 = s[1];
    pD->addr_2 = s[2];
    pD->addr_3 = s[3];

    return( dwOrg );
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

    s[0] = 0;
    s[1] = pD->gdou_size;
    s[1] &= 0x0F;
    s[2] = pD->size_0;
    s[3] = pD->size_1;

    B_memcpy( &dwR, s, 4 );

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

    B_memcpy( &dwR, s, 4 );

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

    B_memcpy( offs, &dwOffset, 4 );

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