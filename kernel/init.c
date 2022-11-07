#include "bellona2.h"
#include <uarea.h>

static ThreadStt	*pInitThread = NULL;
static char			szInitThreadPrompt[] = "#";

// init ���μ����� �ñ׳� ó���� ����ü.
// Ŀ�� ��忡���� �����ϹǷ� startup.lib�� ��ũ���� �ʴ´�.
// �׷��Ƿ� Ŀ�� ������ signal ����ü�� ����� �־�� �Ѵ�.
static UAreaStt init_proc_uarea;

DWORD get_init_phase()
{
	return( bell.dwInitPhase );
}

void set_init_phase( DWORD dwInitPhase )
{
	bell.dwInitPhase = dwInitPhase;
}

ThreadStt *get_init_thread()
{
	return( pInitThread );
}	

// ���μ����� ���� ��ũ�� �����Ѵ�.
static int release_process_stack_link( ProcessStt *pP )
{
	TStackLinkStt *pT, *pNext;

	for( pT = pP->pStartStk; pT != NULL; pT = pNext )
	{
		pNext = pT->pNext;
		kfree( pT );
	}
	pP->pStartStk = pP->pEndStk = NULL;

	return( 0 );
}

// ���μ����� �����Ѵ�.
static int sniper_kill_process( DWORD dwPID )
{
	int					nR;
	ProcessStt			*pProcess, *pNextFG;
	ThreadStt				*pThread, *pNextThread;

	pProcess = find_process_by_id( dwPID );
	if( pProcess == NULL )
	{
		kdbg_printf( "sniper_kill_process: PID(%d) not found!\n", dwPID );
		return( -1 );
	}

	// pProcess�� ���� �ֿܼ��� FG�� �����Ǿ� ������ NULL�� �����Ѵ�.
	pNextFG = pProcess->pNextFG;
	del_fg_link( pProcess->pVConsole, pProcess );
	//set_fg_process( NULL, pNextFG );  �̰� �� �� �ʿ䰡 ���� ��.  2003-02-28

	// ���μ��� ���� ��� �����带 �����Ѵ�.
	for( pThread = pProcess->pStartThread; pThread != NULL; pThread = pNextThread )
	{
		pNextThread = pThread->pNextPLink;
	
		nR = kclose_thread( pThread );
		if( nR >= 0 )
		{
			nR = krelease_thread_struct( pThread );
			if( nR < 0 )
				kdbg_printf( "sniper_kill_process_func: thread structure 0x%08X releasing failed!\n", pThread );
		}
		else
			kdbg_printf( "sniper_kill_process_func: thread 0x%08X closing failed!\n", pThread );
	}	  

	// ���� ��ũ ����ü���� �����Ѵ�.
	release_process_stack_link( pProcess );

	// �ּ� ������ ���μ����κ��� �����.
	detach_addr_space( pProcess ); 

	// ���μ��� ����ü�� �����Ѵ�.
	nR = krelease_process_struct( pProcess );

	return( 0 );
}

static int sniper_kill_thread( DWORD dwTID )
{
	int						nR;
	ThreadStt				*pThread;
	DWORD					dwKillerTID;

	pThread = find_thread_by_id( dwTID );
	if( pThread == NULL )
	{
		kdbg_printf( "sniper_kill_thread: TID(%d) not found!\n", dwTID );
		return( -1 );
	}

	dwKillerTID = pThread->dwKillerTID;

	nR = kclose_thread( pThread );
	if( nR < 0 )
		return( -1 );

	nR = krelease_thread_struct( pThread );
	if( nR < 0 )
		return( -1 );

	if( dwKillerTID == 0 )
		return( 0 );

	// Killer TID�� �޽����� �����Ѵ�.
	pThread = find_thread_by_id( dwKillerTID );
	if( pThread == NULL )
	{
		kdbg_printf( "sniper_kill_thread: dwKillerTID(%d) not found!\n", dwKillerTID );
		return( 0 );
	}

	//kdbg_printf( "sniper_kill_thread: dwKillerTID(%d) <- KMESG_CHILD_THREAD_EXIT\n", dwKillerTID );
	ksend_kmesg( dwKillerTID, KMESG_CHILD_THREAD_EXIT, dwTID, 0 );

	return( 0 );
}

// sniper thread
int sniper_thread( void *pPram )
{
	int		nR, nType;
	DWORD	dwID;

	for( ;; )
	{
		nType = kwait_kmesg( KMESG_ANY, &dwID, NULL, 0 );

		//kdbg_printf( "sniper_thread: nType = %d\n", nType );

		if( nType < 0 )
			continue;
		if( nType == KMESG_KILL_PROCESS )
			nR = sniper_kill_process( dwID );
		else if( nType == KMESG_KILL_THREAD )
			nR = sniper_kill_thread( dwID );
	}	

	return( 0 );
}	

// Ŀ�� ���μ����� �ʱ�ȭ ��
int init_thread( void *pPram )
{
	ProcessStt			*pP;
	KExecveParamStt		*pEP;
	ThreadStt			*pThread;

	set_init_phase( INIT_PHASE_ENTER_INIT_THREAD );

	// ���� �ܼ��� �ʱ�ȭ �Ѵ�.
	init_sys_vconsole();

	// Ŀ�� ���μ���(PID=1)�� ���� �ܼ��� �Ҵ��Ѵ�.
	pP = k_get_current_process();
	pP->pVConsole = get_kernel_vconsole();

	// init ���μ����� �ñ׳� ����ü�� �����Ѵ�.
	pP->e.func[R3EXI_UAREA] = (DWORD)&init_proc_uarea;
	
	// init semaphore
	init_system_semaphore();

	pInitThread = pThread = get_current_thread();

	set_fg_process( NULL, pP );
	set_foreground_thread( pThread );

	init_char_dev();
	init_sys_module_struct();

	kernel_scheduler();

	kdbg_clearscreen();
	disp_version();
	nInitOpCodeTbl();
	scan_pci_devices();
	ide_auto_detection();
	init_vfs();

	// init nic
	//init_nic();

	// Built-in V86 Library�� �ε���¸� ����Ѵ�.
	if( bell.dwBuiltInV86Lib != 0 )
		kdbg_printf( "Built-in V86 Library (%d bytes at 0x%08X).\n", bell.nBuiltInV86LibSize, bell.dwBuiltInV86Lib );
	else
		kdbg_printf( "No built-in V86 Library.\n" );

	// ���� �ܼ��� �����Ѵ�.
	set_active_vconsole( get_kernel_vconsole() );

	// ����� ������ �ε��Ѵ�.
	load_bellona2_dbginfo( "/c/bellona2.dbg" );
	
	// �ʿ��� ������ �ε��Ѵ�.
	load_module( "/c/gui.mod" );
	load_user_module( "/c/stdlib.mod" );
	load_user_module( "/c/guiapp.mod" );
	load_user_module( "/c/jpeg.mod" );

	// user shell�� �����Ѵ�.  (thread_entry, param, stack_size, SCHEDULE_LEVEL
	pEP = make_kexecve_param( "/c/login.exe", NULL, NULL );
	if( pEP != NULL )	// ���ο� ���� �ܼ��� �����Ѵ�.
		kernel_fork( r0_fork_thread, (DWORD)pEP, (128*1024), TS_READY_NORMAL, NULL );
	else
		kdbg_printf( "init_thread: pEP = NULL!\n" );
	
	// ������
	for( ;; )
	{	// Ŀ�� ���� �����Ѵ�.
		kdbg_shell( (char*)szInitThreadPrompt ); 

		kernel_scheduler();
	}

	return( 0);
}

static int ifunc = 0;
void jmp_init_task( int nFunc )
{
    static DWORD addr[2];

	ifunc = nFunc;

    dwSetDescriptorAddr( &gdt[GSEL_DUMMY_TSS32/8], (DWORD)&init_tss );

	addr[0] = 0;
    addr[1] = (DWORD)GSEL_DUMMY_TSS32;

	// clear busy bit to jump
	vSetDescriptorBusyBit( &gdt[GSEL_DUMMY_TSS32/8], 0 );

	_asm JMP FWORD PTR addr;
}
/*
static int init_task_fork_func()
{
	UINT16	wBackLink;
	static  ForkParamStt fp;

	get_global_fork_param( &fp );

	// ���μ����� �����Ѵ�.
	fp.pNewProcess = kcreate_process( fp.pParentThread->pProcess );  
	if( fp.pNewProcess == NULL )
	{
		kdbg_printf( "init_task_fork_func : kcreate_process() failed!\n" );
		goto FAR_RETURN;
	}

	// ���� �ķ����͸� �����Ѵ�.
	if( fp.pNewProcess->pExecParam != NULL )
		memcpy( fp.pNewProcess->pExecParam, &fp.exec_param, sizeof( ExecParamStt ) );

	// �����带 �����Ѵ�. ( Owner Process, StackSize, Entry Function, Parameter )
    // fp.pFunc == exec_new_program (fork.c)
	fp.pNewThread = kcreate_thread( fp.pNewProcess, fp.dwStackSize, (DWORD)fp.pFunc, (DWORD)&fp.exec_param, fp.nInitialState );	
	if( fp.pNewThread == NULL )
		kdbg_printf( "init_task_fork_func: kcreate_thread() failed!\n" );

	// ���ʷ� ������ �����带 FG ������� �����Ѵ�.	2002-12-19
	fp.pNewProcess->pForegroundThread = fp.pNewThread;

	kdbg_printf( "init_tasl_fork_param: ok\n" );

FAR_RETURN:

    // ���� ���� �����Ѵ�.
    set_global_fork_param( &fp );

    // ���� TSS�� BackLink�� ���Ѵ�.
	wBackLink = wGetBackLink( &init_tss );	
	vSetDescriptorBusyBit( &gdt[ wBackLink/8 ], 1 );

	return( 0 );
}
*/
static InitTaskKillParamStt g_kp;
int	set_global_kill_param( void *pV )
{
	InitTaskKillParamStt *pKP;

	pKP = pV;

	memcpy( &g_kp, pKP, sizeof( g_kp ) );
	return( 0 );
}
static int get_global_kill_param( void *pV )
{
	InitTaskKillParamStt *pKP;

	pKP = pV;

	memcpy( pKP, &g_kp, sizeof( g_kp ) );
	return( 0 );
}

static int init_task_kill_thread()
{
	int						nR;
	InitTaskKillParamStt	kp;
	ThreadStt				*pThread;

	get_global_kill_param( &kp );

	pThread = (ThreadStt*)kp.dwTarget;

	nR = kclose_thread( pThread );
	if( nR < 0 )
		return( -1 );

	nR = krelease_thread_struct( pThread );
	if( nR < 0 )
		return( -1 );

	return( 0 );
}

// ���μ����� �����Ѵ�.
static int init_task_kill_process()
{
	int						nR;
	InitTaskKillParamStt	kp;
	ProcessStt				*pProcess, *pNextFG;
	ThreadStt				*pThread, *pNextThread;

	get_global_kill_param( &kp );

	pProcess = (ProcessStt*)kp.dwTarget;
	kdbg_printf( "%%%%%%%%%%%%%%%%%(1)\n" );

	// pProcess�� ���� �ֿܼ��� FG�� �����Ǿ� ������ NULL�� �����Ѵ�.
	pNextFG = pProcess->pNextFG;
	del_fg_link( pProcess->pVConsole, pProcess );
	set_fg_process( NULL, pNextFG );
	

	kdbg_printf( "%%%%%%%%%%%%%%%%%(2)\n" );

	//if( pNextFG == NULL )
	//	kdbg_printf( "FG process <= NULL\n" );
	//else
	//	kdbg_printf( "FG process <= PID(%d)\n", pNextFG->dwID );

	// ���μ��� ���� ��� �����带 �����Ѵ�.
	for( pThread = pProcess->pStartThread; pThread != NULL; pThread = pNextThread )
	{
		pNextThread = pThread->pNextPLink;

		kdbg_printf( "%%%%%%%%%%%%%%%%%(2-1)\n" );
	
		nR = kclose_thread( pThread );
		kdbg_printf( "%%%%%%%%%%%%%%%%%(3)\n" );
		if( nR >= 0 )
		{
			nR = krelease_thread_struct( pThread );
		kdbg_printf( "%%%%%%%%%%%%%%%%%(4)\n" );
			if( nR < 0 )
				kdbg_printf( "init_task_kill_process_func() - thread structure 0x%08X releasing failed!\n", pThread );
		}
		else
			kdbg_printf( "init_task_kill_process_func() - thread 0x%08X closing failed!\n", pThread );
	}	  

	// ���� ��ũ ����ü���� �����Ѵ�.
	release_process_stack_link( pProcess );

	// �ּ� ������ ���μ����κ��� �����.
	detach_addr_space( pProcess ); 

	// ���μ��� ����ü�� �����Ѵ�.
	nR = krelease_process_struct( pProcess );

	return( 0 );
}

//************************************************************************//
//								  init Task								  //  
//************************************************************************//
void init_task_main()
{
    for( ;; )
    {
        switch( ifunc )
        {
        case INIT_TASK_FORK_FUNC :
	    	//init_task_fork_func();
            break;
        case INIT_TASK_KILLTHREAD_FUNC :
	    	init_task_kill_thread();
            break;
        case INIT_TASK_KILLPROCESS_FUNC :
	    	init_task_kill_process();
            break;
        }
        ifunc = 0;
	    kernel_scheduler();
    }
}	

