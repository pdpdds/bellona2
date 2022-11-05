#include "bellona2.h"

static SystemTimerStt sys_timer;

// initialize scheduler and timeout structure
void kinit_scheduler()
{
	SchStt	*pSch;
	int		nI;

	pSch = &sch;
	memset( pSch, 0, sizeof( SchStt ) );

	// setting each q types
	for( nI = 0; nI < MAX_SCH_Q; nI++ )
		pSch->q[nI].nState = nI;

	// initialize timeout structure 
	memset( &sys_timer, 0, sizeof( sys_timer ) );
}

// 새로운 타임아웃 구조체를 할당한다.
TimeOutStt *alloc_timeout()
{
	int				nI;
	TimeOutChunkStt	*pTC;

	// 빈 슬롯을 찾는다.
	for( pTC = sys_timer.pStartChunk; pTC != NULL; pTC = pTC->pNextChunk )
	{
		if( pTC->nTotalUsed < MAX_TIMEOUT_PER_CHUNK )
		{
			for( nI = 0; nI < MAX_TIMEOUT_PER_CHUNK; nI++ )
			{
				if( pTC->slot[nI].pWaitObj == NULL )
				{	// 초기화한 후 리턴한다.
					memset( &pTC->slot[nI], 0, sizeof( TimeOutStt ) );
					pTC->slot[nI].pTimeOutChunk = pTC;
					return( &pTC->slot[nI] );
				}
			}
		}
	}

	// 새로운 타임아웃 청크를 할당한다.
	if( pTC == NULL )
	{	
		pTC = (TimeOutChunkStt*)kmalloc( sizeof( TimeOutChunkStt ) );
		if( pTC == NULL )
			return( NULL );	// 할당할 수 없다.	
		
		// 0번 슬롯을 초기화한 후 리턴한다.
		memset( pTC, 0, sizeof( TimeOutChunkStt ) );
		return( &pTC->slot[0] );
	}

	return( NULL );
}

// free timeout structure
int free_timeout( TimeOutStt *pTimeOut )
{
	if( pTimeOut == NULL || pTimeOut->pTimeOutChunk == NULL )
		return( -1 );

	pTimeOut->pTimeOutChunk->nTotalUsed--;

	memset( pTimeOut, 0, sizeof( TimeOutStt ) );

	return( 0 );
}

// add timeout structure to the timer handler
int add_to_timeout_list( TimeOutStt *pTimeOut )
{
	if( sys_timer.pEndTimeOut == NULL )
	{
		sys_timer.pStartTimeOut = sys_timer.pEndTimeOut = pTimeOut;
		pTimeOut->pPreTimeOut = pTimeOut->pNextTimeOut = NULL;
	}
	else
	{
		sys_timer.pEndTimeOut->pNextTimeOut = pTimeOut;
		pTimeOut->pPreTimeOut  = sys_timer.pEndTimeOut;
		pTimeOut->pNextTimeOut = NULL;
		sys_timer.pEndTimeOut  = pTimeOut;
	}

	sys_timer.nTotalTimeOut++;

	return( 0 );
}	

// sub timeout structure from the timer handler
int sub_from_timeout_list( TimeOutStt *pTimeOut )
{
	if( pTimeOut->pPreTimeOut == NULL )
		sys_timer.pStartTimeOut = pTimeOut->pNextTimeOut;
	else
		pTimeOut->pPreTimeOut->pNextTimeOut = pTimeOut->pNextTimeOut;

	if( pTimeOut->pNextTimeOut == NULL )
		sys_timer.pEndTimeOut = pTimeOut->pPreTimeOut;
	else
		pTimeOut->pNextTimeOut->pPreTimeOut = pTimeOut->pPreTimeOut;

	sys_timer.nTotalTimeOut--;

	return( 0 );
}	

// get current tick count
DWORD dwGetCurTick()
{
	return( sys_timer.dwCurTick );
}

static int nTickTack = 0;
static __int64	cur_tk = 0, prev_tk = 0;

// WaitObject들의 TimeOut 여부를 체크한다.
static int check_waitobj_timeout()
{
	int			nR;
	ThreadStt	*pThread;
	WaitObjStt	*pWaitObj;
	int			nPrevState;
	DWORD		dwElapsedTick;
	TimeOutStt	*pTO, *pExpiredTO;

	_asm {
		PUSHFD
		CLI
	}

    // 테스트 코드 2002-12-20 ///////////////////////
	/*
    nTickTack++;
    if( nTickTack == (int)bell.dwTimerIntPerSecond )
    {
        BYTE *pX = (BYTE*)0xB8000;
        nTickTack = 0;
        if( '0' <= pX[0] && pX[0] <= '9' ) 
			pX[0]++;
        else              
			pX[0] = '0';
    }*////////////////////////////////////////////////


    {
		get_clk( &cur_tk );
		if( prev_tk == 0 )
		{
			dwElapsedTick = ( 1000 / bell.dwTimerIntPerSecond );
		}
		else
		{
			dwElapsedTick = (DWORD)( (DWORD)(prev_tk - cur_tk) / get_rdtsc_per_millis() );
			prev_tk = cur_tk;
		}
    }	

	// 타임아웃 오브젝트의 처음부터 뒤진다.
	for( pTO = sys_timer.pStartTimeOut; pTO != NULL; )
	{
		if( pTO->nReady == 0 || pTO->dwTick == 0 )
		{
			pTO = pTO->pNextTimeOut;
			continue;
		}

		// 빈 슬롯이면 다음으로 넘어간다.
		if( pTO->pWaitObj == NULL && pTO->pCallBack == NULL )
		{
			pTO = pTO->pNextTimeOut;
			continue;
		}

		//pTO->dwCurTick++;  (2002-12-17)
		//dwElapsedTick = ( 1000 / bell.dwTimerIntPerSecond );
        pTO->dwCurTick += dwElapsedTick;

		// Expire된 것에 대한 처리.
		if( pTO->dwCurTick >= pTO->dwTick )
		{	
			pExpiredTO = pTO;
			pTO = pTO->pNextTimeOut;

			// 타임 아웃 오브젝트를 연결 리스트에서 제거한다.
			if( pExpiredTO->byPeriodic == 0 )
				nR = sub_from_timeout_list( pExpiredTO );

			// 콜백이 설정되어 있으면 호출한다.
			if( pExpiredTO->pCallBack != NULL )
				pExpiredTO->pCallBack( pExpiredTO->dwCallBackParam );

			// Wait Object를 링크에서 꺼낸 후 쓰레드를 깨운다.
			if( pExpiredTO->pWaitObj != NULL )
			{
				pWaitObj = pExpiredTO->pWaitObj;
			
				// 쓰레드의 상태를 이전 것으로 복원한다.
				pThread = pWaitObj->pThread;
				if( pThread != NULL )
				{
					// set result with timeout
					pThread->nWaitResult = _TIME_OUT;
					
					// 쓰레드가 대기하고 있는 Wait Object를 더이상 갖고있지 않으면 상태를 변경한다.
					if( is_no_active_wait_obj( pThread ) && pThread->nState == TS_WAIT )
					{					
						nPrevState = pThread->nPrevThreadState;
						// NULL - use default system schedule structure
						nR = change_thread_state( NULL, pThread, nPrevState );						}
				}

				// WaitObject를 링크에서 분리하여 Free시킨다.
				if( pExpiredTO->byPeriodic == 0 )
				{
					nR = unlink_waitobj_from_event( pWaitObj->pE, pWaitObj );
					nR = set_wait_obj_free( pWaitObj );
				}
			}


			// 타임아웃 구조체를 해제한다.
			if( pExpiredTO->byPeriodic == 0 )
				nR = free_timeout( pExpiredTO );
			else
				pExpiredTO->dwCurTick = 0; // 틱을 0으로 만든다.

		}	
		else
			pTO = pTO->pNextTimeOut;
	}	

	_asm POPFD;

	return( 0 );
}

static SysGuiTimerStt sys_gui_timer;

int init_gui_timer( GUI_MESG_POST_FUNC pFunc )
{
	memset( &sys_gui_timer, 0, sizeof( sys_gui_timer ) );
	
	sys_gui_timer.pMesgPost = pFunc;

	return( 0 );
}

GuiTimerStt *find_gui_timer( DWORD dwWinID, DWORD dwTimerID )
{
	GuiTimerStt *pGT, *pNext;

	for( pGT = sys_gui_timer.pStart; pGT != NULL; pGT = pNext ) 
	{
		pNext = pGT->pNext;

		if( pGT->dwWinID == dwWinID && pGT->dwTimerID == dwTimerID )
			return( pGT );
	}

	return( NULL );	
}

GuiTimerStt *register_gui_timer( DWORD dwWinID, DWORD dwTimerID, DWORD dwParamB, DWORD dwTick )
{
	__int64		cur_clk;
	GuiTimerStt *pGT;
	
	// 기존 타이머를 찾아 본다.
	pGT = find_gui_timer( dwWinID, dwTimerID );
	if( pGT == NULL )
	{	// 새로 할당한다.
		pGT = (GuiTimerStt*)kmalloc( sizeof( GuiTimerStt ) );
		if( pGT == NULL )
		{	// 메모리를 할당할 수 없다.
			kdbg_printf( "register_gui_timer: kmalloc failed!\n" );
			return( NULL );
		}
		memset( pGT, 0, sizeof( GuiTimerStt ) );

		_asm {
			PUSHFD
			CLI
		}

		// 링크의 마지막에 추가한다.
		if( sys_gui_timer.pStart == NULL )
		{
			sys_gui_timer.pStart = sys_gui_timer.pEnd = pGT;
		}
		else
		{
			sys_gui_timer.pEnd->pNext 	= pGT;
			pGT->pPre 					= sys_gui_timer.pEnd;
			sys_gui_timer.pEnd 			= pGT;
		}

		_asm POPFD
	}

	// 각 필드 값을 설정한다.
	pGT->dwParamB  = dwParamB;
	pGT->dwTimerID = dwTimerID;
	pGT->dwWinID   = dwWinID;
	pGT->dwTick    = dwTick;
	get_clk( &cur_clk );
	pGT->due_clk   = cur_clk + (__int64)( get_rdtsc_per_millis() * dwTick );
	
	return( pGT );
}

int unregister_gui_timer( GuiTimerStt *pGT )
{
	GuiTimerStt *pGTimer;
	
	// 기존에 등록된 것인지 찾아 본다.
	pGTimer = find_gui_timer( pGT->dwWinID, pGT->dwTimerID );
	if( pGTimer == NULL )
	{	// 기존의 등록에서 찾을 수 없다.
		kdbg_printf( "close_gui_timer: pGT not found!\n" );
		return( -1 );
	}

	//kdbg_printf( "unregister_gui_timer: dwWinID(%d), dwTimerID(%d)\n", pGT->dwWinID, pGT->dwTimerID );
	//kdbg_printf( "unregister_gui_timer: pre(0x%X), next(0x%X)\n", pGT->pPre, pGT->pNext );

	_asm {
		PUSHFD
		CLI
	}
	
	if( pGT->pPre == NULL )
		sys_gui_timer.pStart = pGT->pNext;
	else
		pGT->pPre->pNext = pGT->pNext;

	if( pGT->pNext == NULL )
		sys_gui_timer.pEnd = pGT->pPre;
	else
		pGT->pNext->pPre = pGT->pPre;

	pGT->pPre = pGT->pNext = NULL;

	//kdbg_printf( "sys_gui_timer.pStart=0x%X, pEnd = 0x%X\n", sys_gui_timer.pStart, sys_gui_timer.pEnd );

	_asm POPFD

	kfree( pGT );	// 2004-03-27

	return( 0 );
}

int close_all_gui_timer()
{
	GuiTimerStt 	*pGT, *pNext;

	for( pGT = sys_gui_timer.pStart; pGT != NULL; pGT = pNext )	
	{
		pNext = pGT->pNext;

		unregister_gui_timer( pGT );
	}

	sys_gui_timer.pMesgPost = NULL;
	
	return( 0 );
}

// GUI용 Timer Callout을 검사한다.
static int check_gui_timer_callout()
{	
	int				nR;
	__int64			cur_clk;
	GuiTimerStt 	*pGT, *pNext;
	
	// GUi Mode가 아니면 그냥 돌아간다.
	if( is_gui_mode() == 0 )
		return( -1 );	

	for( pGT = sys_gui_timer.pStart; pGT != NULL; pGT = pNext )
	{
		pNext = pGT->pNext;

		get_clk( &cur_clk );

		// 만료되면 메시지를 보낸다.
		if( pGT->due_clk < cur_clk )
		{
			pGT->due_clk = cur_clk + (__int64)( pGT->dwTick * get_rdtsc_per_millis() );

			// 메시지를 보낸다.
			if( sys_gui_timer.pMesgPost == NULL )
				break;
			// gui_timer_mesg_post_func() 가 호출된다.
			nR = sys_gui_timer.pMesgPost( pGT->dwWinID, pGT->dwTimerID, pGT->dwParamB );
		}		
	}
	return( 0 );
}

static int down_schedule_level( ThreadStt *pThread )
{
	if( pThread == NULL )
		return( -1 );
	if( pThread->nState >=TS_READY_LOWEST )
		return( 0 );

	pThread->dwCurNice++;
	if(  pThread->dwCurNice < pThread->dwNice )
		return( 0 );
	
	pThread->dwNice++;
	if( ( pThread->dwNice % DEFAULT_THREAD_NICE ) == 0 )
	{
		change_thread_state( &sch, pThread, pThread->nState+1 );
		return( 1 );	//	Down 되었다.
	}

	return( 0 );
}

static DWORD kt_flag = 1;
// 타이머 인터럽트가 발생할 때마다 호출된다.
int ktimer_scheduler( DWORD dwESP )
{
	ThreadStt	*pThread;
	DWORD		*pESP, *pR3ESP;

	if( down_cond_value( &kt_flag ) < 0 )
		return( -1 );

	pESP = (DWORD*)dwESP;

	// 시스템 Tick을 증가시킨다.
	if( sys_timer.dwCurTick == (DWORD)0xFFFFFFFF )
		sys_timer.dwTickCarry++;
	
	sys_timer.dwCurTick++;

	// Wait Object 가운데 타임아웃 걸린 것들이 있으면 Wakeup 시킨다.
	check_waitobj_timeout();

	// GUI용 Timer Callout을 검사한다.
	check_gui_timer_callout();
	
	// 현재 쓰레드의 nice 카운터를 증가시킨다.
	pThread = get_current_thread();
	if( pThread != NULL && pThread->pProcess != NULL )
	{	
		down_schedule_level( pThread );

		// EIP가 R3 영역인지 확인한다.
		if( ( pESP[0] > 0x80000000 ) )
		{
			if( pThread->dwCurNice >= pThread->dwNice && pThread->pProcess->e.func[0] != 0 )
			{	
				//kdbg_printf( "ktimer_scheduler : ret_addr = 0x%04X:0x%08X  ESP3 = 0x%08X\n", (unsigned short)pESP[1], pESP[0], pESP[3] );
				pThread->dwCurNice = 0;
				pESP[3]			  -= 4;
				pR3ESP			   = (DWORD*)pESP[3];
				pR3ESP[0]		   = pESP[0];
				pESP[0]            = (DWORD)pThread->pProcess->e.func[0];	// preempt function
			}  
		}
	}

	kt_flag = 1;

	return( 0 );
}

// 다음에 실행할 가장 우선순위가 높은 Thread를 얻는다.
static ThreadStt *get_next_sch_thread( SchStt *pSch )
{
	int			nI;
	ThreadStt	*pT;
	SQStt		*pQ;

	_asm {
		PUSHFD
		CLI
	}
	
	for( nI = 0; ; nI++ )
	{
		if( nI >= TS_WAIT ) 	// Scheduling할 Thread가 존재하지 않는다.
		{
              _asm POPFD
			return( NULL );
		}
		
		if( pSch->q[nI].nTotal > 0 )
			break;
	}

	// 큐에서 첫 번째 쓰레드를 꺼낸다.
	pQ = &pSch->q[nI];
	pT = pQ->pStartThread;
	nPopThread( pT );

	// 쓰레드를 큐의 마지막에 추가한다.
	nPushThread( pQ, pT );

	_asm POPFD

	return( pT );
}

// CPU가 아무런 할일이 없을 때 idle함수가 Call된다.  (Schedule할 Thread가 하나도 없다.)
static void kernel_idle()
{
	BYTE *pX = (BYTE*)(0xB8001 + 156);
	pX[0]++;
}

static DWORD dwPickThreadID = 0;
int set_pick_thread_id( DWORD dwTID )
{
	dwPickThreadID = dwTID;

	kdbg_printf( "pick thread id = %d\n", dwPickThreadID );

	return(0);
}

// 주어진 TSS의 주소를 Dummy Task Descriptor에 저장한 후 
// FAR JMP를 수행한다.
// Current Thread는 변경되지 않는다.
void kernel_task_switching( ThreadStt *pThread )
{
	TSSStt *pTSS; 
    DWORD addr[2], dwDescAddr;

	pTSS = pThread->pTSS;
	pThread->dwCurNice = 0;	// 2004-02-18

	dwDescAddr = dwGetDescriptorAddr( &gdt[GSEL_DUMMY_TSS32/8] );
	if( dwDescAddr == (DWORD)pTSS )
	{	
		// 스위칭할 필요가 없다.
		return;
	}

	// BUSY Bit를 0으로 만들어야 JMP해서 옮겨갈 수 있다.
	vSetDescriptorBusyBit( &gdt[GSEL_DUMMY_TSS32/8], 0 );
	if( pThread->dwID == dwPickThreadID )
	{
		kdbg_printf( "Picked thread( %d )\n", dwPickThreadID );
		_asm int 1
	}

	// 현재 쓰레드로 설정한다.
	sch.pCurrentThread = pThread;

	//vMakeGDTDescriptor( GSEL_DUMMY_TSS32, (DWORD)pTSS, (DWORD)sizeof( TSSStt ), (UCHAR)0x89, (UCHAR)0x00 );
	dwSetDescriptorAddr  ( &gdt[GSEL_DUMMY_TSS32/8], (DWORD)pTSS );
    addr[0] = 0;						 // IP 부분은 무시된다.
    addr[1] = (DWORD)GSEL_DUMMY_TSS32;   // 바로 TSS 셀렉터를 쓴다.
	_asm JMP FWORD PTR addr;
}

static int up_schedule_level( ThreadStt *pThread )
{
	if( pThread == NULL )
		return( -1 );

	if( pThread->dwCurNice == 0 )
		return( 0 );	// 선점된 것. 그냥 리턴.

	if( pThread->nState <= TS_READY_HIGHEST || pThread->dwNice <= DEFAULT_THREAD_NICE )
		return( 0 );	// 레벨업할 수 없다.

	pThread->dwNice--;
	if( ( pThread->dwNice % DEFAULT_THREAD_NICE ) == 0 )
	{
		change_thread_state( &sch, pThread, pThread->nState-1 );
		return( 1 );	// Up 되었다.	
	}

	return( 0 );
}

// 지정된 쓰레드로 전환한다.
// Current Thread가 변경된다.
int kernel_thread_switching( ThreadStt *pThread )
{
	ThreadStt	*pCurrentThread;
	DWORD		dwKernelMappingFlag;

	up_schedule_level( pThread );
	
	// 현재 쓰레드를 설정하는 부분을 kernel_task_switching 안쪽으로 옮김.
	// sch.pCurrentThread = pThread; // 2003-08-31

	// 찾아낸 Thread로 Task Seitching을 수행한다.  
    // 어차피 같은 CPL0일 것이기 때문에 JMP로 이동해 버린다.
	kernel_task_switching( pThread );	

	pCurrentThread = get_current_thread();

	// 커널 영역의 매핑이 바뀌었으면 복사한다.
	dwKernelMappingFlag = get_kernel_mapping_flag();
	if( pCurrentThread->dwMappingFlag != dwKernelMappingFlag )
	{   // 2GB 아래쪽을 복사한다.
		memcpy( (DWORD*)get_thread_page_dir( pCurrentThread ), bell.pPD, 4096 / 2 );
		pCurrentThread->dwMappingFlag = dwKernelMappingFlag;
	}

	// 리턴하기 직전에 커널 모드 시그널 핸들러를 처리한다.
	copy_thread_sig_bits();
	
	return( 0 );
}

// CPL 3에서 바로 kernel_scheduler()를 Call하지 않고 library function을 거쳐서 
// system call과정을 거쳐서 CPL 0의 함수를 call하도록 한다.
int kernel_scheduler()
{
	ThreadStt	*pNextThread;

	for( ;; )
	{	// 다음에 실행할 Thread를 얻는다.
		pNextThread = get_next_sch_thread( &sch );
		if( pNextThread != NULL )
			break;

		//sch.pCurrentThread = NULL;  2003-08-17 (불필요)

		// IDLE 함수를 부른다.
		kernel_idle();
	}

	// 얻어진 쓰레드로 전환한다.
	kernel_thread_switching( pNextThread );

	return( 0 );
}














