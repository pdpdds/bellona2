#include "bellona2.h"

#define SIMULATE_CALL_PARAMETERS	2
_declspec(naked) void simulate_call( DWORD dwEIP3, DWORD dwESP3 )
{	
	static ThreadStt	*pThread;
	static DWORD		dw_user_thread_entry, dw_real_thread_entry;

	pThread = get_current_thread();
	if( pThread == NULL )
		_asm RETN;

	_asm {
		MOV EAX, [ESP+4]				//dwEIP3
		MOV dw_real_thread_entry, EAX
	}
	
	dw_user_thread_entry = pThread->pProcess->e.func[1];
	if( dw_user_thread_entry == 0 )
	{
		dw_user_thread_entry = dw_real_thread_entry;
		dw_real_thread_entry = 0;
	}

	_asm {
		// do not use direct variable dwEIP3! 
		// it will be compiled [EBP+0C] but it is wrong address!
		MOV  EBX, dw_user_thread_entry
		MOV  ECX, [ESP+8]						//dwESP3
		POP  EDX								// POP RETURN_ADDR
		ADD  ESP,SIMULATE_CALL_PARAMETERS*4		// CLEAR 2 parameters
		
		//----------------------------------//
		XOR  EAX,EAX						//
		MOV  AX, ( GSEL_DATA32_R3 + 3 ) 	// RPL = 3
		PUSH EAX							// ** PUSH SS3
		AND  ECX,0xFFFFFFFC 				//
		SUB  ECX,4							//
		MOV  EAX,dw_real_thread_entry		//
		MOV  [ECX], EAX 					// insert reaal thread entry to r3 stack
		PUSH ECX							// ** PUSH ESP3 							  
											//
		XOR  EAX,EAX						//
		MOV  AX,  ( GSEL_CODE32_R3 + 3 )	// RPL = 3
		PUSH EAX							// ** PUSH CS3
		PUSH EBX							// ** PUSH EIP3
		//----------------------------------//

		PUSH EDX

		RETN
	}	
}

int call_r3_thread( void *pParam )
{
	ThreadStt	*pThread;
	ProcessStt	*pProcess;
	DWORD		dwESP3, *pD;

	pThread = get_current_thread();
	pProcess = pThread->pProcess;

	_asm MOV dwESP3, ESP;
	dwESP3 = dwESP3 - (KTHREAD_STACK_SIZE/2) - 8;

	pD = (DWORD*)dwESP3;
	pD[0] = pThread->dwR3EntryFunc;
	pD[1] = (DWORD) pParam;

	simulate_call( pProcess->e.func[1], dwESP3 );
	// ADD ESP, 8 will be appended but this is not needed.
	// so we must sub 8 from ESP !!
	_asm SUB ESP,SIMULATE_CALL_PARAMETERS*4

	// goto R3 Entry Point
	_asm RETF

	return( 0 );
}

// �ܼ��� "R0"���� "R3"�� �Ѿ�� ���� �Լ�.
int call_r3_function( void *pPram )
{
	ThreadStt	*pThread;
	DWORD		dwESP3;

	pThread = get_current_thread();

	_asm MOV dwESP3, ESP;
	simulate_call( pThread->dwR3EntryFunc, dwESP3 - (KTHREAD_STACK_SIZE/2) );
	// ADD ESP, 8 will be appended but this is not needed.
	// so we must sub 8 from ESP !!
	_asm SUB ESP,SIMULATE_CALL_PARAMETERS*4

	// goto R3 Entry Point
	_asm RETF

	return( 0 );
}

// ���ο� ���α׷��� �����Ѵ�.
// ���α׷� �󿡼� ����� ���μ����� �����尡 �̹� ����� ���´�.
// (���ο� ������ ���ؽ�Ʈ �� �����Ѵ�.)
static CritSectStt exec_new_cs;
int launch_r3_program( KExecveParamStt *pExecParam )
{
	ProcessStt	*pP;
	ThreadStt	*pT;
	ModuleStt	*pM;
	DWORD		dwESP3;
	int			nI, nReturn;

    // ENTER Critical Section
    enter_cs( &exec_new_cs );

	nReturn = 0;	
	
	// ������ �̸����� main thread�� alias��  �����Ѵ�.
	k_set_thread_alias( NULL, get_pure_filename( pExecParam->ppArgv[0] ) );

	// PE ������ �ε��Ѵ�. (���ġ���� �Ϸ�ȴ�.)
	pM = load_pe( pExecParam->ppArgv[0], MTYPE_APPS );
	if( pM == NULL )
	{
		nReturn = -1;		
		// ������ �ε����� ������ �� ������ �ɰ��� ����.
        kdbg_printf( "launch_r3_program: load_app() failed!\n" );
		_asm int 1;
	}

	// ���μ����� ����� ������ ��´�.
	pP = k_get_current_process();
	if( pP != NULL && pP->pMyDbg == NULL && pM->pMyDbg!= NULL )
	{
		pP->pMyDbg = get_coff_dbg2( (char*)pM->pMyDbg );
		//kdbg_printf( "set debug info(0x%08X)\n", (DWORD)pM->pMyDbg );
	}

	// ����� ���μ��� ī���͸� ������Ű�� ����� ���μ����� �Ҵ��Ѵ�.
	inc_process_count( pM );
	pP->pModule = pM;

	// KExecveParamStt�� �����Ѵ�.  (���⼭ �������� �ʰ� ���μ��� ���� �ÿ� �����Ѵ�.)
	//kfree( pExecParam );

    // LEAVE Critical Section
    leave_cs( &exec_new_cs );

	// R3 ������ �����Ѵ�.
	_asm MOV  dwESP3, ESP
	dwESP3 -= (KTHREAD_STACK_SIZE/2);
	dwESP3 &= (DWORD)0xFFFFF000;
	pT = get_current_thread();
	
	// 2003-10-22 (������ Reserve�ϸ鼭 R3 ���õ������ϵ��� ����.)
	//pT->pStkLink->dwR3StackTop =  dwESP3;  

	// R3�� ���ư���. (��ġ �ű⼭ CALL�Ǿ��� ��ó��)
	simulate_call( pM->dwEntryPoint, dwESP3 );
	
	// �����Ϸ��� "ADD ESP, 8" ����� �߰��� �������� �ʿ���� �κ��̴�.
	// �׷��� "SUB" ����� ���� ���̴�.
	_asm SUB ESP,SIMULATE_CALL_PARAMETERS*4

	// ����! R3 Entry Point��!
	_asm RETF

    // �Ʒ� �ڵ尡 ����Ǵ� ���� ����� ����.
	for( nI = 0;; )
	{
		kdbg_printf( "exec_new_program( %d )\r", nI++ );
		kernel_scheduler();
	}

//RELINQUISH:

	return( nReturn );		// return value
}

//static ForkParamStt global_fp;

// ���̻� ������� �ʴ´�. 2003-10-01
/*
// copy from global fork param variable
int set_global_fork_param( ForkParamStt *pFP )
{
	// semaphore protect ...

	memcpy( &global_fp, pFP, sizeof( ForkParamStt ) );
	

	// unlock semaphore
	
	return( 0 );
}

// copy pFP to global variable
int get_global_fork_param( ForkParamStt *pFP )
{
	// semaphore protect ...

	memcpy( pFP, &global_fp, sizeof( ForkParamStt ) );
	

	// unlock semaphore
	

	return( 0 );
}


// pPath�� ������ ������ �ε��ϰ� ���ο� ���μ����� �����Ͽ� �����Ѵ�.
ProcessStt *_fork_process( char *pPath, char *pArg, char *pEnv, FORK_FUNC pFunc, int nInitialState )
{
	int				nHandle;
	ThreadStt		*pCurThread;
	ForkParamStt	fp, ret_fp;

	// ������ �ִ��� Ȯ���� ����.
	nHandle = kopen( pPath, FM_READ );
	if( nHandle < 0 )
	{
		kdbg_printf( "fork_process: file not found!\n" );
		goto ERR_RETURN;
	}
	else
		kclose( nHandle );	
	
	// ���� �����尡 NULL�̸� fork�� �ȵȴ�.
	pCurThread = get_current_thread();
	if( pCurThread == NULL )
	{
		kdbg_printf( "fork_process: the current thread is null!\n" );
		goto ERR_RETURN;
	}

	memset( &fp, 0, sizeof( ForkParamStt ) );

	// Parent Thread�� �����Ѵ�.
	fp.pParentThread  = pCurThread;
	fp.dwStackSize	  = 0;             // ������ �˾Ƽ� �⺻ ũ��� �Ҵ�.
	fp.nInitialState  = nInitialState; // �ʱ� ����. (�Ƹ��� TSTATE_READY?)
	fp.pFunc		  = pFunc;
    // �ʿ��� ��Ʈ������ ����.
	if( pPath != NULL )	strcpy( fp.exec_param.szPath, pPath );
	if( pArg  != NULL )	strcpy( fp.exec_param.szArg,  pArg );
	if( pEnv  != NULL )	strcpy( fp.exec_param.szEnv,  pEnv );

	// ���� ������ ����.
	set_global_fork_param( &fp );

	// call init tss to create new process
	jmp_init_task( INIT_TASK_FORK_FUNC );		
	
	// get new process id from global_fp
	get_global_fork_param( &ret_fp );

	return( ret_fp.pNewProcess );

ERR_RETURN:
    
	return( NULL );
}
*/
