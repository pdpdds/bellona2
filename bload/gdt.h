#ifndef BELLONA_GDT_HEADER
#define BELLONA_GDT_HEADER

#pragma pack(1)

#include <mem.h>

#define GSEL_CODE16       8
#define GSEL_DATA16      16
#define GSEL_STACK16     24
#define GSEL_CODE32      32
#define GSEL_DATA32      40
#define GSEL_CALLGATE32  48

typedef unsigned short UINT16;
typedef unsigned long  ULONG;
typedef unsigned char  UCHAR;
typedef unsigned long  DWORD;

typedef struct {
    UCHAR  size_0, size_1;
    UCHAR  addr_0, addr_1, addr_2;
    UCHAR  byAccess;
    UCHAR  gdou_size;
    UCHAR  addr_3;
} DescriptorStt;

typedef struct {
    UCHAR offset_0, offset_1;
    UCHAR selector_h, selector_l;  // Selector High, Low
    UCHAR parameters;              // parameters
    UCHAR type;
    UCHAR offset_2, offset_3;
} CallGate32Stt;

typedef struct {
    UINT16 wSize;
    ULONG  dwAddr;
} GDTRStt;

extern void   vClearGDTR   ( GDTRStt *pG );
extern UINT16 wSetGDTRSize ( GDTRStt *pG, UINT16 wSize );
extern ULONG  dwSetGDTRAddr( GDTRStt *pG, ULONG  dwAddr );
extern UINT16 wGetGDTRSize ( GDTRStt *pG );
extern ULONG  dwGetGDTRAddr( GDTRStt *pG );

extern void vClearDescriptor    ( DescriptorStt *pD );
extern void vCopyDescriptor     ( DescriptorStt *pDest, DescriptorStt *pSrc );

extern ULONG dwSetDescriptorSize  ( DescriptorStt *pD, ULONG dwSize );
extern UCHAR bySetDescriptorAddr  ( DescriptorStt *pD, ULONG dwAddr );
extern UCHAR bySetDescriptorAccess( DescriptorStt *pD, UCHAR byAccess );
extern UCHAR bySetDescriptorGDOU  ( DescriptorStt *pD, UCHAR byGDOU );

extern ULONG dwGetDescriptorSize  ( DescriptorStt *pD );
extern ULONG dwGetDescriptorAddr  ( DescriptorStt *pD );
extern UCHAR byGetDescriptorAccess( DescriptorStt *pD );
extern UCHAR byGetDescriptorGDOU  ( DescriptorStt *pD );

extern ULONG dwSetCallGeteOffset( CallGate32Stt *pC, DWORD dwOffset );
extern ULONG dwGetCallGate32Offset( CallGate32Stt *pC );



#endif