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

// 단순히 "R0"에서 "R3"로 넘어가기 위한 함수.
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

// 새로운 프로그램을 실행한다.
// 프로그램 상에서 실행될 프로세스와 쓰레드가 이미 실행된 상태다.
// (새로운 쓰레드 컨텍스트 상에 존재한다.)
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
	
	// 파일의 이름으로 main thread의 alias를  설정한다.
	k_set_thread_alias( NULL, get_pure_filename( pExecParam->ppArgv[0] ) );

	// PE 파일을 로드한다. (재배치까지 완료된다.)
	pM = load_pe( pExecParam->ppArgv[0], MTYPE_APPS );
	if( pM == NULL )
	{
		nReturn = -1;		
		// 파일을 로드하지 못했을 때 문제가 심각해 진다.
        kdbg_printf( "launch_r3_program: load_app() failed!\n" );
		_asm int 1;
	}

	// 프로세스의 디버그 정보를 얻는다.
	pP = k_get_current_process();
	if( pP != NULL && pP->pMyDbg == NULL && pM->pMyDbg!= NULL )
	{
		pP->pMyDbg = get_coff_dbg2( (char*)pM->pMyDbg );
		//kdbg_printf( "set debug info(0x%08X)\n", (DWORD)pM->pMyDbg );
	}

	// 모듈의 프로세스 카운터를 증가시키고 모듈을 프로세스에 할당한다.
	inc_process_count( pM );
	pP->pModule = pM;

	// KExecveParamStt를 해제한다.  (여기서 해제하지 않고 프로세스 종료 시에 해제한다.)
	//kfree( pExecParam );

    // LEAVE Critical Section
    leave_cs( &exec_new_cs );

	// R3 스택을 설정한다.
	_asm MOV  dwESP3, ESP
	dwESP3 -= (KTHREAD_STACK_SIZE/2);
	dwESP3 &= (DWORD)0xFFFFF000;
	pT = get_current_thread();
	
	// 2003-10-22 (스택을 Reserve하면서 R3 스택도설정하도록 수정.)
	//pT->pStkLink->dwR3StackTop =  dwESP3;  

	// R3로 돌아간다. (마치 거기서 CALL되었던 것처럼)
	simulate_call( pM->dwEntryPoint, dwESP3 );
	
	// 컴파일러가 "ADD ESP, 8" 명령을 추가할 것이지만 필요없는 부분이다.
	// 그래서 "SUB" 명령을 넣은 것이다.
	_asm SUB ESP,SIMULATE_CALL_PARAMETERS*4

	// 가자! R3 Entry Point로!
	_asm RETF

    // 아래 코드가 수행되는 일은 절대로 없다.
	for( nI = 0;; )
	{
		kdbg_printf( "exec_new_program( %d )\r", nI++ );
		kernel_scheduler();
	}

//RELINQUISH:

	return( nReturn );		// return value
}

//static ForkParamStt global_fp;

// 더이상 사용하지 않는다. 2003-10-01
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


// pPath에 지정된 파일을 로드하고 새로운 프로세스를 생성하여 실행한다.
ProcessStt *_fork_process( char *pPath, char *pArg, char *pEnv, FORK_FUNC pFunc, int nInitialState )
{
	int				nHandle;
	ThreadStt		*pCurThread;
	ForkParamStt	fp, ret_fp;

	// 파일이 있는지 확인해 본다.
	nHandle = kopen( pPath, FM_READ );
	if( nHandle < 0 )
	{
		kdbg_printf( "fork_process: file not found!\n" );
		goto ERR_RETURN;
	}
	else
		kclose( nHandle );	
	
	// 현재 쓰레드가 NULL이면 fork가 안된다.
	pCurThread = get_current_thread();
	if( pCurThread == NULL )
	{
		kdbg_printf( "fork_process: the current thread is null!\n" );
		goto ERR_RETURN;
	}

	memset( &fp, 0, sizeof( ForkParamStt ) );

	// Parent Thread를 설정한다.
	fp.pParentThread  = pCurThread;
	fp.dwStackSize	  = 0;             // 스택은 알아서 기본 크기로 할당.
	fp.nInitialState  = nInitialState; // 초기 상태. (아마도 TSTATE_READY?)
	fp.pFunc		  = pFunc;
    // 필요한 스트링들을 복사.
	if( pPath != NULL )	strcpy( fp.exec_param.szPath, pPath );
	if( pArg  != NULL )	strcpy( fp.exec_param.szArg,  pArg );
	if( pEnv  != NULL )	strcpy( fp.exec_param.szEnv,  pEnv );

	// 전역 변수에 복사.
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
