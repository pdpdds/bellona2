#ifndef BELLONA_GDT_HEADER
#define BELLONA_GDT_HEADER

#include <types.h>

#if defined(FLOPPY_BOOT)
////////////////////////////////////
#define GSEL_CODE16				0x08
#define GSEL_DATA16				0x10
#define GSEL_STACK16			0x18
#define GSEL_CODE32				0x20
#define GSEL_DATA32				0x28
#define GSEL_CODE32_R3			0x30
#define GSEL_DATA32_R3			0x38
#define GSEL_CALLGATE32			0x40
#define GSEL_DBGTSS32			0x48
#define GSEL_INITTSS32			0x50
#define GSEL_DBG_TASK_GATE		0x58
#define GSEL_LDT				0x60
#define GSEL_DUMMY_TSS32		0x68
#define GSEL_DUMMY_V86TSS		0x70
#define GSEL_PFTSS32            0x78
#define GSEL_TLBTSS32			0x80
/////////////////////////////////////
#define TOTAL_GSEL				(17)		// +1 means 0 descriptor
/////////////////////////////////////
#else 
#define GSEL_CODE32				0x08
#define GSEL_DATA32				0x10
#define GSEL_CODE32_R3			0x18
#define GSEL_DATA32_R3			0x20
#define GSEL_CALLGATE32			0x28
#define GSEL_DBGTSS32			0x30
#define GSEL_INITTSS32			0x38
#define GSEL_DBG_TASK_GATE		0x40
#define GSEL_LDT				0x48
#define GSEL_DUMMY_TSS32		0x50
#define GSEL_DUMMY_V86TSS		0x58
#define GSEL_PFTSS32            0x60
#define GSEL_TLBTSS32			0x68
/////////////////////////////////////
#define TOTAL_GSEL				(14)		// +1 means 0 descriptor
/////////////////////////////////////
#endif


#define ISEL_INT_1				(0x08 * 1)
#define ISEL_INT_3				(0x08 * 3)
#define ISEL_PF					(0x08 * 0x0E)
#define ISEL_TLB				(0x08 * 0x53)
/////////////////////////////////////

#pragma pack(push, 1)

typedef struct descriptorTag{
    UCHAR  size_0, size_1;
    UCHAR  addr_0, addr_1, addr_2;
    UCHAR  byAccess;
    UCHAR  gdou_size;
    UCHAR  addr_3;
};
typedef struct descriptorTag DescriptorStt;

typedef struct CallGate32Tag{
    UCHAR  offset_0, offset_1;
    UINT16 wSelector;		
    UCHAR  parameters;      
    UCHAR  type;
    UCHAR  offset_2, offset_3;
};
typedef struct CallGate32Tag CallGate32Stt;

typedef struct GDTRTag{
    UINT16 wSize;
    ULONG  dwAddr;
};
typedef struct GDTRTag GDTRStt;

#pragma pack(pop)

extern void   vClearGDTR   ( GDTRStt *pG );
extern UINT16 wSetGDTRSize ( GDTRStt *pG, UINT16 wSize );
extern ULONG  dwSetGDTRAddr( GDTRStt *pG, ULONG  dwAddr );
extern UINT16 wGetGDTRSize ( GDTRStt *pG );
extern ULONG  dwGetGDTRAddr( GDTRStt *pG );

extern void vClearDescriptor    ( DescriptorStt *pD );
extern void vCopyDescriptor     ( DescriptorStt *pDest, DescriptorStt *pSrc );

extern ULONG dwSetDescriptorSize  ( DescriptorStt *pD, ULONG dwSize );
extern ULONG dwSetDescriptorAddr  ( DescriptorStt *pD, ULONG dwAddr );
extern UCHAR bySetDescriptorAccess( DescriptorStt *pD, UCHAR byAccess );
extern UCHAR bySetDescriptorGDOU  ( DescriptorStt *pD, UCHAR byGDOU );

extern ULONG dwGetDescriptorSize  ( DescriptorStt *pD );
extern ULONG dwGetDescriptorAddr  ( DescriptorStt *pD );
extern UCHAR byGetDescriptorAccess( DescriptorStt *pD );
extern UCHAR byGetDescriptorGDOU  ( DescriptorStt *pD );

extern ULONG dwSetCallGeteOffset( CallGate32Stt *pC, DWORD dwOffset );
extern ULONG dwGetCallGate32Offset( CallGate32Stt *pC );

extern void vMakeGDTDescriptor( int nIndex, DWORD dwAddr, DWORD dwSize, UCHAR byAccess, UCHAR byGDOU );
extern void vMakeTaskGate_in_GDT( int nGateOffset, int nSelectorOffset );
extern void vMakeTaskGate_in_IDT( int nGateOffset, int nSelectorOffset );
extern void vDispGDT( DescriptorStt *pD, int nTotal );
extern void vDispGDT( DescriptorStt *pD, int nTotal );
extern void vSetDescriptorBusyBit( DescriptorStt *pD, UCHAR byBusy );

#endif
