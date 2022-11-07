#ifndef BELLONA2_SCHEDULE_HANDLER_jj
#define BELLONA2_SCHEDULE_HANDLER_jj

struct WaitObjTag;
struct TimeOutChunkTag;

typedef int (*TIMEOUT_CALLBACK)( DWORD dwParam );

// timeout structure
typedef struct TimeOutTag {
	DWORD					dwTick;							// tick to expiration
	DWORD					dwCurTick;						// current tick
	int						nReady;							// ready to increase nCurTick
	struct WaitObjTag		*pWaitObj;						// pointer to wait object
	struct TimeOutTag		*pPreTimeOut, *pNextTimeOut;	// timeout link
	struct TimeOutChunkTag	*pTimeOutChunk;					// pointer to the timeout chunk
	TIMEOUT_CALLBACK		pCallBack;						// timeout callback function
	DWORD					dwCallBackParam;				// call back parameter
	BYTE					byPeriodic;						// 한 번 타임아웃이 걸렸다고 제거하지 말고 주기적으로 처리할 것.
};
typedef struct TimeOutTag TimeOutStt;

typedef int (*GUI_MESG_POST_FUNC)( DWORD dwWinID, DWORD dwMesgID, DWORD dwParamB );

typedef struct GuiTimerTag {
	DWORD 					dwWinID;						// 메시지가 전달된 Window ID
	DWORD					dwTimerID;						// Message Handler에 ParamA로 전달된다.
	DWORD					dwParamB;						// ParamB로 전달될 값.
	DWORD					dwTick;
	__int64 				due_clk;
	struct GuiTimerTag 		*pPre, *pNext;
} GuiTimerStt;

typedef struct SysGuiTimerTag {
	GUI_MESG_POST_FUNC		pMesgPost;
	GuiTimerStt				*pStart, *pEnd;
} SysGuiTimerStt;

// timeout chunk structure
#define MAX_TIMEOUT_PER_CHUNK	16
typedef struct TimeOutChunkTag {
	int				nTotalUsed;							// total used timeout slot
	TimeOutStt		slot[MAX_TIMEOUT_PER_CHUNK];		// 16 timeout array
	struct TimeOutChunkTag	*pPreChunk, *pNextChunk;
};
typedef struct TimeOutChunkTag TimeOutChunkStt;				  

// system timer structure
typedef struct SystemTimerTag {
	DWORD			dwCurTick;							// current ystem tick count
	DWORD			dwTickCarry;						// tick count carry
	int				nTotalChunk;						// total allocated chunk
	TimeOutChunkStt	*pStartChunk, *pEndChunk;			// link to chunks
	int				nTotalTimeOut;						// total timeout structure
	TimeOutStt		*pStartTimeOut, *pEndTimeOut;		// timeout list
};
typedef struct SystemTimerTag SystemTimerStt;


// Schedule 구조체와 Queue구조체는 process.h에 정의되어 있다.

extern BELL_EXPORT int kernel_thread_switching		( ThreadStt *pThread );
extern BELL_EXPORT int init_gui_timer				( GUI_MESG_POST_FUNC pFunc );
extern BELL_EXPORT int unregister_gui_timer 		( GuiTimerStt *pGT );
extern BELL_EXPORT int close_all_gui_timer 			();

extern BELL_EXPORT GuiTimerStt *find_gui_timer		( DWORD dwWinID, DWORD dwTimerID );	
extern BELL_EXPORT GuiTimerStt *register_gui_timer	( DWORD dwWinID, DWORD dwTimerID, DWORD dwParamB, DWORD dwTick );

extern DWORD		dwGetCurTick				();

extern TimeOutStt	*alloc_timeout				();

extern int			kernel_scheduler			();
extern int			set_pick_thread_id			( DWORD dwTID );
extern int			ktimer_scheduler			( DWORD dwESP );
extern int			free_timeout				( TimeOutStt *pTimeOut );
extern int			add_to_timeout_list			( TimeOutStt *pTimeOut );
extern int			sub_from_timeout_list		( TimeOutStt *pTimeOut );

extern void 		kernel_task_switching		( ThreadStt *pThread );
extern void 		kinit_scheduler 			();


#endif
