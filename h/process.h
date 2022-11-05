#ifndef BELLONA2_LIB_PROCESS_HEADER
#define BELLONA2_LIB_PROCESS_HEADER

// Thread Priority & State 
typedef enum {
	TS_READY_TIME_CRITICAL	= 0,
	TS_READY_HIGHEST		,
	TS_READY_ABOVE_NORMAL	,		// Disk I/O
	TS_READY_NORMAL 		,		// Normal Threads
	TS_READY_BELOW_NORMAL	,
	TS_READY_LOWEST 		,
	TS_READY_IDLE			,
	TS_READY_LAZY			,
	//=====================//
	TS_WAIT 				,		// blocked thread
	TS_SUSPEND				,		// suspended thread
	TS_TERMINATED			,		// the terminated thread

	TS_ERROR				,		// thread which makes an error (page fault, exception... )
											  
	END_OF_THREAD_PRIORITY
} ThreadPriorityTag;


extern BELL_EXPORT int		execve					( char *pFile, char *argv[], char *envp[] );
extern BELL_EXPORT int 		fork					();
extern BELL_EXPORT int 		wait					( int *pExitCode );
extern BELL_EXPORT int 		waitpid					( int nPID, int *pExitCode );
extern BELL_EXPORT int 		waittid					( int nTID, int *pExitCode );
extern BELL_EXPORT int      set_fg_tid				( DWORD dwTID );
extern BELL_EXPORT int		set_thread_alias		( DWORD dwTID, char *pAlias );
extern BELL_EXPORT int 		set_process_alias		( DWORD dwPID, char *pAlias );
extern BELL_EXPORT int		remove_thread			( DWORD dwTID );

extern BELL_EXPORT DWORD	create_thread			( DWORD dwFunc, DWORD dwParam, int nState, DWORD dwStackSize );
extern BELL_EXPORT DWORD	get_cur_thread_id		();
extern BELL_EXPORT DWORD	get_cur_process_id		();

#endif




