// This file is used by both kernel and c library.

#include "bellona2.h"
#include "shmem.h"

static void system_call_handler( DWORD dwParam );

_declspec(naked) void system_call_wrapper()
{
	static DWORD dwParam;

	_asm {
		PUSH EBP
		MOV  EBP, ESP
		PUSHFD
		PUSHAD
		
		STI					// must enable interrupt
		MOV  BX,GSEL_DATA32
		MOV  DS,BX
		MOV  ES,BX
		MOV  FS,BX
		MOV  GS,BX

		MOV  dwParam, EAX;

		// check if r3 trap flag is on
		//MOV  EBX, [EBP+12]
		//AND  EBX, MASK_TF
		//TEST EBX, EBX
		//JZ   CALL_SYS
		//	 INT 1h		// trigger debugger
	}

//CALL_SYS:	
		system_call_handler( dwParam );	

	_asm {
		POPAD
		POPFD
		POP EBP
		IRETD
	}
}

// this function is used by printf
static int sc_tty_out( SYSCallStt *pSC )
{
	int nR; 

	nR = kdbg_printf( (char*)pSC->dwParam );
	
	return( nR );
}


// register r3 functions to kernel 
static int sc_reg_export( SYSCallStt *pSC )
{
	ProcessStt *pP;

	pP = k_get_current_process();
	if( pP == NULL )
	{
		kdbg_printf( "sc_reg_export: current process is null!\n" );
		return( -1 );
	}

	memcpy( &pP->e, (char*)pSC->dwParam, sizeof( R3ExportTblStt ) );

	return( 0 );
}	

// copy path, argument string, env string to r3 area
/*
static int sc_copy_arg( SYSCallStt *pSC )
{
	ThreadStt		*pThread;

	pThread = get_current_thread();
	if( pThread == NULL )
	{
		kdbg_printf( "sc_copy_arg() - current thread is null!\n" );
		return( -1 );
	}

	if( pThread->pProcess == NULL )
	{
		kdbg_printf( "sc_copy_arg() - owner process of the current thread is null!\n" );
		return( -1 );
	}

	if( pThread->pProcess->pExecParam == NULL )
	{
		kdbg_printf( "sc_copy_arg() - the process has not exec_param!\n" );
		return( -1 );													   
	}

	memcpy( (char*)pSC->dwParam, (char*)pThread->pProcess->pExecParam, sizeof( ExecParamStt ) );

	return( 0 );
}
*/
static int sc_call_scheduler( SYSCallStt *pSC )
{	
	// 스케쥴러를 호출한다.
	kernel_scheduler();
	return( 0 );
}

static int sc_getch( SYSCallStt *pSC )
{
	int nKey;

	nKey = getchar();

	return( nKey );
}

static int sc_malloc( SYSCallStt *pSC )
{
	void		*pAddr;
	DWORD		dwSize;			  
	ProcessStt  *pProcess;

	pProcess = k_get_current_process();
	if( pProcess == NULL )
		return( 0 );

	dwSize = pSC->dwParam;
	pAddr = umalloc( (DWORD*)get_process_page_dir( pProcess ), &pProcess->mp, dwSize );
			
	return( (int)pAddr );
}

static int sc_free( SYSCallStt *pSC )
{
	int			nR;
	DWORD		dwAddr;
	ProcessStt  *pProcess;

	pProcess = k_get_current_process();
	if( pProcess == NULL )
		return( 0 );

	dwAddr = pSC->dwParam;
	nR = ufree( &pProcess->mp, (void*)dwAddr );
			
	return( nR );
}

static int sc_create_shmem( SYSCallStt *pSC )
{
	int				nID;
	ShMemParamStt	sh_param;

	memcpy( &sh_param, (char*)pSC->dwParam, sizeof( ShMemParamStt ) );
	
	nID = create_shmem( sh_param.szName, (int)sh_param.dwSize, 0 );
			   
	return( nID );
}

static int sc_close_shmem( SYSCallStt *pSC )
{
	int nID, nR;

	nID = (DWORD)pSC->dwParam;

	nR = close_shmem( nID );

	return( nR );
}

static int sc_attach_shmem( SYSCallStt *pSC )
{
	int				nR;
	ShMemParamStt	sh_param;

	memcpy( &sh_param, (char*)pSC->dwParam, sizeof( ShMemParamStt ) );

	nR = attach_shmem( sh_param.nID, sh_param.dwAddr );
		 
	return( nR );
}

static int sc_detach_shmem( SYSCallStt *pSC )
{
	int				nR;
	ShMemParamStt	sh_param;

	memcpy( &sh_param, (char*)pSC->dwParam, sizeof( ShMemParamStt ) );

	nR = detach_shmem( sh_param.nID, sh_param.dwAddr );
		 
	return( nR );
}					 

static int sc_find_shmem( SYSCallStt *pSC )
{
	int				nR;
	ShMemParamStt	sh_param;

	memcpy( &sh_param, (char*)pSC->dwParam, sizeof( ShMemParamStt ) );

	nR = find_shmem_ent( sh_param.szName, -1 );		// size parameter is don't care
		 
	return( nR );
}					 

static int sc_lock_shmem( SYSCallStt *pSC )
{
	int nID, nR;

	nID = (int)pSC->dwParam;

	nR = lock_shmem( nID );

	return( nR );
}

static int sc_unlock_shmem( SYSCallStt *pSC )
{
	int nID, nR;

	nID = (int)pSC->dwParam;

	nR = unlock_shmem( nID );

	return( nR );
}

static int sc_open( SYSCallStt *pSC )
{
	int		nR;
	DWORD	*pParam;

	pParam = (DWORD*)pSC->dwParam;

	nR = kopen( (char*)pParam[0], (int)pParam[1] );

	return( nR );
}

static int sc_close( SYSCallStt *pSC )
{
	int		nR;
	DWORD	*pParam;

	pParam = (DWORD*)pSC->dwParam;

	nR = kclose( (int)pParam[0] );

	return( nR );
}

static int sc_read( SYSCallStt *pSC )
{
	int		nR;
	DWORD	*pParam;

	pParam = (DWORD*)pSC->dwParam;

	// check if write is available (pParam[1] is the buffer)
	if( !(pParam[1] & (DWORD)0x80000000) )
		return( -1 );

	nR = kread( (int)pParam[0], (char*)pParam[1], (int)pParam[2] );

	return( nR );
}

static int sc_write( SYSCallStt *pSC )
{
	int		nR;
	DWORD	*pParam;

	pParam = (DWORD*)pSC->dwParam;

	nR = kwrite( (int)pParam[0], (char*)pParam[1], (int)pParam[2] );

	return( nR );
}

static int sc_lseek( SYSCallStt *pSC )
{
	int		nR;
	DWORD	*pParam;

	pParam = (DWORD*)pSC->dwParam;

	nR = (int)klseek( (int)pParam[0], (int)pParam[1], (int)pParam[2] );

	return( nR );
}

static int sc_rename( SYSCallStt *pSC )
{
	int		nR;
	DWORD	*pParam;

	pParam = (DWORD*)pSC->dwParam;
						  // 0 - old, 1 - new
	nR = (int)krename( (char*)pParam[0], (char*)pParam[1] );

	return( nR );
}

static int sc_delay( SYSCallStt *pSC )
{
	return( kdelay( (int)pSC->dwParam ) );
}

static int sc_pause( SYSCallStt *pSC )
{
	return( kpause() );
}

static int sc_get_cur_tid( SYSCallStt *pSC )
{
	ThreadStt	*pThread;

	pThread = get_current_thread();
	if( pThread == NULL )
		return( 0 );

	return( (int)pThread->dwID );
}

static int sc_get_sys_module_addr( SYSCallStt *pSC )
{
	return( (int)get_sys_module() );
}

static int sc_get_module_handle( SYSCallStt *pSC )
{
	ModuleStt *pM;

	pM = find_module_by_alias( (char*)pSC->dwParam , NULL );	// alias name

	return( (int)pM );
}

//*************************************************************************************************//
#pragma data_seg( "data2" )
static SCFuncStt scall_tbl[] = {
	{ SCTYPE_TTYOUT,				sc_tty_out				},
	{ SCTYPE_REG_EXPORT,			sc_reg_export			},
	{ SCTYPE_CALL_SCHEDULER,		sc_call_scheduler		},
	//{ SCTYPE_COPY_ARG,				sc_copy_arg				},
	{ SCTYPE_GETCH,					sc_getch				},
	{ SCTYPE_MALLOC,				sc_malloc				},
	{ SCTYPE_FREE,					sc_free					},
	{ SCTYPE_GET_CUR_TID,			sc_get_cur_tid			},
	{ SCTYPE_CREATE_SHMEM,			sc_create_shmem			},
	{ SCTYPE_CLOSE_SHMEM,			sc_close_shmem			},
	{ SCTYPE_ATTACH_SHMEM,			sc_attach_shmem			},
	{ SCTYPE_DETACH_SHMEM,			sc_detach_shmem			},
	{ SCTYPE_FIND_SHMEM,			sc_find_shmem			},
	{ SCTYPE_LOCK_SHMEM,			sc_lock_shmem			},
	{ SCTYPE_UNLOCK_SHMEM,			sc_unlock_shmem			},
	{ SCTYPE_OPEN,					sc_open					},
	{ SCTYPE_CLOSE,					sc_close				},
	{ SCTYPE_READ,					sc_read					},
	{ SCTYPE_WRITE,					sc_write				},
	{ SCTYPE_LSEEK,					sc_lseek				},
	{ SCTYPE_RENAME,				sc_rename				},
	{ SCTYPE_DELAY,					sc_delay				},
	{ SCTYPE_PAUSE,					sc_pause				},
	{ SCTYPE_GET_SYS_MODULE_HANDLE, sc_get_sys_module_addr  },
	{ SCTYPE_GET_MODULE_HANDLE,     sc_get_module_handle	},

	{ 0, NULL },
};
static SC_FUNC syscall_addr_tbl[ TOTAL_SYSTEM_CALL ];
#pragma data_seg()
//*************************************************************************************************//

// 2002-12-07 
// 매번 for loop에서 함수 주소를 찾아 call하던 것을 한 번 찾은 주소는
// 보관해 두고 다음부터는 이전에 찾은 주소를 call하도록 변경.
static void system_call_handler( DWORD dwParam )
{
	int			nI;
	SYSCallStt *pSC;

	pSC = (SYSCallStt*)dwParam;
    if( pSC == NULL || pSC->nType < 0 || pSC->nType >= TOTAL_SYSTEM_CALL )
        return;
    
	// 한 번 찾은 것은 다시 찾지 않는다.
    if( syscall_addr_tbl[ pSC->nType ] == NULL )
    {
        // 주소를 찾는다.    
    	for( nI = 0; ; nI++ )
	    {
            if( scall_tbl[nI].pFunc == NULL )
            {   // 함수를 찾을 수 없다.
                pSC->nReturn = SYSCALL_FUNC_NOT_FOUND;
                return;
            }          

		    if( scall_tbl[nI].nType == pSC->nType )
    		{	// 함수를 찾았다.  찾은 주소를 테이블에 저장해 둔다.
                syscall_addr_tbl[ pSC->nType ] = scall_tbl[nI].pFunc;
		    	break;
            }
		}	
	}	

    // 해당 시스템 콜 핸들러를 실행한다.
	pSC->nReturn = syscall_addr_tbl[ pSC->nType ]( pSC );

    return;
}
//===========================================================//
//  New System Call                                          //
//===========================================================//
static int ksc_tty_out( char *pStr )
{
	int nR; 

	nR = kdbg_printf( pStr );
	
	return( nR );
}

// 커서의 현재 위치를 구한다.
static void ksc_get_cursor_xy( int *pX, int *pY )
{
	get_cursor_xy( pX, pY );
}

// 커서의 현재 위치를 설정한다.
static void ksc_set_cursor_xy( int nX, int nY )
{
	set_cursor_xy( nX, nY );

	if( is_gui_mode() != 0 )
	{
		gui_set_cursor_xy( nX, nY );
		// 화면 전체가 갱신되는 효과가 있다.
		gui_write( "" );
	}
}

// FG 프로세스를 설정한다.
static int ksc_set_fg_process( DWORD dwPID )
{
	int			nR;
	ProcessStt  *pP;

	if( dwPID == 0 )
		pP = k_get_current_process();
	else
		pP = find_process_by_id( dwPID );

	if( pP == NULL )
	{
		kdbg_printf( "ksc_set_fg_process() - PID is NULL!\n" );
		return( -1 );
	}

	//kdbg_printf( "new FG Process = 0x%08X\n", pP->dwID );
	// FG로 설정한다.
	nR = set_fg_process( NULL, pP );

	return( nR );
}

static int ksc_kbhit()
{
    int nR;
    // 쓰레드에 키 입력이 있는지 확이한다.
    nR =  thread_kbhit();
    return( nR );
}

static int ksc_opendir( char *pPath )
{
	int nHandle;
	nHandle = kopendir( pPath );
	return( nHandle );
}

static int ksc_closedir( int nHandle )
{
	int nR;
	nR = kclosedir( nHandle );
	return( nR );
}

static int ksc_readdir( DIRENTStt *pDIRENT, int nHandle )
{
	int nR;
	nR = kreaddir( pDIRENT, nHandle );
	return( nR );
}

static int ksc_direct_dispstr( int nX, int nY, char *pS )
{
    nWriteToVideoMem( nX, nY, pS );
    return( 0 );
}               

// line nY의 nX부터 라인 끝까지 지운다.
static int ksc_del_line( int nX, int nY )
{
	char	szT[84];

	memset( szT, ' ', sizeof( szT ) );
	szT[ 80 - nX ] = 0;
    nWriteToVideoMem( nX, nY, szT );

	if( is_gui_mode() != 0 )
		gui_direct_write( "", nX );	// 라인 끝까지 지우게 된다.
    
	return( 0 );
}               

// 새로운 프로세스를 생성한다.
static int ksc_fork( DWORD dwESP3 )
{
	int nPID;

	nPID = kfork( dwESP3 );
	return( nPID );
}

static int ksc_wait( int *pExitCode )
{	int nPID;

	nPID = kwait( pExitCode );

	return( nPID );
}

static int ksc_waitpid( int nPID, int *pExitCode )
{	int nResultPID;

	nResultPID = kwaitpid( nPID, pExitCode );

	return( nPID );
}

static int ksc_waittid( int nTID, int *pExitCode )
{	int nResultTID;

	nResultTID = kwaittid( nTID, pExitCode );

	return( nTID );
}

// exit
static int ksc_exit( int nExitCode )
{
	ProcessStt *pProcess;

	// get current process
	pProcess = k_get_current_process();
	if( pProcess == NULL )
	{
		kdbg_printf( "ksc_exit: current process is null!\n" );
		
		for( ;; )	// endless loop
			kernel_scheduler();
	}
	
	// 모듈의 프로세스 카운터를 감소시킨다.
	if( pProcess->pModule != NULL )
		dec_process_count( pProcess->pModule );

	// 종료 코드를 직접 세팅한다.
	pProcess->nExitCode = nExitCode;

	// 프로세스를 죽인다. (리턴되지 않는다.)
	kill_process( pProcess );

	// 이 부분이 절대 실행되지 않는다.
	kdbg_printf( "ksc_exit: critical error\n" );

	return( 0 );
}	

static int ksc_execve( char *pFile, char *argv[], char *envp[] )
{
	int nR;
	nR = kexecve( pFile, argv, envp );
	return( nR );
}

static int ksc_sem_create( char *pName, int nMaxCounter, DWORD dwAttrib )
{
	int nR;
	nR = (int)ksem_create( pName, nMaxCounter, dwAttrib );
	return( nR );
}

static int ksc_sem_open( char *pName )
{
	int nR;
	nR = (int)ksem_open( pName );
	return( nR );
}

static int ksc_sem_close( SemaphoreStt	*pSema )
{
	int				nR;
	nR = ksem_close( pSema );
	return( nR );
}

static int ksc_sem_post( SemaphoreStt *pSem, DWORD dwOption )
{
	int	nR;
	nR = ksem_post( pSem, dwOption );
	return( nR );
}

static int ksc_sem_wait( SemaphoreStt *pSem )
{
	int	nR;
	nR = ksem_wait( pSem );
	return( nR );
}

static int ksc_is_gui_mode()
{
	return( is_gui_mode() );
}

static int ksc_copy_kexecve_param()
{
	ProcessStt		*pP;
	void			*pEP;

	pP = k_get_current_process();
	if( pP == NULL )
		return( 0 );

	if( pP->pKExecveParam == NULL )
		return( 0 );

	pEP = (char*)umalloc( pP->pAddrSpace->pPD, &pP->mp, pP->pKExecveParam->nSize );
	if( pEP == NULL )
		return( 0 );

	memcpy( pEP, pP->pKExecveParam, pP->pKExecveParam->nSize );
	
	return( (int)pEP );
}

static int ksc_get_time( TTimeStt *pT )
{
	read_cmos_time( pT );
	return( 0 );
}

static int ksc_create_thread( DWORD dwStackSize, DWORD dwR3Func, DWORD dwParam, int nState )
{
	ThreadStt	*pT;
	ProcessStt	*pProcess;

	pProcess = k_get_current_process();
	if( pProcess == NULL )
		return( 0 );		// thread id 0 = error;

	//pT = kcreate_thread( pProcess, dwStackSize, dwR3Func, dwParam, nState );  2003-03-01
	pT = kcreate_thread( pProcess, dwStackSize, (DWORD)call_r3_thread, dwParam, nState );
	if( pT == NULL )
		return( 0 );		// creating thread failed!
	
	// set R3 entry function
	pT->dwR3EntryFunc = dwR3Func;
	
	return( pT->dwID );
}

static int ksc_set_fg_tid( DWORD dwTID )
{
	int			nR;
	ThreadStt 	*pT;

	pT = find_thread_by_id( dwTID );
	if( pT == NULL )
		return( -1 );	// Thread를 찾을 수 없다.


	kdbg_printf( "ksc_set_fg_tid(%d)\n", dwTID );

	nR = set_foreground_thread( pT );

	return( nR );	
}

static int ksc_set_thread_alias( DWORD dwTID, char *pAlias )
{
	ThreadStt	*pT;

	pT = find_thread_by_id( dwTID );
	if( pT == NULL )
		return( -1 );
	
	k_set_thread_alias( pT, pAlias );

	return( 0 );
}

static int ksc_set_process_alias( DWORD dwPID, char *pAlias )
{
	ProcessStt	*pP;

	pP = find_process_by_id( dwPID );
	if( pP == NULL )
		return( -1 );
	
	k_set_process_alias( pP, pAlias );

	return( 0 );
}

static DWORD ksc_get_cur_pid()
{
	ProcessStt	*pP;
	
	pP = k_get_current_process();
	if( pP == NULL )
		return( 0 );

	return( pP->dwID );
}

int ksc_get_r3exp_tbl( R3ExportTblStt *pR3Ex )
{
	ProcessStt *pP;

	pP = k_get_current_process();
	if( pP == NULL )
		return( -1 );

	memcpy( pR3Ex, &pP->e, sizeof( R3ExportTblStt ) );
	
	return( 0 );
}

static int ksc_exit_user_thread( DWORD dwTID )
{
	ThreadStt *pT, *pCur;

	pCur = get_current_thread();
	if( dwTID == 0 )
		pT = pCur;	// 현재 Thread를 종료한다.
	else
	{	// 종료할 Thread를 찾는다.
		pT = find_thread_by_id( dwTID );
		if( pT == NULL )
			return( -1 );
	}

	pT->dwKillerTID = 0;
	if( pCur != pT )
	{	// 종료할 놈이 현재 Thread가 아닌 경우에만 kmesg를 전달하도록 한다.
		pT->dwKillerTID = pCur->dwID;
	}
	
	kill_thread( pT );
	
	return( 0 );
}

#pragma data_seg( "data2" )
static SCallTblStt ksyscall_tbl[] = {
	{ SCTYPE_TTYOUT,				(DWORD)ksc_tty_out  			},
	{ SCTYPE_GET_CURSOR_XY,			(DWORD)ksc_get_cursor_xy		},
	{ SCTYPE_SET_CURSOR_XY,			(DWORD)ksc_set_cursor_xy		},
	{ SCTYPE_SET_FG_PROCESS,		(DWORD)ksc_set_fg_process		},
	{ SCTYPE_KBHIT,	         		(DWORD)ksc_kbhit				},
    { SCTYPE_DIRECT_DISPSTR,    	(DWORD)ksc_direct_dispstr		},
	{ SCTYPE_OPENDIR,				(DWORD)ksc_opendir				},
	{ SCTYPE_READDIR,				(DWORD)ksc_readdir				},
	{ SCTYPE_CLOSEDIR,				(DWORD)ksc_closedir				},
	{ SCTYPE_DEL_LINE,				(DWORD)ksc_del_line				},
	{ SCTYPE_FORK,					(DWORD)ksc_fork					},
	{ SCTYPE_WAIT,					(DWORD)ksc_wait					},
	{ SCTYPE_WAITPID,				(DWORD)ksc_waitpid				},
	{ SCTYPE_EXIT,					(DWORD)ksc_exit					},		
	{ SCTYPE_EXECVE,				(DWORD)ksc_execve				},
	{ SCTYPE_CREATE_SEMAPHORE, 		(DWORD)ksc_sem_create			},
	{ SCTYPE_OPEN_SEMAPHORE,    	(DWORD)ksc_sem_open				},
	{ SCTYPE_CLOSE_SEMAPHORE,   	(DWORD)ksc_sem_close			},
	{ SCTYPE_INC_SEMAPHORE,     	(DWORD)ksc_sem_post				},
	{ SCTYPE_DEC_SEMAPHORE,     	(DWORD)ksc_sem_wait				},
	{ SCTYPE_IS_GUI_MODE,			(DWORD)ksc_is_gui_mode			},
	{ SCTYPE_COPY_KEXECVE_PARAM,	(DWORD)ksc_copy_kexecve_param	},
	{ SCTYPE_GET_TIME,				(DWORD)ksc_get_time				},
	{ SCTYPE_CREATE_THREAD,			(DWORD)ksc_create_thread 		},
	{ SCTYPE_SET_FG_TID,			(DWORD)ksc_set_fg_tid 			},		
	{ SCTYPE_SET_THREAD_ALIAS,  	(DWORD)ksc_set_thread_alias 	},
	{ SCTYPE_SET_PROCESS_ALIAS,		(DWORD)ksc_set_process_alias 	},
	{ SCTYPE_GET_CUR_PID,			(DWORD)ksc_get_cur_pid			},
	{ SCTYPE_GET_R3EXP_TBL,			(DWORD)ksc_get_r3exp_tbl		},
	{ SCTYPE_WAITTID,				(DWORD)ksc_waittid				},
	{ SCTYPE_EXIT_USER_THREAD, 		(DWORD)ksc_exit_user_thread		},
		
	{ 0, 0 },
};
#pragma data_seg()

// Type에 따라 실제 system call 함수를 찾아 호출한다.
static DWORD ksyscall_stub( SCallTblStt *pTbl, SC_FUNC *pAddrTbl, int nTotalParam, DWORD *pParam )
{
    int     nI, nR;
    DWORD   dwFunc;
	
	if( pTbl == NULL || pAddrTbl == NULL || pParam[0] >= TOTAL_SYSTEM_CALL )
		return( SYSCALL_INVALID_ID );			    // 잘못된 ID

	// 테이블에 없으면 새로 찾는다.
    if( pAddrTbl[ pParam[0] ] == NULL )
	{	// 함수를 찾는다.
		for( nI = 0; ; nI++ )
		{
			if( pTbl[nI].dwFunc == 0 )    
				return( SYSCALL_FUNC_NOT_FOUND );   // 함수를 찾을 수 없다.
	        if( pTbl[nI].dwType == pParam[0] )
			{
				pAddrTbl[ pParam[0] ] = (SC_FUNC)pTbl[nI].dwFunc;
				break;								// 찾았다.
			}
		}
    }

    dwFunc = (DWORD)pAddrTbl[ pParam[0] ];

    _asm {
        MOV  ESI, pParam
        MOV  ECX, nTotalParam
        DEC  ECX
        PUSH ECX
        SHL  ECX, 2             
        ADD  ESI, ECX
        POP  ECX

PUSH_NEXT:  
        CMP  ECX, 0
        JE   CALL_X
             MOV  EAX, [ESI]
             SUB  ESI, 4        // 뒤에서 부터 거꾸로 PUSH해야 함.
             PUSH EAX
             DEC  ECX
             JMP  PUSH_NEXT

CALL_X: MOV  EAX, dwFunc
        CALL EAX
        MOV  nR, EAX
        MOV  ECX, nTotalParam
        DEC  ECX
        SHL  ECX, 2             
        ADD  ESP, ECX
    }

    return( nR );
}
            
// 인터럽트 핸들러.
_declspec(naked) void ksyscall_handler()
{
	static ProcessStt 		*pP;
    static int   			nTotalParam;
	static DWORD 			*pParam, dwR, dwR3Exp;

	_asm {
		PUSH EBP
		MOV  EBP, ESP
		PUSHFD
		PUSHAD
		
		MOV  DX,GSEL_DATA32
		MOV  DS,DX
		MOV  ES,DX
		MOV  FS,DX
		MOV  GS,DX
		STI	         // must enable interrupt

		MOV  nTotalParam, EAX;
        MOV  pParam,      EBX;
	}

    dwR = ksyscall_stub( ksyscall_tbl, syscall_addr_tbl, nTotalParam, pParam );	

	dwR3Exp = 0;
	pP = k_get_current_process();
	if( pP != NULL )
		dwR3Exp = (DWORD)&pP->e;

	_asm {
		POPAD
		POPFD
		POP EBP
        MOV EAX, dwR    // 리턴 값.
        MOV EBX, dwR3Exp
		IRETD
	}
}

// gra.mod에서 설정한다.
static SCallTblStt *G_pGraCallTbl     = NULL;
static SC_FUNC     *G_pGraCallAddrTbl = NULL;

// gui.mod 모듈이 시작될 때 포인터가 설정되고 종료될 때 NULL로 리셋된다.
void set_grxcall_tbl( void *pFuncTbl, void *pCacheAddrTbl )
{
	G_pGraCallTbl     = (SCallTblStt*)pFuncTbl;
	G_pGraCallAddrTbl = (SC_FUNC*)pCacheAddrTbl;
}

// Graphic Interrupt Handler
void kgrxcall_handler();
_declspec(naked) void kgrxcall_handler()
{
    static int   nTotalParam;
	static DWORD *pParam, dwR;

	_asm {
		PUSH EBP
		MOV  EBP, ESP
		PUSHFD
		PUSHAD
		
		MOV  DX,GSEL_DATA32
		MOV  DS,DX
		MOV  ES,DX
		MOV  FS,DX
		MOV  GS,DX
		STI	         // must enable interrupt

		MOV  nTotalParam, EAX;
        MOV  pParam,      EBX;
	}

	// graphic mode인지 확인한 후 진입한다.
	if( is_gui_mode() != 0 )
	{
		//kdbg_printf( "kgrxcall_handler: ksyscall_stub( 0x%X, 0x%X )\n", G_pGraCallTbl, G_pGraCallAddrTbl );
		dwR = ksyscall_stub( G_pGraCallTbl, G_pGraCallAddrTbl, nTotalParam, pParam );	
	}
	else
	{
		kdbg_printf( "kgrxcall_handler: 0\n" );
		dwR = 0;
	}

	_asm {
		POPAD
		POPFD
		POP EBP
        MOV EAX, dwR    // 리턴 값.
		IRETD
	}
}

