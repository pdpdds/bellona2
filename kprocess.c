#include "bellona2.h"

SchStt	sch;

// �������� Alias�� �����Ѵ�.
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

// ���μ����� Alias�� �����Ѵ�.
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

// ���μ��� ID�� �����Ѵ�.
static DWORD dwMakeProcessThreadID( DWORD dwAddr )
{
	return( ++bell.dwNextProcessThreadID );
}

// ���μ����� ������ ��ũ�� �߰��Ѵ�.
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

// ���μ����� ������ ��ũ���� �����Ѵ�.
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

// �����带 ���� ���� ������ ť�� �߰��Ѵ�.
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

// �����带 ������ ť���� �����Ѵ�.
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

// ���� ��ũ�� ���μ��� ����ü�� �����Ѵ�.
static int append_Stack_link( ProcessStt *pProc, TStackLinkStt *pStk )
{
	// ���� ��ũ�� �ϳ��� ������.
	if( pProc->pStartStk == NULL )
	{
		 pProc->pStartStk = pProc->pEndStk = pStk;
		 pStk->pPre = pStk->pNext = NULL;
		 return(0);
	}

	// ��ũ�� ���� �߰��Ѵ�.
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

// Thread�� �������� ���� ������ Process�κ��� ������ �д�.
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
	{	// FREE�� ������ ã�´�.  (������ üũ 2003-03-01)
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

	// ���ñ���ü�� �ʱ�ȭ �Ѵ�.
	pStk = (TStackLinkStt*)kmalloc( sizeof( TStackLinkStt ) );
	memset( pStk, 0, sizeof( TStackLinkStt ) );

	// ���� ���ε� ���̸� ������ ���μ��� ��ũ�� �߰��Ѵ�.
	append_Stack_link( pProc, pStk );

	pStk->dwBaseAddr  = pProc->dwNextStackBase;
	pStk->dwSize       = dwSize;
	dwESP              = (DWORD)pStk->dwBaseAddr + pStk->dwSize - 4;	// stack bottom
	pStk->dwR0StackTop = dwESP;
	pStk->dwR3StackTop = dwESP - (dwSize/2);

	// ���μ����� ���� ž�� ���� ��Ų��.  (������ �Ҵ��� ���� ��ġ)
	pProc->dwNextStackBase += dwSize;

MAKE_STACK_LINK:
	dwESP = pStk->dwR0StackTop;

	// ������ OWner�� �����Ѵ�.
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

// �����带 Ư�� 'Q'�� �߰��Ѵ�.
int nPushThread( SQStt *pQ, ThreadStt *pThread )
{
	// Thread���� �ִ� 'Q'�����͸� �����Ѵ�.
	pThread->pQ = pQ;

	if( pQ->nTotal == 0 )
	{	// ����� Thread�� �ϳ��� ����.
		pQ->pStartThread    = pQ->pEndThread       = pThread;
		pThread->pPreQLink  = pThread->pNextQLink  = NULL;
	}
	else
	{	// 'Q'�� ���� ���� �����Ѵ�.
		pThread->pPreQLink			= pQ->pEndThread;
		pThread->pNextQLink			= NULL;
		pQ->pEndThread->pNextQLink	= pThread;
		pQ->pEndThread				= pThread;
	}

	pQ->nTotal++;

	return( 0 );
}

// �����带 ť���� �����Ѵ�.
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

// ��ȿ�� ���������� Ȯ���Ѵ�.
int is_thread( ThreadStt *pThread )
{
	if( pThread == NULL || memcmp( pThread->szMagicStr, THREAD_MAGIC_STR, strlen( THREAD_MAGIC_STR ) ) != 0 )
		return( 0 );	// �߸��� ������.
	else
		return( 1 );
}

// ��ȿ�� ���μ������� Ȯ���Ѵ�.
int is_process( ProcessStt *pProcess )
{
	if( pProcess == NULL || memcmp( pProcess->szMagicStr, PROC_MAGIC_STR, strlen( PROC_MAGIC_STR ) ) != 0 )
		return( 0 );	// �߸��� ���μ���.
	else
		return( 1 );
}

// ������ ���¸� �����Ѵ�.
// ���� ť���� ���ŵǾ� ���ο� ������ ť�� ����.
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

    // �����尡 ��ũ�� ���ο� ť
	if( nState < MAX_SCH_Q )
	{
		if( pSch != NULL )
			pNewQ = &pSch->q[nState];
		else
			pNewQ = &sch.q[nState];
	}
	else
		pNewQ = NULL;

	// OLD ť���� �����带 ������.
	nR = nPopThread( pThread );
	if( nR == -1 )
		goto BACK;

	// ���ο� ť�� �����带 �ִ´�.
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

// Thread�� ó�� �����Ǿ��� ��� ��� ���۵Ǵ� �Լ�
_declspec(naked) void kthread_entry_point()
{	// EAX�� ������ Call�� �Լ��� �ִ�.
	static DWORD				dwR, dwFunc, dwParam;
	static THREAD_ENTRY_FUNC	pFn;

	_asm {
		MOV dwFunc,  EAX;	// CALL�� �Լ��� �ּҴ� EAX��
		MOV dwParam, EBX;	// �Լ��� ������ Parameter�� EBX�� ���޵ȴ�.
		PUSHAD;
		PUSHFD
		CLD
		STI					// 2003-08-16  �̰� �����ָ� ���ο� �����尡 CLI�� ���·� ����ȴ�.
	}

	pFn = (THREAD_ENTRY_FUNC)dwFunc;
	dwR = pFn( dwParam );	// ���� �Լ��� ȣ���Ѵ�.

	_asm {
		POPFD
		POPAD;
		MOV EAX, dwR;		// ������� EAX�� ����ȴ�.
		IRETD;				// TASK Switching�� ���� ��� �Ѿ���� ���̹Ƿ� IRETD�� �ش�.
	}
}

//////////////////////////////////////////////////////////////////
// �����췯�� ��ϵ� ���μ����� ������ ����Ѵ�.
void disp_process_list()
{
	int			nI, nVConID;
	char		ch;
	SchStt		*pSch;
	DWORD		dwPID;
	ProcessStt	*pP, *pCurProc;

	pSch = &sch;
	if( pSch->nTotalProcess == 0 )
	{	// ���μ����� �ϳ��� ����.
		kdbg_printf( "no process.\n" );
		return;
	}

	pP = pSch->pStartProcess;
	pCurProc = k_get_current_process();

	kdbg_printf( "          Addr     ID   PID  Threads   PageDir  VCon  Alias\n" );
	for( nI = 0; pP != NULL; pP = pP->pNextProcess, nI++ )
	{
		// Parent ���̵� ���Ѵ�.
		dwPID = pP->dwParentID;

		// ���� ���μ����� '*'�� �߰��Ѵ�.
		if( pP == pCurProc )
			ch = '*';
		else
			ch = ' ';

		// ���� �ܼ� ���̵� ���Ѵ�.
		if( pP->pVConsole == NULL )
			nVConID = 0;
		else
			nVConID = pP->pVConsole->nID;

		// ���μ��� ������ ����Ѵ�.
		kdbg_printf( "[%2d] %c0x%08X  %3d   %3d   %3d    0x%08X  %3d  %s\n",
            nI, ch, (DWORD)pP, pP->dwID, dwPID, pP->nTotalThread, get_process_page_dir( pP ),
			nVConID, pP->szAlias );
	}

	// Zombie Process�� ��� �Ѵ�. (2004-03-26)
	for( pP = pSch->pStartZombie; pP != NULL; pP = pP->pNextProcess )
	{
		// ���μ��� ������ ����Ѵ�.
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

// Thread�� ���� ��Ʈ���� ��´�.
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


// �����췯�� ��ϵ� Thread�� ������ ����Ѵ�.
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

		// wait ������ ��쿡�� Wait ��� ���ڿ��� ���Ѵ�.
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

	// ���μ����� ���� �ֿܼ� FG�� �����Ѵ�.
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
		return( 1 );		//�̹� ������ ����.
		
	pT->pProcess->pForegroundThread = pT;
	
	// ������.
	return( 0 );
}

// �����带 FG�� �����ϰ� Parent Process�� FG�� �����Ѵ�.
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

// ���� ���μ����� FG ���μ����� ���Ѵ�.
ProcessStt *get_fg_process()
{
	ProcessStt *pP;

	pP = k_get_current_process();
	if( pP == NULL || pP->pVConsole == NULL )
		return( NULL );

	return( pP->pVConsole->pStartFg );
}

// Active Console�� FG ���μ����� ���Ѵ�.
ProcessStt *get_sys_fg_process()
{
	VConsoleStt *pVC;

	pVC = get_active_vconsole();
	if( pVC == NULL )
		return( NULL );

	return( pVC->pStartFg );
}

// process���� fg thread�� ���Ѵ�.
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

// �����带 ���μ����� ��ũ�� �߰��Ѵ�.
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

// �����带 �����Ѵ�.
ThreadStt *kcreate_thread( ProcessStt *pProc, DWORD dwStackSize, DWORD dwFunc,
						  DWORD dwParam, int nState )
{
	ThreadStt	*pThread;
	DWORD		*pPD;
	DWORD		dwESP;
	char		szT[128];

	if( pProc == NULL )
	{	// ���μ����� �����Ǿ� ���� ������ ���� ���μ��� �Ʒ��� �����Ѵ�.
		pProc = k_get_current_process();
		if( pProc == NULL )
		{
		    kdbg_printf( "kcreate_thread: current process is NULL!\n" );
			return( NULL );
		}
	}

	// Owner Process�� Page Dir�� ����Ѵ�.
	pPD = (DWORD*)get_process_page_dir( pProc );

	// Thread ����ü�� �Ҵ��� �� �ʱ�ȭ �Ѵ�.
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

	// �����带 ������ ����ü�� �߰��Ѵ�.
	nAppendThreadToList( &sch, pThread );

	// �����带 �ʱ� ���� ť�� �߰��Ѵ�.
	change_thread_state( &sch, pThread, nState );

	// TSS ����ü�� �Ҵ��� �� �ʱ�ȭ �Ѵ�.
	pThread->pTSS = (TSSStt*)kmalloc( sizeof( TSSStt ) );
	if( pThread->pTSS == NULL )
	{
		kdbg_printf( "kcreate_thread: tss allocation failed!\n" );
		kfree( pThread );	// Release thread structure
		return( NULL );
	}
	// make TSS.  (������ ������ ����� ���� TSS�� �ʱ�ȭ�ؾ� �Ѵ�.)
	vMakeTSS( pThread->pTSS, (DWORD)kthread_entry_point, 0 );  // ����! ESP�� �ϴ� 0�� ���� �ִ´�.
	pThread->pTSS->dwCR3 = (DWORD)pPD;	// set CR3

	// register tss
	sprintf( szT, "T%d", pThread->dwID );
	nAppendTSSTbl( szT, (DWORD)pThread->pTSS );

	// ������ ������ ��� �ش�.
	if( dwStackSize != KTHREAD_NO_STACK )
	{	// mapping thread stack
		if( dwStackSize < KTHREAD_STACK_SIZE )	 // ������ ���� ũ�� �̻��� �Ҵ��Ѵ�.
			dwStackSize = KTHREAD_STACK_SIZE;

		dwESP = reserve_thread_stack( pProc, pThread, dwStackSize );
		if( dwESP == 0 )
		{	// ������ ������ �Ҵ��� �� ����.
			kdbg_printf( "kcreate_thread: reserve_thread_stack failed!\n" );
			return( NULL );
		}
		// TSS�� ESP���� �ٽ� ������ �ش�.
		pThread->pTSS->dwESP  = dwESP;
		pThread->pTSS->dwESP0 = dwESP;
	}
	else
	{	// ������ �������̹Ƿ� ������ �Ҵ����� �ʴ´�.
	}

	// EAX �������Ϳ� Call�� Function�� �ּҸ� �����ϸ� kthread_entry_point���� �ݵȴ�.
	pThread->pTSS->dwEAX = dwFunc;
	pThread->pTSS->dwEBX = dwParam;		// set parameter to ebx

	// �����带 ���μ����� �߰��Ѵ�.
	insert_thread_to_process( pProc, pThread );

	// �������� Ű�Է� ť�� �Ҵ��Ѵ�.
	alloc_thread_kbd_q( pThread );

	return( pThread );
}

// �����带 close�ϰ� ���ҽ��� ȸ���Ѵ�.
// �� �Լ��� init_task�� context �󿡼� ȣ��ȴ�.
// ������ ���丮�� ���μ��� �Ұ��̹Ƿ� �ǵ帮�� �ȵȴ�.
int kclose_thread( ThreadStt *pThread )
{
	int nR;

	// ��ȿ�� �������ΰ�?
	if( pThread == NULL || memcmp( pThread->szMagicStr, THREAD_MAGIC_STR, 6 ) != 0 )
	{
		kdbg_printf( "kclose_thread: invalid thread!\n" );
		return( -1 );
	}

	// 'FG' Thread�̸� NULL�� �����Ѵ�.  2002-12-08
	if( pThread->pProcess != NULL && pThread->pProcess->pForegroundThread == pThread )
	{
		pThread->pProcess->pForegroundThread = NULL;
		//kdbg_printf( "process.FG <- NULL\n" );
	}

	// �������� ���¸� �����Ѵ�. (TS_TERMINATED)
	nR = change_thread_state( &sch, pThread, TS_TERMINATED );
	if( nR < 0 )
	{
		kdbg_printf( "change_thread_state( TS_TERMINATED ) failed!\n" );
		return( -1 );
	}

	// Owner ���μ������� �����.
	nR = delete_thread_from_process( pThread );
	if( nR < 0 )
	{
		kdbg_printf( "delete_thread_from_process() failed!\n" );
		return( -1 );
	}

	// �������� KBQ Q�� �����Ѵ�.
	free_thread_kbd_q( pThread );

	// Wait Object�� �����Ѵ�.
	nR = release_thread_wait_object( pThread );
	if( nR < 0 )
	{
		kdbg_printf( "kclose_thread() - release wait objects failed!\n" );
		return( -1 );
	}

	// ������ ������ �����Ѵ�.  (���μ��� ��ũ���� ������ �ɷ� �ִ�.)
	pThread->pStack->pThread = NULL;
	pThread->pStack = NULL;

	// Current Thread�̸� NULL�� �����Ѵ�.  2002-12-08
	if( pThread == get_current_thread() )
	{
		sch.pCurrentThread = NULL;
		//kdbg_printf( "Current thread <- NULL\n" );
	}

	return( 0 );
}

// 	������ ����ü�� �����Ѵ�.
int krelease_thread_struct( ThreadStt *pThread )
{
	int nR;

	if( pThread == NULL || pThread->nState != TS_TERMINATED )
	{
		kdbg_printf( "release_thread_struct() - invalid thread or thread state is not TS_TERMINATED!\n" );
		return( -1 );
	}

	// ������ ����ü�� TS_TERMINATED ť���� �����Ѵ�.
 	nR = nPopThread( pThread );
	if( nR == -1 )
	{
		kdbg_printf( "release_thread_struct() - pop thread from TS_TERMINTED failed!\n" );
		return( -1 );
	}

	// �����층ũ���� �����带 �����Ѵ�.
	nDeleteThreadFromList( &sch, pThread );

	// TSS�� �����Ѵ�.
	kfree( pThread->pTSS );

	// ThreadStt�� �����Ѵ�.
	kfree( pThread );

	return( 0 );
}

// ���μ����� �ּ� ���� ����ü�� �Ҵ��Ѵ�.
// ������ ���丮�� �Ҵ��Ѵ�.
static AddrSpaceStt *alloc_addr_space()
{
	AddrSpaceStt *pA;

	pA = (AddrSpaceStt*)kmalloc( sizeof( AddrSpaceStt) );
	if( pA == NULL )
		return( NULL );

	memset( pA, 0, sizeof( AddrSpaceStt ) );

	// �ʱ� ���� 1�� �����ؾ� �Ѵ�.
	pA->nForkRefCounter = 1;

	// PageDir�� �Ҵ��ϰ� Ŀ���� ���� �����Ѵ�.
	pA->pPD = pAllocPageTable();
    memcpy( pA->pPD, bell.pPD, 4096 );

	return( pA );
}

// ���ο� ���μ����� �����Ѵ�.  (������� ���� ����)
ProcessStt *kcreate_process( ProcessStt *pParent )
{
	ProcessStt	*pProc;

	// ���μ��� ����ü�� �Ҵ��ϰ� �ʱ�ȭ �Ѵ�.
	pProc = (ProcessStt*)kmalloc( sizeof( ProcessStt ) );
	if( pProc == NULL )
	{	// �޸� �Ҵ� ����.
M_ERR:	kdbg_printf( "kcreate_process: memory allocation error!\n" );
		return( NULL );
	}
	memset( pProc, 0, sizeof( ProcessStt ) );

	// �ּҰ��� ����ü�� �Ҵ��Ѵ�.
	pProc->pAddrSpace = alloc_addr_space();
	if( pProc->pAddrSpace == NULL )
	{	// �޸� �Ҵ� ����.
		kfree( pProc );
		goto M_ERR;
	}

	strcpy( pProc->szMagicStr, PROC_MAGIC_STR );
	pProc->dwID		    	= dwMakeProcessThreadID( (DWORD)pProc );
	pProc->dwNextStackBase 	= KPROCESS_STACK_TOP;
	if( pParent != NULL )
		pProc->dwParentID = pParent->dwID;

	// Address Space�� OWner PID�� �����Ѵ�.  (PID ������ ������ ��.)
	pProc->pAddrSpace->nOwnerPID = pProc->dwID;

	// ���μ����� ������ ť�� �߰��Ѵ�.
	append_process_to_sch_q( &sch, pProc );

	// FileName, Arg, EvnString ���� ������ ���� �Ҵ�.
	// pProc->pExecParam = (ExecParamStt*)kmalloc( sizeof( ExecParamStt ) );

	// ���μ����� �޸� Ǯ�� �ʱ�ȭ �Ѵ�.
	// MEM_POOL_SIZE must be 0!
	init_memory_pool( (DWORD*)get_process_page_dir( pProc ), &pProc->mp, PROCESS_MEM_POOL_ADDR, 0 );

	return( pProc );
}

// ���μ��� ��� child wait ������ �����尡 ������ �� ������ �����͸� �����Ѵ�.
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

// ���μ����� ������ť�� zombie ��ũ�� �߰��Ѵ�.
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

	// ���¸� ����� �����Ѵ�.
	pP->dwState = PSTATE_ZOMBIE;

	return( 0 );
}

// ���μ����� ������ť�� zombie ��ũ���� �����Ѵ�.
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

// �ּ� ������ ���μ����κ��� �����.
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

		// Reference Counter�� 1 ���̰� '0'�̸� �ּҰ����� �����Ѵ�.
		pA->nForkRefCounter--;
		if( pA->nForkRefCounter <= 0 )
		{	
			// �����ϰ� ���� �����ϴ� �����ΰ�?
			if( pA->pForkRef == NULL )
			{	// ����� ������ ������ ��� �����Ѵ�.
				release_user_area( pA );
				nFreePageTable( (DWORD)pA->pPD );
				//kdbg_printf( "release all user area( PID = %d )\n", pA->nOwnerPID );
			}
			else
			{	// RW�� ���� �Ҵ��� �����̴�.
				release_rw_user_area( pA );
				//kdbg_printf( "release rw user area( PID = %d )\n", pA->nOwnerPID );
			}

			kfree( pA );
		}
	}

	return( 0 );
}

// 	���μ��� ����ü�� �����Ѵ�.
int krelease_process_struct( ProcessStt *pProcess )
{
	int			nR;
	ThreadStt	*pT;
	ProcessStt	*pParent;

	// ���μ����� ������ ť���� �����Ѵ�.
	nR = delete_process_from_sch_q( &sch, pProcess );

	//if( pProcess->pExecParam != NULL )
    //{   // FileName, Arg, EvnString ���� ������ ���� ����.
	//	kfree( pProcess->pExecParam );
	//	pProcess->pExecParam = NULL;
    //}

	// pExecveParam�� �����Ѵ�.
	if( pProcess->pKExecveParam != NULL )
	{	// Ŀ�� ������ ��쿡�� kfree�� �����Ѵ�.
		if( ( (DWORD)pProcess->pKExecveParam & 0x80000000 ) == 0 )
			kfree( pProcess->pKExecveParam );
	}

	// parent process�� ��� ������ zombie�� �����.
	pParent = find_process_by_id( pProcess->dwParentID );
	if( pParent != NULL )
	{
		// �켱 Zombie ��ũ�� �߰��Ѵ�.
		link_to_zombie( pProcess );

		// parent�� thread ��� wait child ������ �����尡 �ִ��� ã�ƺ���.
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
		// �׳� �ٷ� ������ ������.
		kfree( pProcess );
	}

	return( 0 );
}

// kill thread
int kill_thread( ThreadStt *pThread )
{
	ThreadStt	*pT, *pCurT;
	
	// sniper thread�� ã�´�.
	pT = find_thread_by_alias( "init_sniper" );
	if( pT == NULL )
	{
		kdbg_printf( "kill_thread: sniper thread not found!\n" );
		return( -1 );
	}
	
	// sniper������ kmesg�� ������.
	ksend_kmesg( pT->dwID, KMESG_KILL_THREAD, pThread->dwID, 0 );

	pCurT = get_current_thread();
	if( pCurT == pThread )
	{
		change_thread_state( NULL, pCurT, TS_WAIT );
		kernel_scheduler();
	}
	
	return( 0 );
}

// ���μ����� �����Ѵ�.
int kill_process( ProcessStt *pProcess )
{	
	ThreadStt		*pT;
	ProcessStt	*pP;
	
	// sniper thread�� ã�´�.
	pT = find_thread_by_alias( "init_sniper" );
	if( pT == NULL )
	{
		kdbg_printf( "kill_process: sniper thread not found!\n" );
		return( -1 );
	}
	
	// sniper������ kmesg�� ������.
	ksend_kmesg( pT->dwID, KMESG_KILL_PROCESS, pProcess->dwID, 0 );

	// �ڱ� �ڽ��̸� �����ؼ��� �ȵȴ�.
	// (�޽����� ������ ���� �������۰� ���� ���μ����� ���� ����...)
	pP = k_get_current_process();
	if( pP == pProcess )
	{
		//kdbg_printf( "kill_process(PID=%d): wait for death.\n", pP->dwID );
		change_thread_state( NULL, get_current_thread(), TS_WAIT );
		kernel_scheduler();
	}		

	return( 0 );
}

// ID�� �����带 ã�´�.
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

// TSS�� �����带 ã�´�.
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

// szAlias�� �����带 ã�´�.
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

	// ������ ��ũ���� ã�� ����.
	for( pP = sch.pStartProcess; pP != NULL; pP = pP->pNextProcess )
	{
		if( pP->dwID == dwPID )
			return( pP );
	}

	// ���� ��ũ���� ���μ����� ã�´�.
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
		// ���� ������ �����ϰ� ������ ���� �����Ѵ�.
		MOV dwOrgESP, ESP
		LEA ESP, stack
		ADD ESP, _TMP_STK_SIZE_FOR_FORK_ -4
	}

	pParent = k_get_current_process();

	// ���μ����� �����带 �����Ѵ�.
	pP = kcreate_process( pParent );
	if( pP == NULL )
		goto ERR;

	// pVCon�� NULL�̸� Console�� ���� ������ ���̴�.
	pP->pVConsole = pVCon;
	pP->pKExecveParam = (KExecveParamStt*)dwParam;

	// CR3 ���� �����Ѵ�.
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

	// CR3�� ������ �����Ѵ�.
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
	// CR3�� ������ �����Ѵ�.
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

// �����带 ������ �� ���� Context�� �״�� �����Ѵ�.
// ���� �� ����� ������ �ּҰ����� RDONLY�� �����Ѵ�.
static ThreadStt *duplicate_thread( ProcessStt *pNewProc, ThreadStt *pThisThread, TSSStt *pTSS )
{
	ThreadStt		*pT;
	TStackLinkStt	*pStk;

	// ���� ���� �����带 �Ҵ��Ѵ�.
	pT = kcreate_thread( pNewProc, KTHREAD_NO_STACK, 0, 0, TS_READY_NORMAL );
	//pT = kcreate_thread( pNewProc, KTHREAD_NO_STACK, 0, 0, TS_WAIT );

	// ���� �����忡�� �ʿ��� ������ �����Ѵ�.
	pT->dwMappingFlag  = pThisThread->dwMappingFlag;
	pT->dwR3EntryFunc  = pThisThread->dwR3EntryFunc;
	pT->dwNice	 	   = pThisThread->dwNice;
	pT->dwCurNice	   = pThisThread->dwCurNice;

	// ���� ��ũ�� ���� �Ҵ��� �� �����Ѵ�.
	pStk = (TStackLinkStt*)kmalloc( sizeof( TStackLinkStt ) );
	if( pStk == NULL )
	{
		kdbg_printf( "duplicate_thread: kmalloc failed!\n" );
	}
	memcpy( pStk, pThisThread->pStack, sizeof( TStackLinkStt ) );
	pStk->pPre = pStk->pNext = NULL;
	pStk->pThread = pT;
	pT->pStack    = pStk;

	// ���� ��ũ�� ���μ����� �߰��Ѵ�.
	append_Stack_link( pNewProc, pStk );

	// Current CR3�� �״�� �����ȴ�. (��������� CR3�� ������ �־�� �Ѵ�.)
	vMakeTSS( pT->pTSS, pTSS->dwEIP, pTSS->dwESP );
    pT->pTSS->dwESI    = pTSS->dwESI;
    pT->pTSS->dwEDI    = pTSS->dwEDI;
    pT->pTSS->dwEBP    = pTSS->dwEBP;
	pT->pTSS->dwEFLAG  = pTSS->dwEFLAG;
	pT->pTSS->dwCR3    = pTSS->dwCR3;
    pT->pTSS->dwEFLAG |= MASK_IF;		// Interrupt Enable

	return( pT );
}

// ���μ����� �����ϰ� ���� �����带 �����Ѵ�.
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

	// ���� ������ �������� ���ؽ�Ʈ�� �����Ѵ�.
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

	// Current CR3�� �״�� �����ȴ�.
	// duplicate_thread ���� ���� ���� �����ȴ�.
	// ESI, ESI, EDI, EBP, ESp�� �����ϱ� ���� Temporary TSS
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

	// ���ο� ���μ����� �����Ѵ�.
	pP = kcreate_process( pCurThread->pProcess );
	if( pP == NULL )
	{
		kdbg_printf( "internal_kfork: kcreate_process error!\n" );
		return( -1 );
	}

	// Parent Process�� ���� �״�� �����ϴ� �����͵�.
	pP->pMyDbg	      	= pCurProcess->pMyDbg;
	pP->pModule         = pCurProcess->pModule;
	pP->pVConsole       = pCurProcess->pVConsole;
	pP->dwNextStackBase = pCurProcess->dwNextStackBase;

	// R3 export Function ���̺��� �����Ѵ�.
	memcpy( &pP->e, &pCurProcess->e, sizeof( R3ExportTblStt ) );

	// ����� ���� ������ ���̺��� �����Ѵ�.
	dup_page_dir( pP, pCurProcess, 512 ); // 512 = ������ ������ ������ ���丮 �ε���.
			 
	// Ring0 ������ R->RW�� �����Ѵ�. (dwESP���� StackTop�� �����ϴ� ������)
	dwESI = dwESP & (DWORD)0xFFFFF000;
	dwEDI = dwR0StackTop & (DWORD)0xFFFFF000;
	//kdbg_printf( "Duplicate Ring0 Stack: 0x%X - 0x%X (R0StackTop=0x%X)\n", dwESI, dwEDI, dwR0StackTop );
	for( ; dwESI <= dwEDI; dwESI += 0x1000 )
		copy_on_write( (DWORD*)get_process_page_dir( pP ), dwESI );

	// Ring3 ������ R->RW�� �����Ѵ�.
	dwESI = (dwESP3 - 4096) & (DWORD)0xFFFFF000;
	dwEDI = dwR3StackTop & (DWORD)0xFFFFF000;
	//kdbg_printf( "Duplicate Ring3 Stack: 0x%X - 0x%X (R3StackTop=0x%X)\n", dwESI, dwEDI, dwR3StackTop );
	for( ; dwESI <= dwEDI; dwESI += 0x1000 )
		copy_on_write( (DWORD*)get_process_page_dir( pP ), dwESI );

	nPID = (int)pP->dwID;

	// CR3�� ���� �����Ѵ�.
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

	// �����带 �����Ͽ� �����Ѵ�.
	pT = duplicate_thread( pP, pCurThread, &t_tss );

	// CR3�� �����Ѵ�.
	set_cur_cr3_in_tss( dwOrgCR3 );
	_asm {
		MOV EAX, dwOrgCR3
		MOV CR3, EAX
		FLUSH_TLB2(dwOrgCR3);
	}
	///////////////////////////////////////////////////////////////////////////////

	if( pT == NULL )
	{	// ���������Ƿ� ���μ��� ����ü�� �����Ѵ�.
		nPID = -1;
		krelease_process_struct( pP );
		kdbg_printf( "internal_kfork: critical error!\n" );
		return( -1 );
	}

	// ������ �����带 FG�� �����Ѵ�.
	pP->pForegroundThread = pT;

	//kdbg_printf( "internal_kfork: end\n" );

	return( nPID );
}

// �������� ���ƾ� �ϸ� Serialize�Ǿ�� �Ѵ�.
int kfork( DWORD dwESP3 )
{
	int nPID;
	nPID = internal_kfork( dwESP3 );
	return( nPID );
}

// �ϳ��� ���μ����� �ڼ��ϰ� ����Ѵ�.
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

// �ϳ��� �����带 �ڼ��ϰ� ����Ѵ�.
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

// ������ ť�� ������ ����Ѵ�.
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

// ���� ���μ����� ���� ���� ��� �� �� ���μ��� ����ü�� �����Ѵ�.
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

	// ��� �����尡 ����� ������ Ȯ���Ѵ�.
	if( pP->dwState != PSTATE_ZOMBIE )
	{	// �����尡 ������ ������� �ʾҴ�.
		kdbg_printf( "get_zombie_exit_code: process is not zombie.\n" );
		_asm int 1
	}

	// ���μ����� ���� ��ũ���� �и��Ѵ�.
	unlink_from_zombie( pP );
	nExitCode = pP->nExitCode;

	// ���μ��� ����ü�� �����Ѵ�.
	kfree( pP );

	return( nExitCode );
}

// Child Process�� ����� ������ ����Ѵ�.
int kwait( int *pExitCode )
{
	ThreadStt	*pT;
	int			nType, nPID, nExitCode;

	pT = get_current_thread();
	if( pT == NULL )
		return( -1 );

	// Child Process�� ����� ������ ����Ѵ�. (Parameter�� ���� �ʴ´�.)
	nType = kwait_kmesg( KMESG_CHILD_PROCESS_EXIT, &nPID, NULL, 0 );
	if( nType < 0 )
	{
		kdbg_printf( "kwait: error!\n" );
		return( -1 );
	}

	// ���� ���� �˾Ƴ���.
	nExitCode = get_zombie_exit_code( nPID );
	if( pExitCode != NULL )
		pExitCode[0] = nExitCode;

	return( nPID );
}

// Ư�� Child Process�� ����� ������ ����Ѵ�.
int kwaitpid( int nPID, int *pExitCode )
{
	ThreadStt	*pT;
	int			nR, nResultPID, nExitCode;

	pT = get_current_thread();
	if( pT == NULL )
		return( -1 );

	// Child Process�� ����� ������ ����Ѵ�.
	for( ;; )
	{	// ���� ���� Child Process ��� ����� ���� ID�� nResultPID�� ���޵ȴ�.
		nR = kwait_kmesg( KMESG_CHILD_PROCESS_EXIT, &nResultPID, NULL, 0 );
		if( nResultPID == nPID )
			break;
	}

	// ���� ���� �˾Ƴ���.
	nExitCode = get_zombie_exit_code( nResultPID );
	if( pExitCode != NULL )
		pExitCode[0] = nExitCode;

	return( nResultPID );
}

// Ư�� Thread�� ����� ������ ����Ѵ�.
int kwaittid( int nTID, int *pExitCode )
{
	ThreadStt	*pCurT, *pT;
	int 		nR, nResultTID;

	pCurT = get_current_thread();
	if( pCurT == NULL )
		return( -1 );

	pT = find_thread_by_id( nTID );
	if( pT == NULL )
	{	// ���Ḧ ����Ϸ��� Thread�� ã�� �� ����.
		kdbg_printf( "kwaittid: TID(%d) not found!\n", nTID );
		return( -1 );
	}

	if( pT->dwKillerTID != pCurT->dwID )
	{	// ���Ḧ ����Ϸ��� Thread�� dwKillerTID�� ���� Thread�� ID�� �ٸ���.  (��ٷ����� �ҿ��� ����.)
		kdbg_printf( "kwaittid: TID(%d).dwKillerTID(%d) != current TID(%d)\n", 
			nTID, pT->dwKillerTID, pCurT->dwID );
			
		return( -1 );
	}	

	// Thread�� ����� ������ ����Ѵ�.
	for( ;; )
	{	// ����� Thread�� ID�� nResultTID�� ���޵ȴ�.
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

// Application�� �ε��Ͽ� �����Ѵ�.
static DWORD kexecve_thread_entry( DWORD dwParam )
{
	KExecveParamStt	*pEP;

	pEP = (KExecveParamStt*)dwParam;

	launch_r3_program( pEP );

	// ������� ���� ���� ���̴�.
	for( ;; )
		kernel_scheduler();

	return( 0 );
}

// �� ���α׷� �ε忡 �ʿ��� �ķ����͸� �����Ѵ�.
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

	// ������ ����, ���̸� ���Ѵ�.
	nFileSize = strlen( pFile ) + 1;

	nArgSize = 0;
	// argc[0] = pFileName�ؾ� �ϹǷ� 1���� �����Ѵ�.
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

	// FileName�� �����Ѵ�.
	strcpy( pFileName, pFile );
	nI = nFileSize;

	// Argv�� �����Ѵ�.
	pP->ppArgv[0] = pFileName;			// argv[0]�� ���� �н��� �����Ѵ�.
	for( nK = 1, nJ = 0; ; nJ++, nK++ )
	{
		if( ppArgv[nJ] == NULL )		// argv[1] ���� parameter�� �����Ѵ�.
		{
			pP->ppArgv[nK] = NULL;
			break;
		}

		pP->ppArgv[nK] = &pStrBuff[nI];
		strcpy( &pStrBuff[nI], ppArgv[nJ] );
		nI += strlen( ppArgv[nJ] ) +1;
	}

	// Env�� �����Ѵ�.
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

	// nI�� nSttSize�� ���ƾ� �Ѵ�.
	nI += sizeof( KExecveParamStt ) + (int)( (DWORD)pStrBuff - (DWORD)pP->ppArgv );

	// ����ü + ���� ����� �����Ѵ�.
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

    // �ο� ��ȣ�� ���� ���.
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

// �� ���α׷� �ε忡 �ʿ��� �ķ����͸� �����Ѵ�.
static KExecveParamStt *make_kexecve_param_ex( char *pFile, char *pArg, char *pEnv )
{
	KExecveParamStt 	*pP;
	int 				nI, nK, nJ, nT;
	char				*pStrBuff, *pNext;
	int					nArgCounter, nEnvCounter, nArgSize, nEnvSize, nFileSize, nSttSize;

	if( pFile == NULL || pFile[0] == 0 )
		return( NULL );

	// ������ ũ�⸦ ���Ѵ�.
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

	// Arg �����͸� �����ϰ� ��Ʈ���� �����Ѵ�.
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

// ���μ����� �ּҰ����� ���ο� ���α׷��� �ε��Ѵ�.
int kexecve( char *t_pFile, char *t_argv[], char *t_envp[] )
{
	DWORD				dwT;
	KExecveParamStt		*pEP;
	int					nNewPID;
	ProcessStt			*pP, *pCurP;

	// �޸� �Ҵ��� �Ͼ��.
	pEP = make_kexecve_param( t_pFile, t_argv, t_envp );

	// wait ���·� ���μ����� �����带 �����Ѵ�.
	nNewPID = kernel_fork( kexecve_thread_entry, (DWORD)pEP, (128*1024), TS_WAIT, get_current_vconsole() );
	if( nNewPID < 0 )
		return( -1 );

	// ������ ���μ����� �ּҸ� ���Ѵ�.
	pP = find_process_by_id( nNewPID );
	if( pP == NULL )
	{
		kdbg_printf( "kexecve: find_process_by_id( %d ) = NULL!\n", nNewPID );
		return( -1 );
	}

	pCurP = k_get_current_process();

	// ���� ���μ����� �ű� ���μ������� ��ȯ�ؾ� �� �����ʹ� ��ȯ�Ѵ�.
	dwT					= pP->dwID;
	pP->dwID			= pCurP->dwID;;
	pP->dwParentID		= pCurP->dwParentID;;
	pP->pVConsole		= pCurP->pVConsole;
	pP->pKExecveParam	= pEP;				// kexecve param�� �����Ѵ�.

	// ���⼭ VConsole�� NULL�� ����� ������ ���ڿ� ����� �ȵȴ�. !!!
	//pCurP->pVConsole  = NULL;

	pCurP->dwID       = dwT;	// ���μ��� ID�� ��ȯ�Ѵ�.
	pCurP->dwParentID = 0;		// parent ID�� ���� ������ �Ѵ�.

	// �������� ���¸� �����Ѵ�.
	change_thread_state( NULL, pP->pStartThread, TS_READY_NORMAL );

	// ���� ���μ����� �״´�.  
	//kdbg_printf( "kexecve: kill_process( %d )\n", pCurP->dwID );
	kill_process( pCurP );

	return( 0 );
}

// ���ο� ���� �ܼ��� �����Ͽ� login.exe�� �����Ѵ�.
DWORD r0_fork_thread( DWORD dwParam )
{
	ProcessStt		*pP;
	KExecveParamStt	*pEP;

	pEP = (KExecveParamStt*)dwParam;

	// vconsole�� �����Ѵ�.
	pP = k_get_current_process();
	if( pP->pVConsole == NULL )
		pP->pVConsole = make_vconsole();
	else
	{
		//kdbg_printf( "r0_session: pP->pVConsole is not NULL(ID=%d) !\n",
		//	pP->pVConsole->nID );
	}

	// login.exe�� ����ǰ� �ִ� �ܼ��� Active�� �����Ѵ�.
	//set_active_vconsole( pP->pVConsole );

	// ���� ���μ����� FG�� �����Ѵ�.
	set_fg_process( NULL, pP );

	launch_r3_program( pEP );

	// ������� ���� ���� ���̴�.
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
		return( -1 );	// ���� �������� ���°� �ƴϴ�.
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
	{	// SUSPEND�� �ƴ϶� �̹� READY�� ����...
		//kdbg_printf( "ksuspend_thread: thread(%d) is not suspend state!\n", pT->dwID );
		return( -1 );	// ���� �������� ���°� �ƴϴ�.
	}

	nR = change_thread_state( NULL, pT, pT->nPrevThreadState );

	return( nR );
}


