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

// ���ο� Ÿ�Ӿƿ� ����ü�� �Ҵ��Ѵ�.
TimeOutStt *alloc_timeout()
{
	int				nI;
	TimeOutChunkStt	*pTC;

	// �� ������ ã�´�.
	for( pTC = sys_timer.pStartChunk; pTC != NULL; pTC = pTC->pNextChunk )
	{
		if( pTC->nTotalUsed < MAX_TIMEOUT_PER_CHUNK )
		{
			for( nI = 0; nI < MAX_TIMEOUT_PER_CHUNK; nI++ )
			{
				if( pTC->slot[nI].pWaitObj == NULL )
				{	// �ʱ�ȭ�� �� �����Ѵ�.
					memset( &pTC->slot[nI], 0, sizeof( TimeOutStt ) );
					pTC->slot[nI].pTimeOutChunk = pTC;
					return( &pTC->slot[nI] );
				}
			}
		}
	}

	// ���ο� Ÿ�Ӿƿ� ûũ�� �Ҵ��Ѵ�.
	if( pTC == NULL )
	{	
		pTC = (TimeOutChunkStt*)kmalloc( sizeof( TimeOutChunkStt ) );
		if( pTC == NULL )
			return( NULL );	// �Ҵ��� �� ����.	
		
		// 0�� ������ �ʱ�ȭ�� �� �����Ѵ�.
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

// WaitObject���� TimeOut ���θ� üũ�Ѵ�.
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

    // �׽�Ʈ �ڵ� 2002-12-20 ///////////////////////
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

	// Ÿ�Ӿƿ� ������Ʈ�� ó������ ������.
	for( pTO = sys_timer.pStartTimeOut; pTO != NULL; )
	{
		if( pTO->nReady == 0 || pTO->dwTick == 0 )
		{
			pTO = pTO->pNextTimeOut;
			continue;
		}

		// �� �����̸� �������� �Ѿ��.
		if( pTO->pWaitObj == NULL && pTO->pCallBack == NULL )
		{
			pTO = pTO->pNextTimeOut;
			continue;
		}

		//pTO->dwCurTick++;  (2002-12-17)
		//dwElapsedTick = ( 1000 / bell.dwTimerIntPerSecond );
        pTO->dwCurTick += dwElapsedTick;

		// Expire�� �Ϳ� ���� ó��.
		if( pTO->dwCurTick >= pTO->dwTick )
		{	
			pExpiredTO = pTO;
			pTO = pTO->pNextTimeOut;

			// Ÿ�� �ƿ� ������Ʈ�� ���� ����Ʈ���� �����Ѵ�.
			if( pExpiredTO->byPeriodic == 0 )
				nR = sub_from_timeout_list( pExpiredTO );

			// �ݹ��� �����Ǿ� ������ ȣ���Ѵ�.
			if( pExpiredTO->pCallBack != NULL )
				pExpiredTO->pCallBack( pExpiredTO->dwCallBackParam );

			// Wait Object�� ��ũ���� ���� �� �����带 �����.
			if( pExpiredTO->pWaitObj != NULL )
			{
				pWaitObj = pExpiredTO->pWaitObj;
			
				// �������� ���¸� ���� ������ �����Ѵ�.
				pThread = pWaitObj->pThread;
				if( pThread != NULL )
				{
					// set result with timeout
					pThread->nWaitResult = _TIME_OUT;
					
					// �����尡 ����ϰ� �ִ� Wait Object�� ���̻� �������� ������ ���¸� �����Ѵ�.
					if( is_no_active_wait_obj( pThread ) && pThread->nState == TS_WAIT )
					{					
						nPrevState = pThread->nPrevThreadState;
						// NULL - use default system schedule structure
						nR = change_thread_state( NULL, pThread, nPrevState );						}
				}

				// WaitObject�� ��ũ���� �и��Ͽ� Free��Ų��.
				if( pExpiredTO->byPeriodic == 0 )
				{
					nR = unlink_waitobj_from_event( pWaitObj->pE, pWaitObj );
					nR = set_wait_obj_free( pWaitObj );
				}
			}


			// Ÿ�Ӿƿ� ����ü�� �����Ѵ�.
			if( pExpiredTO->byPeriodic == 0 )
				nR = free_timeout( pExpiredTO );
			else
				pExpiredTO->dwCurTick = 0; // ƽ�� 0���� �����.

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
	
	// ���� Ÿ�̸Ӹ� ã�� ����.
	pGT = find_gui_timer( dwWinID, dwTimerID );
	if( pGT == NULL )
	{	// ���� �Ҵ��Ѵ�.
		pGT = (GuiTimerStt*)kmalloc( sizeof( GuiTimerStt ) );
		if( pGT == NULL )
		{	// �޸𸮸� �Ҵ��� �� ����.
			kdbg_printf( "register_gui_timer: kmalloc failed!\n" );
			return( NULL );
		}
		memset( pGT, 0, sizeof( GuiTimerStt ) );

		_asm {
			PUSHFD
			CLI
		}

		// ��ũ�� �������� �߰��Ѵ�.
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

	// �� �ʵ� ���� �����Ѵ�.
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
	
	// ������ ��ϵ� ������ ã�� ����.
	pGTimer = find_gui_timer( pGT->dwWinID, pGT->dwTimerID );
	if( pGTimer == NULL )
	{	// ������ ��Ͽ��� ã�� �� ����.
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

// GUI�� Timer Callout�� �˻��Ѵ�.
static int check_gui_timer_callout()
{	
	int				nR;
	__int64			cur_clk;
	GuiTimerStt 	*pGT, *pNext;
	
	// GUi Mode�� �ƴϸ� �׳� ���ư���.
	if( is_gui_mode() == 0 )
		return( -1 );	

	for( pGT = sys_gui_timer.pStart; pGT != NULL; pGT = pNext )
	{
		pNext = pGT->pNext;

		get_clk( &cur_clk );

		// ����Ǹ� �޽����� ������.
		if( pGT->due_clk < cur_clk )
		{
			pGT->due_clk = cur_clk + (__int64)( pGT->dwTick * get_rdtsc_per_millis() );

			// �޽����� ������.
			if( sys_gui_timer.pMesgPost == NULL )
				break;
			// gui_timer_mesg_post_func() �� ȣ��ȴ�.
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
		return( 1 );	//	Down �Ǿ���.
	}

	return( 0 );
}

static DWORD kt_flag = 1;
// Ÿ�̸� ���ͷ�Ʈ�� �߻��� ������ ȣ��ȴ�.
int ktimer_scheduler( DWORD dwESP )
{
	ThreadStt	*pThread;
	DWORD		*pESP, *pR3ESP;

	if( down_cond_value( &kt_flag ) < 0 )
		return( -1 );

	pESP = (DWORD*)dwESP;

	// �ý��� Tick�� ������Ų��.
	if( sys_timer.dwCurTick == (DWORD)0xFFFFFFFF )
		sys_timer.dwTickCarry++;
	
	sys_timer.dwCurTick++;

	// Wait Object ��� Ÿ�Ӿƿ� �ɸ� �͵��� ������ Wakeup ��Ų��.
	check_waitobj_timeout();

	// GUI�� Timer Callout�� �˻��Ѵ�.
	check_gui_timer_callout();
	
	// ���� �������� nice ī���͸� ������Ų��.
	pThread = get_current_thread();
	if( pThread != NULL && pThread->pProcess != NULL )
	{	
		down_schedule_level( pThread );

		// EIP�� R3 �������� Ȯ���Ѵ�.
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

// ������ ������ ���� �켱������ ���� Thread�� ��´�.
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
		if( nI >= TS_WAIT ) 	// Scheduling�� Thread�� �������� �ʴ´�.
		{
              _asm POPFD
			return( NULL );
		}
		
		if( pSch->q[nI].nTotal > 0 )
			break;
	}

	// ť���� ù ��° �����带 ������.
	pQ = &pSch->q[nI];
	pT = pQ->pStartThread;
	nPopThread( pT );

	// �����带 ť�� �������� �߰��Ѵ�.
	nPushThread( pQ, pT );

	_asm POPFD

	return( pT );
}

// CPU�� �ƹ��� ������ ���� �� idle�Լ��� Call�ȴ�.  (Schedule�� Thread�� �ϳ��� ����.)
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

// �־��� TSS�� �ּҸ� Dummy Task Descriptor�� ������ �� 
// FAR JMP�� �����Ѵ�.
// Current Thread�� ������� �ʴ´�.
void kernel_task_switching( ThreadStt *pThread )
{
	TSSStt *pTSS; 
    DWORD addr[2], dwDescAddr;

	pTSS = pThread->pTSS;
	pThread->dwCurNice = 0;	// 2004-02-18

	dwDescAddr = dwGetDescriptorAddr( &gdt[GSEL_DUMMY_TSS32/8] );
	if( dwDescAddr == (DWORD)pTSS )
	{	
		// ����Ī�� �ʿ䰡 ����.
		return;
	}

	// BUSY Bit�� 0���� ������ JMP�ؼ� �Űܰ� �� �ִ�.
	vSetDescriptorBusyBit( &gdt[GSEL_DUMMY_TSS32/8], 0 );
	if( pThread->dwID == dwPickThreadID )
	{
		kdbg_printf( "Picked thread( %d )\n", dwPickThreadID );
		_asm int 1
	}

	// ���� ������� �����Ѵ�.
	sch.pCurrentThread = pThread;

	//vMakeGDTDescriptor( GSEL_DUMMY_TSS32, (DWORD)pTSS, (DWORD)sizeof( TSSStt ), (UCHAR)0x89, (UCHAR)0x00 );
	dwSetDescriptorAddr  ( &gdt[GSEL_DUMMY_TSS32/8], (DWORD)pTSS );
    addr[0] = 0;						 // IP �κ��� ���õȴ�.
    addr[1] = (DWORD)GSEL_DUMMY_TSS32;   // �ٷ� TSS �����͸� ����.
	_asm JMP FWORD PTR addr;
}

static int up_schedule_level( ThreadStt *pThread )
{
	if( pThread == NULL )
		return( -1 );

	if( pThread->dwCurNice == 0 )
		return( 0 );	// ������ ��. �׳� ����.

	if( pThread->nState <= TS_READY_HIGHEST || pThread->dwNice <= DEFAULT_THREAD_NICE )
		return( 0 );	// �������� �� ����.

	pThread->dwNice--;
	if( ( pThread->dwNice % DEFAULT_THREAD_NICE ) == 0 )
	{
		change_thread_state( &sch, pThread, pThread->nState-1 );
		return( 1 );	// Up �Ǿ���.	
	}

	return( 0 );
}

// ������ ������� ��ȯ�Ѵ�.
// Current Thread�� ����ȴ�.
int kernel_thread_switching( ThreadStt *pThread )
{
	ThreadStt	*pCurrentThread;
	DWORD		dwKernelMappingFlag;

	up_schedule_level( pThread );
	
	// ���� �����带 �����ϴ� �κ��� kernel_task_switching �������� �ű�.
	// sch.pCurrentThread = pThread; // 2003-08-31

	// ã�Ƴ� Thread�� Task Seitching�� �����Ѵ�.  
    // ������ ���� CPL0�� ���̱� ������ JMP�� �̵��� ������.
	kernel_task_switching( pThread );	

	pCurrentThread = get_current_thread();

	// Ŀ�� ������ ������ �ٲ������ �����Ѵ�.
	dwKernelMappingFlag = get_kernel_mapping_flag();
	if( pCurrentThread->dwMappingFlag != dwKernelMappingFlag )
	{   // 2GB �Ʒ����� �����Ѵ�.
		memcpy( (DWORD*)get_thread_page_dir( pCurrentThread ), bell.pPD, 4096 / 2 );
		pCurrentThread->dwMappingFlag = dwKernelMappingFlag;
	}

	// �����ϱ� ������ Ŀ�� ��� �ñ׳� �ڵ鷯�� ó���Ѵ�.
	copy_thread_sig_bits();
	
	return( 0 );
}

// CPL 3���� �ٷ� kernel_scheduler()�� Call���� �ʰ� library function�� ���ļ� 
// system call������ ���ļ� CPL 0�� �Լ��� call�ϵ��� �Ѵ�.
int kernel_scheduler()
{
	ThreadStt	*pNextThread;

	for( ;; )
	{	// ������ ������ Thread�� ��´�.
		pNextThread = get_next_sch_thread( &sch );
		if( pNextThread != NULL )
			break;

		//sch.pCurrentThread = NULL;  2003-08-17 (���ʿ�)

		// IDLE �Լ��� �θ���.
		kernel_idle();
	}

	// ����� ������� ��ȯ�Ѵ�.
	kernel_thread_switching( pNextThread );

	return( 0 );
}














