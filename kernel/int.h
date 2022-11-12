#ifndef BELLONA_IDT_HEADER_oh
#define BELLONA_IDT_HEADER_oh

//--------------------------------------------------------------//
// IDT Descriptor들의 구조
typedef struct IDTTag{
    UINT16 wOffs0;  // 오프셋 0-15
    UINT16 wSel;    // 셀렉터 값
    UINT16 wType;   // 타입
    UINT16 wOffs1;  // 오프셋 16-31
};
typedef struct IDTTag IDTStt;

#pragma pack( push, 1 )
typedef struct IDTRTag{
    UINT16 wSize;
    DWORD  dwAddr;
};
typedef struct IDTRTag IDTRStt;
#pragma pack( pop )


extern BELL_EXPORT void get_clk 		( __int64 *pClk );

//--------------------------------------------------------------//
extern UCHAR pm_pic_data[];
extern UCHAR real_pic_data[];

extern int display_idt					( IDTStt *pIDT );

extern void pagefault_task_main			();
extern void vDefaultExceptionHandler	( DWORD dwAddr );
extern void vSetDefaultIDT				( IDTStt *pIDT, UINT16 wSize );
extern void set_int_handler				( int nInt, void *pHandler );

extern DWORD get_rdtsc_per_millis			();


#endif
