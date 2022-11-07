#include "bellona2.h"

// 원하는 kmesg를 찾아 본다.
static int find_kmesg( KMesgQStt *pKMQ, UINT16 wType )
{
	int nI, nK, nR;
	
	_asm {
		PUSHFD
		CLI
	}
	
	nR = -1;  
	for( nK = 0, nI = pKMQ->wStart; ; nI++, nK++ )
	{
		if( nK >= MAX_KMESG )
		{	// 원하는 메시지를 찾을 수 없다.
			nI = -1;
			break;
		}		

		if( nI >= MAX_KMESG )
			 nI = 0;
		 
         if( pKMQ->ent[nI].wType == KMESG_PROCESSED )
		 	continue;  // 빈 슬롯은 SKIP 한다.
		
		if( wType == KMESG_ANY )
		    break;
		else if (pKMQ->ent[nI].wType == wType )
		    break;    
	}
	
	_asm POPFD
	
	return( nI );
}

int ksend_kmesg( DWORD dwTID, UINT16 wType, DWORD dwAParam, DWORD dwBParam )
{
	int			nI;
	ThreadStt	*pT;
	KMesgQStt	*pKMQ;

	// kmsg를 받을 쓰레드를 찾는다.
	pT = find_thread_by_id( dwTID );
	if( pT == NULL )
	{	// 쓰레드를 찾을 수 없다.
		kdbg_printf( "ksend_kmesg: TID(%d) not found!\n", dwTID );
		return( -1 );
	}

	pKMQ = &pT->kmesgq;
	if( pKMQ->wTotalMesg >= MAX_KMESG )
	{
		kdbg_printf( "ksend_kmesg: TID(%d) no empty slot!\n", dwTID );
		return( -1 );
	}

	_asm {
		PUSHFD
		CLI
	}

	for( nI = pKMQ->wStart; ; nI++ )
	{
		if( nI == pKMQ->wEnd )
		{	// 중간에 빈 슬롯이 없으면 마지막에 추가한다.
			nI = pKMQ->wEnd;
			pKMQ->wEnd++;
			if( pKMQ->wEnd >= MAX_KMESG )
				pKMQ->wEnd = 0;
			break;
		}
		
		if( nI >= MAX_KMESG )
			nI = 0;
		
		if (pKMQ->ent[nI].wType == KMESG_PROCESSED )
		{	// wStart, wEnd 사이에 빈 슬롯을 찾았다.
			break;
		}	
	}

	// Q에 추가한다.
	pKMQ->ent[nI].wType    = wType;
	pKMQ->ent[nI].dwAParam = dwAParam;
	pKMQ->ent[nI].dwBParam = dwBParam;
	
	// 메시지 개수를 증가 시킨다.
	pKMQ->wTotalMesg++;

	_asm POPFD

	// 해당 쓰레드가 KMESG 대기 상태였으면 꺠워준다. (이전 상태로 복원)
	if( pT->nState == TS_WAIT && pT->pStartWaitObj->wWaitType == WAIT_TYPE_KMESG )
	{
		if( pT->pStartWaitObj == NULL )
		{
			kdbg_printf( "kwait_kmesg: critical error.  waitobj is null!\n" );
			return( -1 );
		}

		// 쓰레드의 상태만 바꾸면 깨어나서 TimeOut과 WaitObj를 FREE 시킨다.
		pT->nWaitResult = _EVENT_OCCURRED;
		change_thread_state( NULL, pT, pT->nPrevThreadState );
	}

	return( 0 );
}

// kmesg 대기 상태로 들어간다.
static int wait_for_kmesg( ThreadStt *pThread, UINT16 wType, DWORD dwTimeOut )
{
	int			nR, nReturn;
	WaitObjStt	*pWaitObj;
	TimeOutStt	*pTimeOut;

	nReturn = -1;

	// WaitObj를 할당한다.
	pWaitObj = get_thread_empty_wait_obj( pThread );
	if( pWaitObj == NULL )
	{
		kdbg_printf( "wait_for_kmesg: wait obj failed!\n" );
		return( -1 );
	}
	
	// 타임아웃을 설정한다.
	if( dwTimeOut > 0 )
	{
		pTimeOut = alloc_timeout();
		if( pTimeOut == NULL )
		{
			kdbg_printf( "wait_for_kmesg: timeout obj failed!\n" );
			goto BACK;
		}
		// set timeout fields
		pTimeOut->dwTick	  = dwTimeOut;
		pTimeOut->pCallBack   = NULL;
		pTimeOut->nReady      = 1;
		pTimeOut->byPeriodic  = 0;		// 한 번만 발생시킬 것.
		pTimeOut->pWaitObj	  = pWaitObj;
		// add timeout to sys_timer list
		add_to_timeout_list( pTimeOut ); // 항상 0을 리턴한다.
	}
	else
		pTimeOut = NULL;

	// WaitObj의 각 필드를 설정한다.
	// WaitObj를 링크걸지는 않는다.
	pWaitObj->wWaitType        = WAIT_TYPE_KMESG;
	pWaitObj->wWaitSubType     = wType;
	pWaitObj->pThread          = pThread;
	pWaitObj->pTimeOut         = pTimeOut;
	pThread->nPrevThreadState  = pThread->nState;

	// 쓰레드를 Wait 상태로 만든다.
	nR = change_thread_state( NULL, pThread, TS_WAIT );		

	pThread->nWaitResult = 0;

	// 스케쥴러를 호출하여 다른 쓰레드를 스케쥴링한다.
	_asm STI;
	kernel_scheduler();

	// TimeOut Object가 아직 걸려 있는가?
	if( pWaitObj->pTimeOut != NULL )
	{	// TimeOut을 해제한다.
		sub_from_timeout_list( pWaitObj->pTimeOut );
		free_timeout( pWaitObj->pTimeOut );
		pWaitObj->pTimeOut = NULL;
	}

	// Wait Obj를 FREE로 만든다.
	set_wait_obj_free( pWaitObj );

	nReturn = pThread->nWaitResult;
	
BACK:
	_asm POPFD;

	return( nReturn );
}

// KMESG가 들어오기를 기다린다. 이미 들어와 있으면 그냥 리턴한다.
int kwait_kmesg( UINT16 wType, DWORD *pAParam, DWORD *pBParam, DWORD dwTimeOut )
{
	ThreadStt 	*pT;
	KMesgStt	*pKM;
	int			nI, nR;
	
	pT = get_current_thread();
	if( pT == NULL )
	{
		kdbg_printf( "kwait_kmesg: current thread is NULL!\n" );
		return( -1 );
	}
		
	for( ;; )
	{	// 메시지가 들어왔는지 찾아 본다.
		nI = find_kmesg( &pT->kmesgq, wType );
		if( nI >= 0 )
		    break;		// 원하는 메시지가 들어왔다.
		    
		// 대기 상태로 들어간다.
		nR = wait_for_kmesg( pT, wType, dwTimeOut ); 
		if( nR < 0 )
			return( -1 );
	}
	
	pKM = &pT->kmesgq.ent[nI];
	if( pAParam != NULL )
		pAParam[0] = pKM->dwAParam;	
	if( pBParam != NULL )
		pBParam[0] = pKM->dwBParam;	
		
	wType = pKM->wType;			
	
	_asm {
		PUSHFD
		CLI
	}
	pKM->wType = KMESG_PROCESSED;		
	// 가장 앞쪽 엔트리였으면 wStart 값을 증가시킨다.
	if( nI == pT->kmesgq.wStart )
	{
		pT->kmesgq.wStart++;
		if( pT->kmesgq.wStart >= MAX_KMESG )
			pT->kmesgq.wStart = 0;
	}
	pT->kmesgq.wTotalMesg--;

	_asm POPFD
	
	// 메시지 타입을 리턴한다.
	return( wType );
}
