#ifndef BELLONA_PROCESS_HEADER_jj
#define BELLONA_PROCESS_HEADER_jj

struct ThreadTag;
struct WaitObjTag;
struct KbdQTag;
struct MemPoolTag;
struct SignalTag;

#define PROC_MAGIC_STR				"PROCESS"
#define THREAD_MAGIC_STR			"THREAD"
#define DEFAULT_THREAD_NICE			8

#define KPROCESS_STACK_TOP			( 0x80000000 + (0x100000 * 136) )	// thread stack allocated from 2GB+136M
#define KTHREAD_STACK_SIZE			(1024 * 128 )						// Default thread stack size is 64k
#define KTHREAD_NO_STACK			(DWORD)(0xFFFFFFFF)
#define STACK_LINK_MAGIC_STR		"STKLINK"

struct SQTag;
struct R3ExportTblTag;

typedef DWORD (*THREAD_ENTRY_FUNC)(DWORD dwParam);

typedef struct InitTaskKillParamTag {
	DWORD		dwTarget;
};
typedef struct InitTaskKillParamTag InitTaskKillParamStt;

// 쓰레드 스택
typedef struct TStackLinkTag {
	DWORD					dwBaseAddr;
	DWORD					dwSize;
	DWORD					dwR0StackTop, dwR3StackTop;	// ESP에 들어갈 값
	struct TStackLinkTag	*pPre, *pNext;
	struct ThreadTag		*pThread;					// Stack Owner Thread
} TStackLinkStt;

typedef struct AddrSpaceTag {
	DWORD					*pPD;
	struct AddrSpaceTag		*pForkRef;					// Fork Reference pid of this process
	int						nForkRefCounter;			// Reference Counter by fork. (init value = 1 )
	int						nOwnerPID;
} AddrSpaceStt;

// 프로세스의 상태.
#define PSTATE_ZOMBIE		1	

#define THREAD_ALIAS_SIZE	16
#define PROCESS_ALIAS_SIZE	16


// process
typedef struct ProcessTag {
	char					szMagicStr[8];					// "PROCESS"
	char					szAlias[ PROCESS_ALIAS_SIZE ];	// Alias
 	int						nTotalThread;					// Total number of threads in the process
	DWORD					dwID;							// Process ID
	
	AddrSpaceStt			*pAddrSpace;					// address space
	
	DWORD					dwParentID;						// Parent process ID.
	DWORD					dwState;						// process state
	struct ProcessTag		*pPreProcess, *pNextProcess;	// Link to the pre, next process
	struct ThreadTag		*pStartThread, *pEndThread;		// Start and end link to thread
	struct ThreadTag		*pForegroundThread;				// foreground thread
	struct R3ExportTblTag	e;								// export function table
	struct KExecveParamTag	*pKExecveParam;					// kexecve param structure
	int						nExitCode;						// exit code
	struct MemPoolTag		mp;								// user memory pool
	DWORD					dwNextStackBase;				// Next thread stack allocation offset
	TStackLinkStt			*pStartStk, *pEndStk;			// Thread stack link
	void					*pMyDbg;						// debug information
	struct ModuleTag		*pModule;						// owner module
	struct VConsoleTag		*pVConsole;						// Virtual Console
	struct ProcessTag		*pPreFG, *pNextFG;				// FG Link
} ProcessStt;

// THREAD
typedef struct ThreadTag{
	char				szMagicStr[8];					// "THREAD"
	char				szAlias[THREAD_ALIAS_SIZE];		// Alias
	DWORD				dwID;							// Thread ID
	TSSStt				*pTSS;							// Thread TSS
	DWORD				dwMappingFlag;					// mapping flag is compared to kernel's flag
	DWORD				dwR3EntryFunc;					// R3 entry function
	struct	SQTag		*pQ;							// Thread's q pointer
	struct	KbdQTag		*pKbdQ;							// thread's kbd q
	int 				nPrevThreadState;				// thread state before it enters into wait state
	int					nState;							// Current state of the thread
	int					nWaitResult;					// wait result
	DWORD				dwNice, dwCurNice;				// Nice value
	ProcessStt			*pProcess;						// Owner process handle
	struct ThreadTag	*pPreQLink,  *pNextQLink;		// Thread link from Q
	struct ThreadTag	*pPreSLink,  *pNextSLink;		// Thread link from scheduler
	struct ThreadTag	*pPrePLink,  *pNextPLink;		// Thread link from process
	int					nTotalWaitObj;					// the total number of wait objects
	struct WaitObjTag	*pStartWaitObj, *pEndWaitObj;	// Link to wait objects
	//struct SignalTag	signal;							// Thread Signal structure
	DWORD				dwNewSigBits;					// Newly arrived signals.
	TStackLinkStt		*pStack;						// Thread's stack
	struct KMesgQTag	kmesgq;							// kernel message queue
	DWORD				dwKillerTID;					// 현재 Thread를 종료하도로 call한 Thread, KMESG_CHILD_THREAD_EXIT가 전달된다.
} ThreadStt;   

#define	MAX_SCH_Q	10

// Scedule Q structure
typedef struct SQTag {
	int			nTotal;
	int			nState;
	ThreadStt	*pStartThread, *pEndThread;
} SQStt;	 

// schedule structure
typedef struct SchTag {
	
	SQStt		q[MAX_SCH_Q];						// schedule q
	ThreadStt	*pCurrentThread;						// current running thread
	
	// 2003-08-16 가상 콘솔 별로 FG를 설정하기 위하여 스케쥴 구조체에서 제거.
	//ProcessStt	*pForegroundProcess;				// foreground process

	int			nTotalProcess;					// total process in the system
	ProcessStt	*pStartProcess, *pEndProcess;
	int			nTotalThread;						// total system in the system
	ThreadStt	*pStartThread, *pEndThread;
	// 2003-09-22
	ProcessStt	*pStartZombie, *pEndZombie;			// Link for Zombie process
};
typedef struct SchTag SchStt;	


extern SchStt	sch;

extern BELL_EXPORT int			kill_thread				( ThreadStt *pThread );
extern BELL_EXPORT int 			ksuspend_thread			( ThreadStt *pT );
extern BELL_EXPORT int			kresume_thread			( ThreadStt *pT );
extern BELL_EXPORT void			k_set_thread_alias		( ThreadStt *pThread, char *pAlias );
extern BELL_EXPORT void 		k_set_process_alias			( ProcessStt *pP, char *pAlias );
extern BELL_EXPORT ThreadStt	*find_thread_by_alias	( char *pAlias );
extern BELL_EXPORT ThreadStt	*kcreate_thread			( ProcessStt *pProc, DWORD dwStackSize, DWORD dwFunc, DWORD dwParam, int nState );
extern BELL_EXPORT ProcessStt	*k_get_current_process	(); 
extern BELL_EXPORT ThreadStt	*find_thread_by_id		( DWORD dwTID );


extern int			change_thread_state				( SchStt *pSch, ThreadStt *pThread, int nState );
extern int			detach_addr_space				( ProcessStt *pP );
extern int			disp_process					( DWORD dwPID );
extern int			disp_thread						( DWORD dwTID );
extern int			is_process						( ProcessStt *pProcess );
extern int			is_thread						( ThreadStt *pThread );
extern int			kclose_thread					( ThreadStt *pThread );
extern int			kernel_fork						( THREAD_ENTRY_FUNC t_pThreadEntry, DWORD t_dwParam, DWORD t_dwStackSize, DWORD t_dwSchLevel, struct VConsoleTag *pVCon );
extern int			kexecve							( char *t_pFile, char *t_argv[], char *t_envp[] );
extern int			kfork							( DWORD dwESP3 );
extern int			kill_process					( ProcessStt *pProcess );
extern int			krelease_process_struct			( ProcessStt *pProcess );
extern int			krelease_thread_struct			( ThreadStt *pThread );
extern int			kwait							( int *pExitCode );
extern int			kwaitpid						( int nPID, int *pExitCode );
extern int 			kwaittid						( int nTID, int *pExitCode );
extern int			nPopThread						( ThreadStt *pThread );
extern int			nPushThread						( SQStt *pQ, ThreadStt *pThread );
extern int			set_foreground_thread			( ThreadStt *pThread );
extern int			set_global_foreground_thread	( ThreadStt *pThread );

extern char 		*skip_space						( char *pS );
extern char 		*search_space					( char *pS );

extern void			disp_thread_list				();
extern void			disp_process_list				();
extern void			display_schedule_q				();

extern DWORD		get_thread_page_dir				( struct ThreadTag *pT );
extern DWORD		get_process_page_dir			( struct ProcessTag *pP );
extern DWORD		r0_fork_thread					( DWORD dwParam );


extern ThreadStt	*get_current_thread				();
extern ThreadStt	*find_thread_by_tss				( TSSStt *pTSS );
extern ThreadStt	*get_fg_thread					( ProcessStt *pProcess );

extern ProcessStt	*kcreate_process				( ProcessStt *pParent );
extern ProcessStt	*get_sys_fg_process				();
extern ProcessStt	*get_fg_process					();
extern ProcessStt	*find_process_by_id				( DWORD dwPID );

extern struct KExecveParamTag *make_kexecve_param		( char *pFile, char **ppArgv, char **ppEnv );

#endif
