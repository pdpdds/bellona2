#include "bellona2.h"

SchStt	sch;

// 쓰래드의 Alias를 설정한다.
void k_set_thread_alias( ThreadStt *pThread, char *pAlias )
{
	if( pThread == NULL )
	{
		pThread = get_current_thread();
		if( pThread == NULL )
			return;
	}

	if( pAlias == NULL )
		pThread->szAlias[0] = 0;
	else
		strcpy( pThread->szAlias, pAlias );
}

// 프로세스의 Alias를 설정한다.
void k_set_process_alias( ProcessStt *pP, char *pAlias )
{
	if( pP == NULL )
	{
		pP = k_get_current_process();
		if( pP == NULL )
			return;
	}
	if( pAlias == NULL )
		pP->szAlias[0] = 0;
	else
		strcpy( pP->szAlias, pAlias );
}

DWORD get_thread_page_dir( ThreadStt *pT )
{
	return( (DWORD)pT->pProcess->pAddrSpace->pPD );
}

DWORD get_process_page_dir( ProcessStt *pP )
{
	return( (DWORD)pP->pAddrSpace->pPD );
}

// 프로세스 ID를 생성한다.
static DWORD dwMakeProcessThreadID( DWORD dwAddr )
{
	return( ++bell.dwNextProcessThreadID );
}

// 프로세스를 스케쥴 링크에 추가한다.
static int append_process_to_sch_q( SchStt *pSch, ProcessStt *pProc )
{
	ProcessStt	*pT;

	if( pSch == NULL )
		pSch = &sch;

	_asm {
		PUSHFD
		CLI
	}

	pT = pSch->pEndProcess;
	if( pSch->pStartProcess == NULL )
	{
		pSch->pStartProcess = pSch->pEndProcess   = pProc;
		pProc->pPreProcess  = pProc->pNextProcess = NULL;
	}
	else
	{
		pProc->pPreProcess = pSch->pEndProcess;
		pSch->pEndProcess->pNextProcess = pProc;
		pSch->pEndProcess   = pProc;
		pProc->pNextProcess = NULL;
	}

	pSch->nTotalProcess++;

	_asm POPFD

	return( 0 );
}

// 프로세스를 스케쥴 링크에서 제거한다.
static int delete_process_from_sch_q( SchStt *pSch, ProcessStt *pP )
{
	if( pSch == NULL )
		pSch = &sch;

	_asm {
		PUSHFD
		CLI
	}
	
	if( pP->pPreProcess == NULL )
		pSch->pStartProcess = pP->pNextProcess;
	else
		pP->pPreProcess->pNextProcess = pP->pNextProcess;

	if( pP->pNextProcess == NULL )
		pSch->pEndProcess = pP->pPreProcess;
	else
		pP->pNextProcess->pPreProcess = pP->pPreProcess;

	pSch->nTotalProcess--;

	pP->pPreProcess = pP->pNextProcess = NULL;

	_asm POPFD
		
	return( 0 );
}

// 쓰레드를 현재 현재 스케쥴 큐에 추가한다.
static int nAppendThreadToList( SchStt *pSch, ThreadStt *pThread )
{
	if( pSch->nTotalThread == 0 )
	{
		pSch->pStartThread   = pSch->pEndThread     = pThread;
		pThread->pPreSLink	 = pThread->pNextSLink	= NULL;
	}
	else
	{
		pThread->pPreSLink		     = pSch->pEndThread;
		pSch->pEndThread->pNextSLink = pThread;
		pSch->pEndThread			 = pThread;
		pThread->pNextSLink			 = NULL;
	}

	pSch->nTotalThread++;

	return( 0 );
}

// 쓰레드를 스케쥴 큐에서 제거한다.
static int nDeleteThreadFromList( SchStt *pSch, ThreadStt *pThread )
{
	if( pThread->pPreSLink == NULL )
		pSch->pStartThread = pThread->pNextSLink;
	else
		pThread->pPreSLink->pNextSLink = pThread->pNextSLink;

	if( pThread->pNextSLink == NULL )
		pSch->pEndThread = pThread->pPreSLink;
	else
		pThread->pNextSLink->pPreSLink = pThread->pPreSLink;

	pSch->nTotalThread--;

	return( 0 );
}

// 스택 링크를 프로세스 구조체에 연결한다.
static int append_Stack_link( ProcessStt *pProc, TStackLinkStt *pStk )
{
	// 스택 링크가 하나도 없었다.
	if( pProc->pStartStk == NULL )
	{
		 pProc->pStartStk = pProc->pEndStk = pStk;
		 pStk->pPre = pStk->pNext = NULL;
		 return(0);
	}

	// 릉크의 끝에 추가한다.
	pStk->pPre = pProc->pEndStk;
	pStk->pPre->pNext = pStk;
	pProc->pEndStk    = pStk;
	pStk->pNext       = NULL;

	return( 0 );
}

// make stack link structure
static int nFreeStackLink( ProcessStt *pProc, TStackLinkStt *pStk )
{
	pStk->pThread = NULL;

	return( 0 );
}

// Thread의 스택으로 사용될 영역을 Process로부터 예약해 둔다.
static DWORD reserve_thread_stack( ProcessStt *t_pProc, ThreadStt *t_pThread, DWORD t_dwSize )
{
	static TStackLinkStt	*pStk;
	static ProcessStt		*pProc;
	static ThreadStt		*pThread;
	static int				nR, nMapped;
	static DWORD			dwCR3, dwESP, dwSize, dwOrgCR3;

	pProc   = t_pProc;
	pThread = t_pThread;
	dwSize  = t_dwSize;

	// disable interrupt
	_asm {
		PUSHFD
		CLI
	}

	// change CR3 temporarily
	dwCR3 = get_thread_page_dir( pThread );
	set_cur_cr3_in_tss( dwCR3 );
	_asm {
		MOV EAX, CR3;
		MOV dwOrgCR3, EAX
		MOV EAX, dwCR3
		MOV CR3, EAX
		FLUSH_TLB2(dwCR3)
	}

	dwESP = 0;
	nMapped = 0;
	for( pStk = pProc->pStartStk; pStk != NULL; pStk = pStk->pNext )
	{	// FREE인 스택을 찾는다.  (사이즈 체크 2003-03-01)
		if( pStk->pThread == NULL && dwSize == pStk->dwSize )
			goto MAKE_STACK_LINK;
	}

	// mapping
	nR = nMappingUser( (DWORD*)get_process_page_dir( pProc ), pProc->dwNextStackBase, dwSize );
	if( nR < 0 )
	{
		kdbg_printf( "reserve_thread_stack: mapping failed!\n" );
		goto END_RSV_PSTACK;
	}
	nMapped = 1;

	// 스택구조체를 초기화 한다.
	pStk = (TStackLinkStt*)kmalloc( sizeof( TStackLinkStt ) );
	memset( pStk, 0, sizeof( TStackLinkStt ) );

	// 새로 매핑된 것이면 스택을 프로세스 링크에 추가한다.
	append_Stack_link( pProc, pStk );

	pStk->dwBaseAddr  = pProc->dwNextStackBase;
	pStk->dwSize       = dwSize;
	dwESP              = (DWORD)pStk->dwBaseAddr + pStk->dwSize - 4;	// stack bottom
	pStk->dwR0StackTop = dwESP;
	pStk->dwR3StackTop = dwESP - (dwSize/2);

	// 프로세스의 스택 탑을 증가 시킨다.  (다음에 할당할 스택 위치)
	pProc->dwNextStackBase += dwSize;

MAKE_STACK_LINK:
	dwESP = pStk->dwR0StackTop;

	// 스택의 OWner를 설정한다.
	pStk->pThread   = pThread;
	pThread->pStack = pStk;

END_RSV_PSTACK:
	// recover org CR3
	set_cur_cr3_in_tss( dwOrgCR3 );
	_asm {
		MOV EAX, dwOrgCR3
		MOV CR3, EAX
		FLUSH_TLB2(dwOrgCR3)
	}

	// enable interrupt, return( dwESP )
	_asm POPFD;

	return( dwESP );
}

// 쓰레드를 특정 'Q'에 추가한다.
int nPushThread( SQStt *pQ, ThreadStt *pThread )
{
	// Thread내에 있는 'Q'포인터를 변경한다.
	pThread->pQ = pQ;

	if( pQ->nTotal == 0 )
	{	// 연결된 Thread가 하나도 없다.
		pQ->pStartThread    = pQ->pEndThread       = pThread;
		pThread->pPreQLink  = pThread->pNextQLink  = NULL;
	}
	else
	{	// 'Q'의 가장 끝에 연결한다.
		pThread->pPreQLink			= pQ->pEndThread;
		pThread->pNextQLink			= NULL;
		pQ->pEndThread->pNextQLink	= pThread;
		pQ->pEndThread				= pThread;
	}

	pQ->nTotal++;

	return( 0 );
}

// 쓰레드를 큐에서 제거한다.
int nPopThread( ThreadStt *pThread )
{
	SQStt *pQ;

	if( pThread == NULL || pThread->pQ == NULL )
		return( 0 );

	pQ = pThread->pQ;

	if( pThread->pPreQLink == NULL )
		pQ->pStartThread = pThread->pNextQLink;
	else
		pThread->pPreQLink->pNextQLink = pThread->pNextQLink;

	if( pThread->pNextQLink == NULL )
		pQ->pEndThread = pThread->pPreQLink;
	else
		pThread->pNextQLink->pPreQLink = pThread->pPreQLink;

	pThread->pQ = NULL;
	pQ->nTotal--;

	return( 0 );
}

// 유효한 쓰레드인지 확인한다.
int is_thread( ThreadStt *pThread )
{
	if( pThread == NULL || memcmp( pThread->szMagicStr, THREAD_MAGIC_STR, strlen( THREAD_MAGIC_STR ) ) != 0 )
		return( 0 );	// 잘못된 쓰레드.
	else
		return( 1 );
}

// 유효한 프로세스인지 확인한다.
int is_process( ProcessStt *pProcess )
{
	if( pProcess == NULL || memcmp( pProcess->szMagicStr, PROC_MAGIC_STR, strlen( PROC_MAGIC_STR ) ) != 0 )
		return( 0 );	// 잘못된 프로세스.
	else
		return( 1 );
}

// 쓰레드 상태를 변경한다.
// 현재 큐에서 제거되어 새로운 상태의 큐에 들어간다.
int change_thread_state( SchStt *pSch, ThreadStt *pThread, int nState )
{
	SQStt	*pNewQ;
	int		nR;

	if( !is_thread( pThread ) )
		return( -1 );

	_asm {
		PUSHFD
		CLI
	}

	nR = 0;

    // 쓰레드가 링크될 새로운 큐
	if( nState < MAX_SCH_Q )
	{
		if( pSch != NULL )
			pNewQ = &pSch->q[nState];
		else
			pNewQ = &sch.q[nState];
	}
	else
		pNewQ = NULL;

	// OLD 큐에서 쓰레드를 꺼낸다.
	nR = nPopThread( pThread );
	if( nR == -1 )
		goto BACK;

	// 새로운 큐에 쓰레드를 넣는다.
	if( pNewQ != NULL )
	{
		nR = nPushThread( pNewQ, pThread );
		if( nR == -1 )
			goto BACK;
	}

	pThread->nState = nState;

BACK:
	_asm POPFD

	return( nR );
}

// Thread가 처음 생성되었을 당시 제어가 시작되는 함수
_declspec(naked) void kthread_entry_point()
{	// EAX에 실제로 Call할 함수가 있다.
	static DWORD				dwR, dwFunc, dwParam;
	static THREAD_ENTRY_FUNC	pFn;

	_asm {
		MOV dwFunc,  EAX;	// CALL할 함수의 주소는 EAX에
		MOV dwParam, EBX;	// 함수에 전달한 Parameter는 EBX에 전달된다.
		PUSHAD;
		PUSHFD
		CLD
		STI					// 2003-08-16  이걸 안해주면 새로운 쓰레드가 CLI된 상태로 실행된다.
	}

	pFn = (THREAD_ENTRY_FUNC)dwFunc;
	dwR = pFn( dwParam );	// 실제 함수를 호출한다.

	_asm {
		POPFD
		POPAD;
		MOV EAX, dwR;		// 결과값은 EAX에 저장된다.
		IRETD;				// TASK Switching에 의해 제어가 넘어왔을 것이므로 IRETD해 준다.
	}
}

//////////////////////////////////////////////////////////////////
// 스케쥴러에 등록된 프로세스의 정보를 출력한다.
void disp_process_list()
{
	int			nI, nVConID;
	char		ch;
	SchStt		*pSch;
	DWORD		dwPID;
	ProcessStt	*pP, *pCurProc;

	pSch = &sch;
	if( pSch->nTotalProcess == 0 )
	{	// 프로세스가 하나도 없다.
		kdbg_printf( "no process.\n" );
		return;
	}

	pP = pSch->pStartProcess;
	pCurProc = k_get_current_process();

	kdbg_printf( "          Addr     ID   PID  Threads   PageDir  VCon  Alias\n" );
	for( nI = 0; pP != NULL; pP = pP->pNextProcess, nI++ )
	{
		// Parent 아이디를 구한다.
		dwPID = pP->dwParentID;

		// 현재 프로세스에 '*'을 추가한다.
		if( pP == pCurProc )
			ch = '*';
		else
			ch = ' ';

		// 가상 콘솔 아이디를 구한다.
		if( pP->pVConsole == NULL )
			nVConID = 0;
		else
			nVConID = pP->pVConsole->nID;

		// 프로세스 정보를 출력한다.
		kdbg_printf( "[%2d] %c0x%08X  %3d   %3d   %3d    0x%08X  %3d  %s\n",
            nI, ch, (DWORD)pP, pP->dwID, dwPID, pP->nTotalThread, get_process_page_dir( pP ),
			nVConID, pP->szAlias );
	}

	// Zombie Process도 출력 한다. (2004-03-26)
	for( pP = pSch->pStartZombie; pP != NULL; pP = pP->pNextProcess )
	{
		// 프로세스 정보를 출력한다.
		kdbg_printf( "ZOMB  0x%08X  %3d   %3d                           %s\n", 
			(DWORD)pP, pP->dwID, pP->dwParentID, pP->szAlias );
	}
}

typedef struct {
	int nState;
	char *pStr;
} ThreadStateStrEntStt;
static char *pUnknownStr = "UNKNOWN";
static ThreadStateStrEntStt thread_state_str[] = {
    { TS_READY_TIME_CRITICAL , "READY_TIME_CRITICAL"	},
    { TS_READY_HIGHEST       , "READY_HIGHEST"			},
    { TS_READY_ABOVE_NORMAL  , "READY_ABOVE_NORMAL"		},
    { TS_READY_NORMAL        , "READY_NORMAL"			},
    { TS_READY_BELOW_NORMAL  , "READY_BELOW_NORMAL"		},
    { TS_READY_LOWEST        , "READY_LOWEST"			},
    { TS_READY_IDLE          , "READY_IDLE"				},
    { TS_READY_LAZY          , "READY_LAZY"				},
    { TS_WAIT                , "WAIT"					},
    { TS_SUSPEND             , "SUSPENDED"				},
    { TS_TERMINATED          , "TERMINATED"				},
    { TS_ERROR				 , "ERROR"					},

    { 0, NULL }
};

// Thread의 상태 스트링을 얻는다.
static char *get_thread_state_str( int nState )
{
	int nI;

	for( nI = 0; thread_state_str[nI].pStr != NULL; nI++ )
	{
		if( thread_state_str[nI].nState == nState )
			return( thread_state_str[nI].pStr );
	}
	return( pUnknownStr );
}

static char *get_wait_type_str( WaitObjStt *pW )
{
	if( pW == NULL )
		return( "ERROR" );

	if( pW->wWaitType == WAIT_TYPE_EVENT )
	{
		if( pW->pE == NULL )
			return( "EVENT ??" );
		else
			return( pW->pE->szName );
	}
	else if( pW->wWaitType == WAIT_TYPE_KMESG )
		return( "KMESG" );		

	return( "UNKNOWN" );
}


// 스케쥴러에 등록된 Thread의 정보를 출력한다.
void disp_thread_list()
{
	int			nI;
	char		c, c2;
	SchStt		*pSch;
	char		*pEventName;
	ThreadStt	*pT, *pCurThread;
	DWORD		dwPID, dwDebugeeTID;

	pSch = &sch;
	if( pSch->nTotalThread == 0 )
	{
		kdbg_printf( "no threads.\n" );
		return;
	}

    pCurThread 		= get_current_thread();
	pT 				= pSch->pStartThread;
	dwDebugeeTID 	= kdbg_get_debugee_tid();
	if( dwDebugeeTID == 0 )
		dwDebugeeTID = pT->dwID;

	kdbg_printf( "      Alias         TID    PID     Addr    State\n" );
	for( nI = 0; pT != NULL; pT = pT->pNextSLink, nI++ )
	{
		c = c2 = ' ';
		if( pT == pCurThread )
			c = '*';
		if( pT->dwID == dwDebugeeTID )
			c2 = '<';	

		// wait 상태인 경우에만 Wait 대상 문자열을 구한다.
		pEventName = "";
		if( pT->nState == TS_WAIT )
			pEventName = get_wait_type_str( pT->pStartWaitObj );

		if( pT->pProcess != NULL )
			dwPID = pT->pProcess->dwID;
		else
			dwPID = 0;

		kdbg_printf( "[%2d]%c%-12s %5d%c %5d   %08X  %s %s\n",
			nI, c, pT->szAlias, pT->dwID, c2, dwPID, (DWORD)pT,
			get_thread_state_str( pT->nState ), pEventName );
	}
}

// get the current running thread
ThreadStt *get_current_thread()
{
	return( sch.pCurrentThread );
}
/*
// set the process to foreground process
int set_foreground_process( ProcessStt *pProcess )
{
	if( pProcess == NULL )
	{
		pProcess = k_get_current_process();
		if( pProcess == NULL )
			return( -1 );
	}

	//sch.pForegroundProcess = pProcess;

	// 프로세스의 가상 콘솔에 FG로 설정한다.
	if( pProcess->pVConsole != NULL )
		pProcess->pVConsole->pFgProcess = pProcess;
	else
		kdbg_printf( "process %d has no vconsole!\n", pProcess->dwID );

	return( 0 );
}
*/

// set the pThread to foreground thread
int set_foreground_thread( ThreadStt *pT )
{
	if( pT == NULL || pT->pProcess == NULL )
		return( -1 );
	
	if( pT->pProcess->pForegroundThread == pT )
		return( 1 );		//이미 설정된 상태.
		
	pT->pProcess->pForegroundThread = pT;
	
	// 설정됨.
	return( 0 );
}

// 쓰레드를 FG로 설정하고 Parent Process를 FG로 설정한다.
int set_global_foreground_thread( ThreadStt *pThread )
{
	int nR;

	if( pThread == NULL )
	{
		kdbg_printf( "set_global_foreground_thread() - thread is bull!\n" );
		return( -1 );
	}

	nR = set_fg_process( NULL, pThread->pProcess );
	nR = set_foreground_thread( pThread );

	return( nR );
}

// 현재 프로세스의 FG 프로세스를 구한다.
ProcessStt *get_fg_process()
{
	ProcessStt *pP;

	pP = k_get_current_process();
	if( pP == NULL || pP->pVConsole == NULL )
		return( NULL );

	return( pP->pVConsole->pStartFg );
}

// Active Console의 FG 프로세스를 구한다.
ProcessStt *get_sys_fg_process()
{
	VConsoleStt *pVC;

	pVC = get_active_vconsole();
	if( pVC == NULL )
		return( NULL );

	return( pVC->pStartFg );
}

// process에서 fg thread를 구한다.
ThreadStt *get_fg_thread( ProcessStt *pProcess )
{
	if( pProcess == NULL )
		return( NULL );

	return( pProcess->pForegroundThread );
}

// get current process
ProcessStt *k_get_current_process()
{
	ThreadStt *pT;

	pT = get_current_thread();
	if( pT == NULL )
		return( NULL );

	return( pT->pProcess );
}

// 쓰레드를 프로세스의 링크에 추가한다.
static int insert_thread_to_process( ProcessStt *pProcess, ThreadStt *pThread )
{
	if( pProcess == NULL || pThread == NULL )
	{
		kdbg_printf( "insert_thread_to_process: invalid process(0x%X) or thread(0x%X).\n",
			(DWORD)pProcess, (DWORD)pThread );
		return( -1 );
	}

	pThread->pPrePLink = pThread->pNextPLink = NULL;
	if( pProcess->pEndThread == NULL )
		pProcess->pStartThread = pProcess->pEndThread = pThread;
	else
	{
		pProcess->pEndThread->pNextPLink = pThread;
		pThread->pPrePLink				 = pProcess->pEndThread;
		pProcess->pEndThread			 = pThread;
	}

	pProcess->nTotalThread++;

	return( 0 );
}

// delete thread from process
static int delete_thread_from_process( ThreadStt *pThread )
{
	ProcessStt *pProcess;

	if( pThread == NULL || pThread->pProcess == NULL )
	{
		kdbg_printf( "delete_thread_to_process() - invalid thread or its owner process is null.\n" );
		return( -1 );
	}

	pProcess = pThread->pProcess;

	if( pThread->pPrePLink == NULL )
		pProcess->pStartThread = pThread->pNextPLink;
	else
		pThread->pPrePLink->pNextPLink = pThread->pNextPLink;

	if( pThread->pNextPLink == NULL )
		pProcess->pEndThread = pThread->pPrePLink;
	else
		pThread->pNextPLink->pPrePLink = pThread->pPrePLink;

	pThread->pPrePLink = pThread->pNextPLink = NULL;
	pProcess->nTotalThread--;

	return( 0 );
}

// 쓰레드를 생성한다.
ThreadStt *kcreate_thread( ProcessStt *pProc, DWORD dwStackSize, DWORD dwFunc,
						  DWORD dwParam, int nState )
{
	ThreadStt	*pThread;
	DWORD		*pPD;
	DWORD		dwESP;
	char		szT[128];

	if( pProc == NULL )
	{	// 프로세스가 지정되어 있지 않으면 현재 프로세스 아래에 생성한다.
		pProc = k_get_current_process();
		if( pProc == NULL )
		{
		    kdbg_printf( "kcreate_thread: current process is NULL!\n" );
			return( NULL );
		}
	}

	// Owner Process의 Page Dir을 사용한다.
	pPD = (DWORD*)get_process_page_dir( pProc );

	// Thread 구조체를 할당한 후 초기화 한다.
	pThread = (ThreadStt*)kmalloc( sizeof( ThreadStt ) );
	if( pThread == NULL )
	{
		kdbg_printf( "kcreate_thread: alloc thread failed!\n" );
		return( NULL );
	}
	memset( pThread, 0, sizeof( ThreadStt ) );
	strcpy( pThread->szMagicStr, THREAD_MAGIC_STR );					
	pThread->dwID		= dwMakeProcessThreadID( (DWORD)pThread );	
	pThread->pProcess	= pProc;									
	pThread->dwNice		= DEFAULT_THREAD_NICE;						
	pThread->nState		= -1;

	// 쓰레드를 스케쥴 구조체에 추가한다.
	nAppendThreadToList( &sch, pThread );

	// 쓰레드를 초기 상태 큐에 추가한다.
	change_thread_state( &sch, pThread, nState );

	// TSS 구조체를 할당한 후 초기화 한다.
	pThread->pTSS = (TSSStt*)kmalloc( sizeof( TSSStt ) );
	if( pThread->pTSS == NULL )
	{
		kdbg_printf( "kcreate_thread: tss allocation failed!\n" );
		kfree( pThread );	// Release thread structure
		return( NULL );
	}
	// make TSS.  (쓰레드 스택을 만들기 전에 TSS를 초기화해야 한다.)
	vMakeTSS( pThread->pTSS, (DWORD)kthread_entry_point, 0 );  // 주의! ESP에 일단 0을 집어 넣는다.
	pThread->pTSS->dwCR3 = (DWORD)pPD;	// set CR3

	// register tss
	sprintf( szT, "T%d", pThread->dwID );
	nAppendTSSTbl( szT, (DWORD)pThread->pTSS );

	// 쓰레드 스택을 잡아 준다.
	if( dwStackSize != KTHREAD_NO_STACK )
	{	// mapping thread stack
		if( dwStackSize < KTHREAD_STACK_SIZE )	 // 무조건 일정 크기 이상을 할당한다.
			dwStackSize = KTHREAD_STACK_SIZE;

		dwESP = reserve_thread_stack( pProc, pThread, dwStackSize );
		if( dwESP == 0 )
		{	// 쓰레드 스택을 할당할 수 없다.
			kdbg_printf( "kcreate_thread: reserve_thread_stack failed!\n" );
			return( NULL );
		}
		// TSS의 ESP값을 다시 설정해 준다.
		pThread->pTSS->dwESP  = dwESP;
		pThread->pTSS->dwESP0 = dwESP;
	}
	else
	{	// 복제할 쓰레드이므로 스택을 할당하지 않는다.
	}

	// EAX 레지스터에 Call할 Function의 주소를 저장하면 kthread_entry_point에서 콜된다.
	pThread->pTSS->dwEAX = dwFunc;
	pThread->pTSS->dwEBX = dwParam;		// set parameter to ebx

	// 쓰레드를 프로세스에 추가한다.
	insert_thread_to_process( pProc, pThread );

	// 쓰레드의 키입력 큐를 할당한다.
	alloc_thread_kbd_q( pThread );

	return( pThread );
}

// 쓰레드를 close하고 리소스를 회수한다.
// 이 함수는 init_task의 context 상에서 호출된다.
// 페이지 디렉토리는 프로세스 소관이므로 건드리면 안된다.
int kclose_thread( ThreadStt *pThread )
{
	int nR;

	// 유효한 쓰레드인가?
	if( pThread == NULL || memcmp( pThread->szMagicStr, THREAD_MAGIC_STR, 6 ) != 0 )
	{
		kdbg_printf( "kclose_thread: invalid thread!\n" );
		return( -1 );
	}

	// 'FG' Thread이면 NULL로 설정한다.  2002-12-08
	if( pThread->pProcess != NULL && pThread->pProcess->pForegroundThread == pThread )
	{
		pThread->pProcess->pForegroundThread = NULL;
		//kdbg_printf( "process.FG <- NULL\n" );
	}

	// 쓰레드의 상태를 변경한다. (TS_TERMINATED)
	nR = change_thread_state( &sch, pThread, TS_TERMINATED );
	if( nR < 0 )
	{
		kdbg_printf( "change_thread_state( TS_TERMINATED ) failed!\n" );
		return( -1 );
	}

	// Owner 프로세스에서 떼어낸다.
	nR = delete_thread_from_process( pThread );
	if( nR < 0 )
	{
		kdbg_printf( "delete_thread_from_process() failed!\n" );
		return( -1 );
	}

	// 쓰레드의 KBQ Q를 해제한다.
	free_thread_kbd_q( pThread );

	// Wait Object를 해제한다.
	nR = release_thread_wait_object( pThread );
	if( nR < 0 )
	{
		kdbg_printf( "kclose_thread() - release wait objects failed!\n" );
		return( -1 );
	}

	// 쓰레드 스택을 해제한다.  (프로세스 링크에는 여전히 걸려 있다.)
	pThread->pStack->pThread = NULL;
	pThread->pStack = NULL;

	// Current Thread이면 NULL로 설정한다.  2002-12-08
	if( pThread == get_current_thread() )
	{
		sch.pCurrentThread = NULL;
		//kdbg_printf( "Current thread <- NULL\n" );
	}

	return( 0 );
}

// 	쓰레드 구조체를 해제한다.
int krelease_thread_struct( ThreadStt *pThread )
{
	int nR;

	if( pThread == NULL || pThread->nState != TS_TERMINATED )
	{
		kdbg_printf( "release_thread_struct() - invalid thread or thread state is not TS_TERMINATED!\n" );
		return( -1 );
	}

	// 쓰레드 구조체를 TS_TERMINATED 큐에서 제거한다.
 	nR = nPopThread( pThread );
	if( nR == -1 )
	{
		kdbg_printf( "release_thread_struct() - pop thread from TS_TERMINTED failed!\n" );
		return( -1 );
	}

	// 스케쥴링크에서 쓰레드를 제거한다.
	nDeleteThreadFromList( &sch, pThread );

	// TSS를 제거한다.
	kfree( pThread->pTSS );

	// ThreadStt를 해제한다.
	kfree( pThread );

	return( 0 );
}

// 프로세스의 주소 공간 구조체를 할당한다.
// 페이지 디렉토리를 할당한다.
static AddrSpaceStt *alloc_addr_space()
{
	AddrSpaceStt *pA;

	pA = (AddrSpaceStt*)kmalloc( sizeof( AddrSpaceStt) );
	if( pA == NULL )
		return( NULL );

	memset( pA, 0, sizeof( AddrSpaceStt ) );

	// 초기 값을 1로 설정해야 한다.
	pA->nForkRefCounter = 1;

	// PageDir을 할당하고 커널의 것을 복사한다.
	pA->pPD = pAllocPageTable();
    memcpy( pA->pPD, bell.pPD, 4096 );

	return( pA );
}

// 새로운 프로세스를 생성한다.  (쓰레드는 없는 상태)
ProcessStt *kcreate_process( ProcessStt *pParent )
{
	ProcessStt	*pProc;

	// 프로세스 구조체를 할당하고 초기화 한다.
	pProc = (ProcessStt*)kmalloc( sizeof( ProcessStt ) );
	if( pProc == NULL )
	{	// 메모리 할당 에러.
M_ERR:	kdbg_printf( "kcreate_process: memory allocation error!\n" );
		return( NULL );
	}
	memset( pProc, 0, sizeof( ProcessStt ) );

	// 주소공간 구조체를 할당한다.
	pProc->pAddrSpace = alloc_addr_space();
	if( pProc->pAddrSpace == NULL )
	{	// 메모리 할당 에러.
		kfree( pProc );
		goto M_ERR;
	}

	strcpy( pProc->szMagicStr, PROC_MAGIC_STR );
	pProc->dwID		    	= dwMakeProcessThreadID( (DWORD)pProc );
	pProc->dwNextStackBase 	= KPROCESS_STACK_TOP;
	if( pParent != NULL )
		pProc->dwParentID = pParent->dwID;

	// Address Space의 OWner PID를 설정한다.  (PID 생성후 설정할 것.)
	pProc->pAddrSpace->nOwnerPID = pProc->dwID;

	// 프로세스를 스케쥴 큐에 추가한다.
	append_process_to_sch_q( &sch, pProc );

	// FileName, Arg, EvnString 등을 저장할 공간 할당.
	// pProc->pExecParam = (ExecParamStt*)kmalloc( sizeof( ExecParamStt ) );

	// 프로세스의 메모리 풀을 초기화 한다.
	// MEM_POOL_SIZE must be 0!
	init_memory_pool( (DWORD*)get_process_page_dir( pProc ), &pProc->mp, PROCESS_MEM_POOL_ADDR, 0 );

	return( pProc );
}

// 프로세스 가운데 child wait 상태인 쓰레드가 있으면 그 쓰레드 포인터를 리턴한다.
static ThreadStt *find_waitpid_thread( ProcessStt *pParent )
{
	ThreadStt	*pT;
	WaitObjStt	*pWaitObj;

	for( pT = pParent->pStartThread; pT != NULL; pT = pT->pNextPLink )
	{
		if( pT->pStartWaitObj == NULL || pT->nState != TS_WAIT )
			continue;

		pWaitObj = pT->pStartWaitObj;
		if( pWaitObj->wWaitType == WAIT_TYPE_KMESG && pWaitObj->wWaitSubType == KMESG_CHILD_PROCESS_EXIT )
			return( pT );
	}

	return( NULL );
}

// 프로세스를 스케쥴큐의 zombie 링크에 추가한다.
static int link_to_zombie( ProcessStt *pP )
{
	SchStt	*pQ;

	pQ = &sch;

	if( pQ->pStartZombie == NULL )
	{
		pQ->pStartZombie = pQ->pEndZombie = pP;
		pP->pPreProcess = pP->pNextProcess = NULL;
	}
	else
	{
		pP->pPreProcess					= pQ->pEndZombie;
		pP->pNextProcess				= NULL;
		pQ->pEndZombie->pNextProcess	= pP;
		pQ->pEndZombie					= pP;
	}

	// 상태를 좀비로 설정한다.
	pP->dwState = PSTATE_ZOMBIE;

	return( 0 );
}

// 프로세스를 스케쥴큐의 zombie 링크에서 제거한다.
static int unlink_from_zombie( ProcessStt *pP )
{
	SchStt	*pQ;

	pQ = &sch;

	if( pP->pPreProcess == NULL )
		pQ->pStartZombie = pP->pNextProcess;
	else
		pP->pPreProcess->pNextProcess = pP->pNextProcess;

	if( pP->pNextProcess == NULL )
		pQ->pEndZombie = pP->pPreProcess;
	else
		pP->pNextProcess->pPreProcess = pP->pPreProcess;

	return( 0 );
}

// 주소 공간을 프로세스로부터 떼어낸다.
int detach_addr_space( ProcessStt *pP )
{
	AddrSpaceStt *pA, *pNext;

	//kdbg_printf( "detach_addr_space\n" );

	pA = pP->pAddrSpace;
	if( pA == NULL )
	{
		kdbg_printf( "detach_addr_space: addr space is null!\n" );
		return( -1 );
	}
	pP->pAddrSpace = NULL;

	for( ; pA != NULL; pA = pNext)
	{
		pNext = pA->pForkRef;

		// Reference Counter를 1 줄이고 '0'이면 주소공간을 해제한다.
		pA->nForkRefCounter--;
		if( pA->nForkRefCounter <= 0 )
		{	
			// 순수하게 내가 관할하는 영역인가?
			if( pA->pForkRef == NULL )
			{	// 사용자 영역의 매핑을 모두 해제한다.
				release_user_area( pA );
				nFreePageTable( (DWORD)pA->pPD );
				//kdbg_printf( "release all user area( PID = %d )\n", pA->nOwnerPID );
			}
			else
			{	// RW만 내가 할당한 영역이다.
				release_rw_user_area( pA );
				//kdbg_printf( "release rw user area( PID = %d )\n", pA->nOwnerPID );
			}

			kfree( pA );
		}
	}

	return( 0 );
}

// 	프로세스 구조체를 해제한다.
int krelease_process_struct( ProcessStt *pProcess )
{
	int			nR;
	ThreadStt	*pT;
	ProcessStt	*pParent;

	// 프로세스를 스케쥴 큐에서 제거한다.
	nR = delete_process_from_sch_q( &sch, pProcess );

	//if( pProcess->pExecParam != NULL )
    //{   // FileName, Arg, EvnString 등을 저장할 공간 해제.
	//	kfree( pProcess->pExecParam );
	//	pProcess->pExecParam = NULL;
    //}

	// pExecveParam을 해제한다.
	if( pProcess->pKExecveParam != NULL )
	{	// 커널 영역일 경우에만 kfree로 해제한다.
		if( ( (DWORD)pProcess->pKExecveParam & 0x80000000 ) == 0 )
			kfree( pProcess->pKExecveParam );
	}

	// parent process가 살아 있으면 zombie로 만든다.
	pParent = find_process_by_id( pProcess->dwParentID );
	if( pParent != NULL )
	{
		// 우선 Zombie 링크에 추가한다.
		link_to_zombie( pProcess );

		// parent의 thread 가운데 wait child 상태인 쓰레드가 있는지 찾아본다.
		pT = find_waitpid_thread( pParent );
		if( pT != NULL )
		{
			kdbg_printf( "wakeup thread( %d )\n", pT->dwID );
  
			ksend_kmesg( pT->dwID, KMESG_CHILD_PROCESS_EXIT, pProcess->dwID, 0 );
		}
		else
			kdbg_printf( "krelease_process_struct: no waitpid thread!\n" );
	}
	else
	{
		// 그냥 바로 해제해 버린다.
		kfree( pProcess );
	}

	return( 0 );
}

// kill thread
int kill_thread( ThreadStt *pThread )
{
	ThreadStt	*pT, *pCurT;
	
	// sniper thread를 찾는다.
	pT = find_thread_by_alias( "init_sniper" );
	if( pT == NULL )
	{
		kdbg_printf( "kill_thread: sniper thread not found!\n" );
		return( -1 );
	}
	
	// sniper쪽으로 kmesg를 날린다.
	ksend_kmesg( pT->dwID, KMESG_KILL_THREAD, pThread->dwID, 0 );

	pCurT = get_current_thread();
	if( pCurT == pThread )
	{
		change_thread_state( NULL, pCurT, TS_WAIT );
		kernel_scheduler();
	}
	
	return( 0 );
}

// 프로세스를 종료한다.
int kill_process( ProcessStt *pProcess )
{	
	ThreadStt		*pT;
	ProcessStt	*pP;
	
	// sniper thread를 찾는다.
	pT = find_thread_by_alias( "init_sniper" );
	if( pT == NULL )
	{
		kdbg_printf( "kill_process: sniper thread not found!\n" );
		return( -1 );
	}
	
	// sniper쪽으로 kmesg를 날린다.
	ksend_kmesg( pT->dwID, KMESG_KILL_PROCESS, pProcess->dwID, 0 );

	// 자기 자신이면 리턴해서는 안된다.
	// (메시지를 보내는 순간 스나이퍼가 현재 프로세스를 죽일 수도...)
	pP = k_get_current_process();
	if( pP == pProcess )
	{
		//kdbg_printf( "kill_process(PID=%d): wait for death.\n", pP->dwID );
		change_thread_state( NULL, get_current_thread(), TS_WAIT );
		kernel_scheduler();
	}		

	return( 0 );
}

// ID로 쓰레드를 찾는다.
ThreadStt *find_thread_by_id( DWORD dwTID )
{
	ThreadStt *pT;

	for( pT = sch.pStartThread; pT != NULL; pT = pT->pNextSLink )
	{
		if( pT->dwID == dwTID )
			return( pT );
	}

	return( NULL );
}

// TSS로 쓰레드를 찾는다.
ThreadStt *find_thread_by_tss( TSSStt *pTSS )
{
	ThreadStt *pT;

	for( pT = sch.pStartThread; pT != NULL; pT = pT->pNextSLink )
	{
		if( pT->pTSS == pTSS )
			return( pT );
	}

	return( NULL );
}

// szAlias로 쓰레드를 찾는다.
ThreadStt *find_thread_by_alias( char *pAlias )
{
	ThreadStt *pT;

	for( pT = sch.pStartThread; pT != NULL; pT = pT->pNextSLink )
	{
		if( strcmpi( pT->szAlias, pAlias ) == 0 )
			return( pT );
	}

	return( NULL );
}

// find process with its thread id
ProcessStt *find_process_by_id( DWORD dwPID )
{
	ProcessStt *pP;

	// 스케쥴 링크에서 찾아 본다.
	for( pP = sch.pStartProcess; pP != NULL; pP = pP->pNextProcess )
	{
		if( pP->dwID == dwPID )
			return( pP );
	}

	// 좀비 링크에서 프로세스를 찾는다.
	for( pP = sch.pStartZombie; pP != NULL; pP = pP->pNextProcess )
	{
		if( pP->dwID == dwPID )
			return( pP );
	}

	return( NULL );
}

#define _TMP_STK_SIZE_FOR_FORK_	8192

int kernel_fork( THREAD_ENTRY_FUNC t_pThreadEntry, DWORD t_dwParam, DWORD t_dwStackSize, DWORD t_dwSchLevel, VConsoleStt *t_pVCon )
{
	static ThreadStt			*pT;
	static VConsoleStt			*pVCon;
	static THREAD_ENTRY_FUNC	pThreadEntry;
	static ProcessStt			*pP, *pParent;
	static BYTE					stack[_TMP_STK_SIZE_FOR_FORK_];
	static DWORD				dwOrgESP, dwOrgCR3, dwCR3, dwParam, dwStackSize, dwSchLevel;

	pThreadEntry	= t_pThreadEntry;
	dwParam			= t_dwParam;
	dwStackSize		= t_dwStackSize;
	dwSchLevel		= t_dwSchLevel;
	pVCon			= t_pVCon;

	_asm {
		PUSHFD
		CLI
		// 이전 스택을 보관하고 스택을 새로 설정한다.
		MOV dwOrgESP, ESP
		LEA ESP, stack
		ADD ESP, _TMP_STK_SIZE_FOR_FORK_ -4
	}

	pParent = k_get_current_process();

	// 프로세스와 쓰레드를 생성한다.
	pP = kcreate_process( pParent );
	if( pP == NULL )
		goto ERR;

	// pVCon이 NULL이면 Console이 새로 생성될 것이다.
	pP->pVConsole = pVCon;
	pP->pKExecveParam = (KExecveParamStt*)dwParam;

	// CR3 새로 설정한다.
	dwCR3 = get_process_page_dir( pP );
	set_cur_cr3_in_tss( dwCR3 );
	_asm {
		MOV EAX,		CR3
		MOV dwOrgCR3,	EAX
		MOV EAX,		dwCR3
		MOV CR3,		EAX
		FLUSH_TLB2(dwCR3)
	}

	// Owner Process, StackSize, Entry Function, Parameter
	pT = kcreate_thread( pP, dwStackSize, (DWORD)pThreadEntry, dwParam, dwSchLevel );
	if( pT == NULL )
		goto ERR;

	pP->pForegroundThread = pT;

	// CR3와 스택을 복원한다.
	set_cur_cr3_in_tss( dwOrgCR3 );
	_asm {
		MOV EAX, dwOrgCR3
		MOV CR3, EAX
		FLUSH_TLB2(dwOrgCR3)
		MOV ESP, dwOrgESP
		POPFD
	}
	//kdbg_printf( "kernel_fork: ok\n" );
	return( pP->dwID );

ERR:
	// CR3와 스택을 복원한다.
	set_cur_cr3_in_tss( dwCR3 );
	_asm {
		MOV EAX, dwOrgCR3
		MOV CR3, EAX
		FLUSH_TLB2(dwOrgCR3)
		MOV ESP, dwOrgESP
		POPFD
	}
	kdbg_printf( "kernel_fork: failed!\n" );
	return( -1 );
}

// 쓰레드를 생성한 후 현재 Context를 그대로 복제한다.
// 스택 등 사용자 영역의 주소공간은 RDONLY로 공유한다.
static ThreadStt *duplicate_thread( ProcessStt *pNewProc, ThreadStt *pThisThread, TSSStt *pTSS )
{
	ThreadStt		*pT;
	TStackLinkStt	*pStk;

	// 스택 없이 쓰레드를 할당한다.
	pT = kcreate_thread( pNewProc, KTHREAD_NO_STACK, 0, 0, TS_READY_NORMAL );
	//pT = kcreate_thread( pNewProc, KTHREAD_NO_STACK, 0, 0, TS_WAIT );

	// 기존 쓰레드에서 필요한 정보를 복사한다.
	pT->dwMappingFlag  = pThisThread->dwMappingFlag;
	pT->dwR3EntryFunc  = pThisThread->dwR3EntryFunc;
	pT->dwNice	 	   = pThisThread->dwNice;
	pT->dwCurNice	   = pThisThread->dwCurNice;

	// 스택 링크를 새로 할당한 후 복제한다.
	pStk = (TStackLinkStt*)kmalloc( sizeof( TStackLinkStt ) );
	if( pStk == NULL )
	{
		kdbg_printf( "duplicate_thread: kmalloc failed!\n" );
	}
	memcpy( pStk, pThisThread->pStack, sizeof( TStackLinkStt ) );
	pStk->pPre = pStk->pNext = NULL;
	pStk->pThread = pT;
	pT->pStack    = pStk;

	// 스택 링크를 프로세스에 추가한다.
	append_Stack_link( pNewProc, pStk );

	// Current CR3가 그대로 설정된다. (명시적으로 CR3를 설정해 주어야 한다.)
	vMakeTSS( pT->pTSS, pTSS->dwEIP, pTSS->dwESP );
    pT->pTSS->dwESI    = pTSS->dwESI;
    pT->pTSS->dwEDI    = pTSS->dwEDI;
    pT->pTSS->dwEBP    = pTSS->dwEBP;
	pT->pTSS->dwEFLAG  = pTSS->dwEFLAG;
	pT->pTSS->dwCR3    = pTSS->dwCR3;
    pT->pTSS->dwEFLAG |= MASK_IF;		// Interrupt Enable

	return( pT );
}

// 프로세스를 생성하고 현재 쓰레드를 복제한다.
static int internal_kfork( DWORD _dwESP3 )
{
	static ProcessStt	*pP;
	static TSSStt		t_tss;
	static DWORD		dwOrgCR3, dwESP3;
	static ThreadStt	*pCurThread, *pT;

	int				nPID;
	ProcessStt		*pCurProcess;
	DWORD			dwCR3, dwESI, dwEDI, dwEIP, dwESP, dwEBP, dwR0StackTop, dwR3StackTop;

	dwESP3 = _dwESP3;

	pCurThread = get_current_thread();
	if( pCurThread == NULL )
	{
		kdbg_printf( "internal_kfork: pCurThread = NULL!\n" );
		return( -1 );
	}
	pCurProcess = pCurThread->pProcess;

	// 새로 생성할 쓰레드의 컨텍스트를 설정한다.
	_asm {
		LEA EAX,   NEW_ENTRY
		MOV dwEIP, EAX
		MOV dwESI, ESI
		MOV dwEDI, EDI
		MOV dwEBP, EBP
		MOV dwESP, ESP
	}
	dwR0StackTop = pCurThread->pStack->dwR0StackTop;
	dwR3StackTop = pCurThread->pStack->dwR3StackTop;

	//kdbg_printf( "T(%d) R0(0x%X), R3(0x%X)\n", 
	//	pCurThread->dwID, pCurThread->pStack->dwR0StackTop, pCurThread->pStack->dwR3StackTop );

	// Current CR3가 그대로 설정된다.
	// duplicate_thread 내에 들어가서 따로 설정된다.
	// ESI, ESI, EDI, EBP, ESp를 설정하기 위한 Temporary TSS
	memcpy( &t_tss, pCurThread->pTSS, sizeof( TSSStt ) );
	t_tss.dwEIP    = dwEIP;
	t_tss.dwESP    = dwESP;
	t_tss.dwESI    = dwESI;
	t_tss.dwEDI    = dwEDI;
	t_tss.dwEBP    = dwEBP;
	t_tss.dwEFLAG |= MASK_IF;		// Interrupt Enable

	if( 0 )
	{
NEW_ENTRY:
		//kdbg_printf( "New Thread!!!\n" );
		//_asm int 1
		return( 0 );
	}
	else
	{
		//_asm int 1
	}

	// 새로운 프로세스를 생성한다.
	pP = kcreate_process( pCurThread->pProcess );
	if( pP == NULL )
	{
		kdbg_printf( "internal_kfork: kcreate_process error!\n" );
		return( -1 );
	}

	// Parent Process의 것을 그대로 복제하는 데이터들.
	pP->pMyDbg	      	= pCurProcess->pMyDbg;
	pP->pModule         = pCurProcess->pModule;
	pP->pVConsole       = pCurProcess->pVConsole;
	pP->dwNextStackBase = pCurProcess->dwNextStackBase;

	// R3 export Function 테이블을 복사한다.
	memcpy( &pP->e, &pCurProcess->e, sizeof( R3ExportTblStt ) );

	// 사용자 영역 페이지 테이블을 복제한다.
	dup_page_dir( pP, pCurProcess, 512 ); // 512 = 복제를 시작할 페이지 디렉토리 인덱스.
			 
	// Ring0 스택을 R->RW로 복제한다. (dwESP부터 StackTop을 포함하는 페이지)
	dwESI = dwESP & (DWORD)0xFFFFF000;
	dwEDI = dwR0StackTop & (DWORD)0xFFFFF000;
	//kdbg_printf( "Duplicate Ring0 Stack: 0x%X - 0x%X (R0StackTop=0x%X)\n", dwESI, dwEDI, dwR0StackTop );
	for( ; dwESI <= dwEDI; dwESI += 0x1000 )
		copy_on_write( (DWORD*)get_process_page_dir( pP ), dwESI );

	// Ring3 스택을 R->RW로 복제한다.
	dwESI = (dwESP3 - 4096) & (DWORD)0xFFFFF000;
	dwEDI = dwR3StackTop & (DWORD)0xFFFFF000;
	//kdbg_printf( "Duplicate Ring3 Stack: 0x%X - 0x%X (R3StackTop=0x%X)\n", dwESI, dwEDI, dwR3StackTop );
	for( ; dwESI <= dwEDI; dwESI += 0x1000 )
		copy_on_write( (DWORD*)get_process_page_dir( pP ), dwESI );

	nPID = (int)pP->dwID;

	// CR3를 새로 설정한다.
	dwCR3 = t_tss.dwCR3 = get_process_page_dir( pP );
	///////////////////////////////////////////////////////////////////////////////
	set_cur_cr3_in_tss( dwCR3 );
	_asm {
		MOV EAX,		CR3
		MOV dwOrgCR3,	EAX
		MOV EAX,		dwCR3
		MOV CR3,		EAX
		FLUSH_TLB2(dwCR3);		// 2003-09-03
	}

	// 쓰레드를 생성하여 복제한다.
	pT = duplicate_thread( pP, pCurThread, &t_tss );

	// CR3를 복원한다.
	set_cur_cr3_in_tss( dwOrgCR3 );
	_asm {
		MOV EAX, dwOrgCR3
		MOV CR3, EAX
		FLUSH_TLB2(dwOrgCR3);
	}
	///////////////////////////////////////////////////////////////////////////////

	if( pT == NULL )
	{	// 실패했으므로 프로세스 구조체를 해제한다.
		nPID = -1;
		krelease_process_struct( pP );
		kdbg_printf( "internal_kfork: critical error!\n" );
		return( -1 );
	}

	// 복제한 쓰레드를 FG로 설정한다.
	pP->pForegroundThread = pT;

	//kdbg_printf( "internal_kfork: end\n" );

	return( nPID );
}

// 재진입을 막아야 하며 Serialize되어야 한다.
int kfork( DWORD dwESP3 )
{
	int nPID;
	nPID = internal_kfork( dwESP3 );
	return( nPID );
}

// 하나의 프로세스를 자세하게 출력한다.
int disp_process( DWORD dwPID )
{
	ProcessStt *pP;
	int			nForkRefPID;
	DWORD		dwVConID, dwFG_TID;

	pP = find_process_by_id( dwPID );
	if( pP == NULL )
	{
		kdbg_printf( "Process %d not found!\n", dwPID );
		return( -1 );
	}

	if( pP->pAddrSpace != NULL && pP->pAddrSpace->pForkRef != NULL )
		nForkRefPID = pP->pAddrSpace->pForkRef->nOwnerPID;
	else
		nForkRefPID = 0;

	if( pP->pVConsole != NULL )
	    dwVConID = pP->pVConsole->nID;
	else
		dwVConID = 0;

	if( pP->pForegroundThread != NULL )
		dwFG_TID = pP->pForegroundThread->dwID;
	else dwFG_TID = 0;

	kdbg_printf( "PROCESS(%d): 0x%X\n", pP->dwID, (DWORD)pP );
	kdbg_printf( "PD:0x%X, MyDbg;0x%X, Module:0x%X\n", get_process_page_dir( pP ), (DWORD)pP->pMyDbg, (DWORD)pP->pModule );
	kdbg_printf( "ParentID:%d, VCon:%d, Threads:%d\n", (DWORD)pP->dwParentID, dwVConID, pP->nTotalThread );
	kdbg_printf( "NextStackBase:0x%X, (0x%X-0x%X)\n", pP->dwNextStackBase, (DWORD)pP->pStartStk, (DWORD)pP->pEndStk );
	kdbg_printf( "ForkRefPID:%d, ForkRefCount:%d\n", nForkRefPID, pP->pAddrSpace->nForkRefCounter );
	kdbg_printf( "FG-Thread:%d, StartThread:0x%X, EndThread:0x%X\n", dwFG_TID, (DWORD)pP->pStartThread, (DWORD)pP->pEndThread );
	kdbg_printf( "PreProcess:0x%X, NextProcess:0x%X\n", pP->pPreProcess, pP->pNextProcess );

	return( 0 );
}

// 하나의 쓰레드를 자세하게 출력한다.
int disp_thread( DWORD dwTID )
{
	ThreadStt	*pT;

	pT = find_thread_by_id( dwTID );
	if( pT == NULL )
	{							
		kdbg_printf( "Thread %d not found!\n", dwTID );
		return( -1 );
	}

	kdbg_printf( "THREAD (%d) %s : 0x%X\n",
		pT->dwID, pT->szAlias, (DWORD)pT );
	kdbg_printf( " pQ:0x%X, TSS:0x%X, kbd-q:0x%X, process:0x%X\n",
		pT->pQ, pT->pTSS, pT->pKbdQ, pT->pProcess );
	kdbg_printf( " nState:0x%X, nWaitResult:0x%X, dwMappingFlag:0x%X, dwR3EntryFunc:0x%X\n",
		pT->nState, pT->nWaitResult, pT->dwMappingFlag, pT->dwR3EntryFunc );
	kdbg_printf( " nice:0x%X, cur:0x%X\n",
		pT->dwNice, pT->dwCurNice );
	kdbg_printf( " link Q(0x%X,0x%X),S(0x%X,0x%X),P(0x%X,0x%X)\n",
		pT->pPreQLink,  pT->pNextQLink, pT->pPreSLink,  pT->pNextSLink, pT->pPrePLink,  pT->pNextPLink );
	kdbg_printf( " stack(0x%X): r0_top:0x%X, r3_top:0x%X, dwSize:0x%X(%dk)\n",
		pT->pStack, pT->pStack->dwR0StackTop, pT->pStack->dwR3StackTop, pT->pStack->dwSize, pT->pStack->dwSize/1024 );
	kdbg_printf( " wait-obj: (Total:%d) Link[ 0x%X-0x%X]\n",
		pT->nTotalWaitObj, pT->pStartWaitObj, pT->pEndWaitObj );

	return( 0 );
}

// 스케쥴 큐의 내용을 출력한다.
void display_schedule_q()
{
	ThreadStt	*pT;
	SQStt		*pQ;

	pQ = &sch.q[TS_READY_NORMAL];
	kdbg_printf( "READY_NORMAL: " );
	for( pT = pQ->pStartThread; pT != NULL; pT = pT->pNextQLink )
		kdbg_printf( "%d ", pT->dwID );
	kdbg_printf( "\n" );

	pQ = &sch.q[TS_WAIT];
	kdbg_printf( "WAIT: " );
	for( pT = pQ->pStartThread; pT != NULL; pT = pT->pNextQLink )
		kdbg_printf( "%d ", pT->dwID );

	kdbg_printf( "\n" );
}

// 좀비 프로세스의 리턴 값을 얻어 온 후 프로세스 구조체를 해제한다.
static int get_zombie_exit_code( int nPID )
{
	ProcessStt	*pP;
	int			nExitCode;

	pP = find_process_by_id( nPID );
	if( pP == NULL )
	{
		kdbg_printf( "get_zombie_exit_code: PID(%d) not found!\n", nPID );
		return( -1 );
	}

	// 모든 쓰레드가 종료된 것인지 확인한다.
	if( pP->dwState != PSTATE_ZOMBIE )
	{	// 쓰레드가 완전히 종료되지 않았다.
		kdbg_printf( "get_zombie_exit_code: process is not zombie.\n" );
		_asm int 1
	}

	// 프로세스를 좀비 링크에서 분리한다.
	unlink_from_zombie( pP );
	nExitCode = pP->nExitCode;

	// 프로세스 구조체를 해제한다.
	kfree( pP );

	return( nExitCode );
}

// Child Process가 종료될 때까지 대기한다.
int kwait( int *pExitCode )
{
	ThreadStt	*pT;
	int			nType, nPID, nExitCode;

	pT = get_current_thread();
	if( pT == NULL )
		return( -1 );

	// Child Process가 종료될 때까지 대기한다. (Parameter를 받지 않는다.)
	nType = kwait_kmesg( KMESG_CHILD_PROCESS_EXIT, &nPID, NULL, 0 );
	if( nType < 0 )
	{
		kdbg_printf( "kwait: error!\n" );
		return( -1 );
	}

	// 리턴 값을 알아낸다.
	nExitCode = get_zombie_exit_code( nPID );
	if( pExitCode != NULL )
		pExitCode[0] = nExitCode;

	return( nPID );
}

// 특정 Child Process가 종료될 때까지 대기한다.
int kwaitpid( int nPID, int *pExitCode )
{
	ThreadStt	*pT;
	int			nR, nResultPID, nExitCode;

	pT = get_current_thread();
	if( pT == NULL )
		return( -1 );

	// Child Process가 종료될 때까지 대기한다.
	for( ;; )
	{	// 여러 개의 Child Process 가운데 종료된 놈의 ID가 nResultPID에 전달된다.
		nR = kwait_kmesg( KMESG_CHILD_PROCESS_EXIT, &nResultPID, NULL, 0 );
		if( nResultPID == nPID )
			break;
	}

	// 리턴 값을 알아낸다.
	nExitCode = get_zombie_exit_code( nResultPID );
	if( pExitCode != NULL )
		pExitCode[0] = nExitCode;

	return( nResultPID );
}

// 특정 Thread가 종료될 때까지 대기한다.
int kwaittid( int nTID, int *pExitCode )
{
	ThreadStt	*pCurT, *pT;
	int 		nR, nResultTID;

	pCurT = get_current_thread();
	if( pCurT == NULL )
		return( -1 );

	pT = find_thread_by_id( nTID );
	if( pT == NULL )
	{	// 종료를 대기하려는 Thread를 찾을 수 없다.
		kdbg_printf( "kwaittid: TID(%d) not found!\n", nTID );
		return( -1 );
	}

	if( pT->dwKillerTID != pCurT->dwID )
	{	// 종료를 대기하려는 Thread의 dwKillerTID가 현재 Thread의 ID와 다르다.  (기다려봐야 소용이 없다.)
		kdbg_printf( "kwaittid: TID(%d).dwKillerTID(%d) != current TID(%d)\n", 
			nTID, pT->dwKillerTID, pCurT->dwID );
			
		return( -1 );
	}	

	// Thread가 종료될 때까지 대기한다.
	for( ;; )
	{	// 종료된 Thread의 ID가 nResultTID에 전달된다.
		nR = kwait_kmesg( KMESG_CHILD_THREAD_EXIT, &nResultTID, NULL, 0 );
		if( nResultTID == nTID )
			break;
	}

	if( pExitCode != NULL )
		pExitCode[0] = 0;

	return( nResultTID );
}

static int free_kexecve_param( KExecveParamStt *pEP )
{
	kfree( pEP );
	return( 0 );
}

// Application을 로드하여 실행한다.
static DWORD kexecve_thread_entry( DWORD dwParam )
{
	KExecveParamStt	*pEP;

	pEP = (KExecveParamStt*)dwParam;

	launch_r3_program( pEP );

	// 여기까지 오지 않을 것이다.
	for( ;; )
		kernel_scheduler();

	return( 0 );
}

// 새 프로그램 로드에 필요한 파러메터를 설정한다.
KExecveParamStt *make_kexecve_param( char *pFile, char **ppArgv, char **ppEnv )
{
	KExecveParamStt 	*pP;
	char				*pNULL, *pFileName, *pStrBuff;
	int				nArgCounter, nEnvCounter, nSttSize;
	int 				nI, nJ, nK, nFileSize, nArgSize, nEnvSize;

	if( pFile == NULL || pFile[0] == 0 )
		return( NULL );

	pNULL = NULL;
	if( ppArgv == NULL )
		ppArgv = &pNULL;
	if( ppEnv == NULL )
		ppEnv = &pNULL;

	// 각각의 개수, 길이를 구한다.
	nFileSize = strlen( pFile ) + 1;

	nArgSize = 0;
	// argc[0] = pFileName해야 하므로 1부터 시작한다.
	for( nArgCounter = 0; ppArgv[nArgCounter] != NULL; nArgCounter++ )
		nArgSize += strlen( ppArgv[nArgCounter] ) + 1;
	nArgCounter += 2;

	nEnvSize = 0;
	for( nEnvCounter = 0; ppEnv[nEnvCounter] != NULL; nEnvCounter++ )
		nEnvSize += strlen( ppEnv[nEnvCounter] ) + 1;
	nEnvCounter++;

	nSttSize = sizeof( KExecveParamStt ) + nFileSize + nArgSize + nEnvSize +
	           ( nArgCounter*4 ) + ( nEnvCounter*4 );

	pP = (KExecveParamStt*)kmalloc( nSttSize );
	if( pP == NULL )
		return( NULL );

	pP->nSize  = nSttSize;
	pP->nArgc  = nArgCounter-1;
	pP->ppArgv = (char**)( (DWORD)pP + sizeof( KExecveParamStt ) );
	pP->ppEnv  = (char**)( (DWORD)pP->ppArgv + nArgCounter * 4 );
	pFileName  = pStrBuff = (char*)( (DWORD)pP->ppEnv +  nEnvCounter * 4 );

	// FileName을 복사한다.
	strcpy( pFileName, pFile );
	nI = nFileSize;

	// Argv를 복사한다.
	pP->ppArgv[0] = pFileName;			// argv[0]에 파일 패스를 지정한다.
	for( nK = 1, nJ = 0; ; nJ++, nK++ )
	{
		if( ppArgv[nJ] == NULL )		// argv[1] 부터 parameter를 저장한다.
		{
			pP->ppArgv[nK] = NULL;
			break;
		}

		pP->ppArgv[nK] = &pStrBuff[nI];
		strcpy( &pStrBuff[nI], ppArgv[nJ] );
		nI += strlen( ppArgv[nJ] ) +1;
	}

	// Env를 복사한다.
	for( nK = 0; ; nK++ )
	{
		if( ppEnv[nK] == NULL )
		{
			pP->ppEnv[nK] = NULL;
			break;
		}
		pP->ppEnv[nK] = &pStrBuff[nI];
		strcpy( &pStrBuff[nI], ppEnv[nK] );
		nI += strlen( ppEnv[nK] ) +1;
	}

	// nI가 nSttSize와 같아야 한다.
	nI += sizeof( KExecveParamStt ) + (int)( (DWORD)pStrBuff - (DWORD)pP->ppArgv );

	// 구조체 + 버퍼 사이즈를 저장한다.
	//kdbg_printf( "KExecveParamStt size = %d\n", nSttSize );

	return( pP );
}

static int is_whitespace( char *pS )
{
     if( pS[0] == ' ' || pS[0] == 9 || pS[0] == 13 || pS[0] == 10 )
         return( 1 );
     else
         return( 0 );
}

char *skip_space( char *pS )
{
    for( ; pS[0] != 0; pS++ )
    {
        if( is_whitespace( pS ) == 0 )
			break;
    }
    return( pS );
}

char *search_space( char *pS )
{
	for( ; pS[0] != 0; pS++ )
	{
		if( is_whitespace( pS ) )
			break;
	}
	return( pS );
}

static char *get_next_word( char *pWord, char *pS )
{
    int nI;

    pS = skip_space( pS );

    // 인용 부호로 쌓인 경우.
    if( pS[0] == '"' )
    {
        pS++;
        pWord[0] = 0;
        for( nI = 0; ; nI++ )
        {
            if( pS[nI] == '"' )
                return( &pS[nI+1] );

            pWord[nI] = pS[nI];
            if( pS[nI] == 0 )
                return( &pS[nI] );
            pWord[nI+1] = 0;
        }
    }


    for( nI = 0; ; )
    {
        pWord[nI] = pS[nI];
        if( pS[nI] == 0 )
            break;

        nI++;
        pWord[nI] = 0;
        if( is_whitespace( &pS[nI] ) != 0 )
            break;
    }

    return( &pS[nI] );
}


static int shrink_buffer( char *pS, int *pBuffSize )
{
	int		nI, nK, nTotal;
	char	szWord[260], *pNext;

	if( pS == NULL )
	{
		pBuffSize[0] = 1;
		return( 0 );
	}

	pNext = pS;
	for( nTotal = nI = 0;; )
	{
		pNext = get_next_word( szWord, pNext );
		nK = strlen( szWord );
		if( nK == 0 )
		{
			pS[nI] = 0;
			nI++;
			break;
		}

		strcpy( &pS[nI], szWord );
		nI += nK +1;
		nTotal++;
		if( (DWORD)pNext < (DWORD)&pS[nI] )
			pNext = &pS[nI];

	}
	if( nI == 1 )
	{
		pS[nI] = 0;
		nI++;
	}

	pBuffSize[0] = nI;

	return( nTotal );
}

// 새 프로그램 로드에 필요한 파러메터를 설정한다.
static KExecveParamStt *make_kexecve_param_ex( char *pFile, char *pArg, char *pEnv )
{
	KExecveParamStt 	*pP;
	int 				nI, nK, nJ, nT;
	char				*pStrBuff, *pNext;
	int					nArgCounter, nEnvCounter, nArgSize, nEnvSize, nFileSize, nSttSize;

	if( pFile == NULL || pFile[0] == 0 )
		return( NULL );

	// 버퍼의 크기를 구한다.
	nFileSize   = strlen( pFile ) + 1;
	nArgCounter = shrink_buffer( pArg, &nArgSize );
	nEnvCounter = shrink_buffer( pEnv, &nEnvSize );
	nSttSize = sizeof( KExecveParamStt ) + nFileSize +
		       nArgSize + nEnvSize + (nEnvCounter+nArgCounter+2)*4;

	pP = (KExecveParamStt*)kmalloc( nSttSize );
	if( pP == NULL )
		return( NULL );

	pP->nSize  = nSttSize;
	pP->nArgc  = nArgCounter;
	pP->ppArgv = (char**)( (DWORD)pP + sizeof( KExecveParamStt ) );
	pP->ppEnv  = (char**)( (DWORD)pP->ppArgv + (nArgCounter+1)*4 );
	pStrBuff   = (char*) ( (DWORD)pP->ppEnv  + (nEnvCounter+1)*4 );

	strcpy( pStrBuff, pFile );
	nI = nFileSize;

	// Arg 포인터를 설정하고 스트링을 복사한다.
	pP->ppArgv[0] = pStrBuff;
	pNext = pArg;
	for( nJ = 1, nK = 0; ; nK++ )
	{
		if( nK == nArgCounter )
		{
			pP->ppArgv[nJ] = NULL;
			pStrBuff[nI++] = 0;
			break;
		}

		strcpy( &pStrBuff[nI], pNext );
		nT = strlen( pNext ) + 1;

		pP->ppArgv[nJ++] = &pStrBuff[nI];

		nI += nT;
		pNext += nT;
	}

	pNext = pEnv;
	for( nJ = 0; ; )
	{
		if( nJ == nEnvCounter )
		{
			pP->ppEnv[nJ] = NULL;
			pStrBuff[nI++] = 0;
			break;
		}

		strcpy( &pStrBuff[nI], pNext );
		nT = strlen( pNext ) + 1;

		pP->ppEnv[nJ++] = &pStrBuff[nI];

		nI += nT;
		pNext += nT;
	}

	return( pP  );
}

// 프로세스의 주소공간에 새로운 프로그램을 로드한다.
int kexecve( char *t_pFile, char *t_argv[], char *t_envp[] )
{
	DWORD				dwT;
	KExecveParamStt		*pEP;
	int					nNewPID;
	ProcessStt			*pP, *pCurP;

	// 메모리 할당이 일어난다.
	pEP = make_kexecve_param( t_pFile, t_argv, t_envp );

	// wait 상태로 프로세스와 쓰레드를 생성한다.
	nNewPID = kernel_fork( kexecve_thread_entry, (DWORD)pEP, (128*1024), TS_WAIT, get_current_vconsole() );
	if( nNewPID < 0 )
		return( -1 );

	// 생성된 프로세스의 주소를 구한다.
	pP = find_process_by_id( nNewPID );
	if( pP == NULL )
	{
		kdbg_printf( "kexecve: find_process_by_id( %d ) = NULL!\n", nNewPID );
		return( -1 );
	}

	pCurP = k_get_current_process();

	// 현재 프로세스와 신규 프로세스에서 교환해야 할 데이터는 교환한다.
	dwT					= pP->dwID;
	pP->dwID			= pCurP->dwID;;
	pP->dwParentID		= pCurP->dwParentID;;
	pP->pVConsole		= pCurP->pVConsole;
	pP->pKExecveParam	= pEP;				// kexecve param을 설정한다.

	// 여기서 VConsole을 NULL로 만들어 버리면 문자열 출력이 안된다. !!!
	//pCurP->pVConsole  = NULL;

	pCurP->dwID       = dwT;	// 프로세스 ID를 교환한다.
	pCurP->dwParentID = 0;		// parent ID는 없는 것으로 한다.

	// 쓰레드의 상태를 변경한다.
	change_thread_state( NULL, pP->pStartThread, TS_READY_NORMAL );

	// 현재 프로세스는 죽는다.  
	//kdbg_printf( "kexecve: kill_process( %d )\n", pCurP->dwID );
	kill_process( pCurP );

	return( 0 );
}

// 새로운 가상 콘솔을 생성하여 login.exe를 실행한다.
DWORD r0_fork_thread( DWORD dwParam )
{
	ProcessStt		*pP;
	KExecveParamStt	*pEP;

	pEP = (KExecveParamStt*)dwParam;

	// vconsole을 생성한다.
	pP = k_get_current_process();
	if( pP->pVConsole == NULL )
		pP->pVConsole = make_vconsole();
	else
	{
		//kdbg_printf( "r0_session: pP->pVConsole is not NULL(ID=%d) !\n",
		//	pP->pVConsole->nID );
	}

	// login.exe이 실행되고 있는 콘솔을 Active로 설정한다.
	//set_active_vconsole( pP->pVConsole );

	// 현재 프로세스를 FG로 설정한다.
	set_fg_process( NULL, pP );

	launch_r3_program( pEP );

	// 여기까지 오지 않을 것이다.
	for( ;; )
		kernel_scheduler();

	return( 0 );
}

int ksuspend_thread( ThreadStt *pT )
{
	int	nR;
	
	if( pT == NULL )
		return( -1 );

	if( pT->nState >= TS_READY_LAZY )
	{
		kdbg_printf( "ksuspend_thread: thread(%d) is not ready state! (%d)\n", pT->dwID, pT->nState );
		return( -1 );	// 현재 실행중인 상태가 아니다.
	}

	pT->nPrevThreadState = pT->nState;
	nR = change_thread_state( NULL, pT, TS_SUSPEND );
	kernel_scheduler();

	return( nR );
}

int kresume_thread( ThreadStt *pT )
{
	int	nR;

	if( pT == NULL )
		return( -1 );

	if( pT->nState != TS_SUSPEND )
	{	// SUSPEND가 아니라 이미 READY일 수도...
		//kdbg_printf( "ksuspend_thread: thread(%d) is not suspend state!\n", pT->dwID );
		return( -1 );	// 현재 실행중인 상태가 아니다.
	}

	nR = change_thread_state( NULL, pT, pT->nPrevThreadState );

	return( nR );
}


