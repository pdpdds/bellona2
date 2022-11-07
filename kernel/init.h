#ifndef BELLONA2_INIT_THREAD_HEADER_jj
#define BELLONA2_INIT_THREAD_HEADER_jj

#define INIT_TASK_FORK_FUNC					1
#define INIT_TASK_KILLTHREAD_FUNC			2
#define INIT_TASK_KILLPROCESS_FUNC			3

typedef enum {
	INIT_PHASE_NOTHING = 0,
	INIT_PHASE_TR_SET,					// TR이 설정됨.  Task Switching이 가능함.
	INIT_PHASE_ENTER_INIT_THREAD,		// init thread로 진입함.
	END_OF_INITPHASE
} INIT_PHASE_TAG;

extern int		init_thread				( void *pPram );
extern int		sniper_thread			( void *pPram );
extern int		init_thread2			( void *pPram );
extern int		set_global_kill_param	( void *pV );

extern void		init_task_main			();
extern void		jmp_init_task			( int nFunc );
extern void		call_init_task			( int nFunc );
extern void		set_init_phase			();

extern DWORD	get_init_phase			();

extern ThreadStt *get_init_thread		();

#endif