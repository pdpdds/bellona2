#ifndef BELLONA2_SYSTEM_CALL_HEADER_jj
#define BELLONA2_SYSTEM_CALL_HEADER_jj

#include <sctype.h>

typedef enum {
	R3EXI_PREEMPT_R3_THREAD = 0,	// 사용자 Thread 선점용 함수.
	R3EXI_USER_THREAD_ENTRY,		// 사용자 Thread 진입 함수
	R3EXI_UAREA,					// uarea 주소
	MAX_R3_EXPORT
} R3_EXP_INDEX_TAG;

typedef struct R3ExportTblTag {
	DWORD	func[ MAX_R3_EXPORT ];
};
typedef struct  R3ExportTblTag R3ExportTblStt;

typedef struct SYSCallTag {
	int		nType;							// function type
	DWORD	dwParam;						// parameter
	int		nReturn;						// return value
    DWORD   dwSigBit;                       // 더이상 사용하지 않음.
};
typedef struct SYSCallTag SYSCallStt;

typedef int (*SC_FUNC)( SYSCallStt *pSC );

typedef struct SC_FUNC_TAG
{
	int		nType;
	SC_FUNC	pFunc;
};
typedef struct SC_FUNC_TAG SCFuncStt;
				  
typedef struct {
    DWORD dwType;
    DWORD dwFunc;
} SCallTblStt;

extern BELL_EXPORT void set_grxcall_tbl( void *pFuncTbl, void *pCacheAddrTbl );

extern void system_call_wrapper();  // int 0x50 handler
extern void ksyscall_handler();     // int 0x52 handler
extern void kgrxcall_handler();		// int 0x54 handler

extern void init_sc_struct( SYSCallStt *pSC );

#endif
