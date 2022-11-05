#include "bellona2.h"

// ���ϴ� kmesg�� ã�� ����.
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
		{	// ���ϴ� �޽����� ã�� �� ����.
			nI = -1;
			break;
		}		

		if( nI >= MAX_KMESG )
			 nI = 0;
		 
         if( pKMQ->ent[nI].wType == KMESG_PROCESSED )
		 	continue;  // �� ������ SKIP �Ѵ�.
		
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

	// kmsg�� ���� �����带 ã�´�.
	pT = find_thread_by_id( dwTID );
	if( pT == NULL )
	{	// �����带 ã�� �� ����.
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
		{	// �߰��� �� ������ ������ �������� �߰��Ѵ�.
			nI = pKMQ->wEnd;
			pKMQ->wEnd++;
			if( pKMQ->wEnd >= MAX_KMESG )
				pKMQ->wEnd = 0;
			break;
		}
		
		if( nI >= MAX_KMESG )
			nI = 0;
		
		if (pKMQ->ent[nI].wType == KMESG_PROCESSED )
		{	// wStart, wEnd ���̿� �� ������ ã�Ҵ�.
			break;
		}	
	}

	// Q�� �߰��Ѵ�.
	pKMQ->ent[nI].wType    = wType;
	pKMQ->ent[nI].dwAParam = dwAParam;
	pKMQ->ent[nI].dwBParam = dwBParam;
	
	// �޽��� ������ ���� ��Ų��.
	pKMQ->wTotalMesg++;

	_asm POPFD

	// �ش� �����尡 KMESG ��� ���¿����� �ƿ��ش�. (���� ���·� ����)
	if( pT->nState == TS_WAIT && pT->pStartWaitObj->wWaitType == WAIT_TYPE_KMESG )
	{
		if( pT->pStartWaitObj == NULL )
		{
			kdbg_printf( "kwait_kmesg: critical error.  waitobj is null!\n" );
			return( -1 );
		}

		// �������� ���¸� �ٲٸ� ����� TimeOut�� WaitObj�� FREE ��Ų��.
		pT->nWaitResult = _EVENT_OCCURRED;
		change_thread_state( NULL, pT, pT->nPrevThreadState );
	}

	return( 0 );
}

// kmesg ��� ���·� ����.
static int wait_for_kmesg( ThreadStt *pThread, UINT16 wType, DWORD dwTimeOut )
{
	int			nR, nReturn;
	WaitObjStt	*pWaitObj;
	TimeOutStt	*pTimeOut;

	nReturn = -1;

	// WaitObj�� �Ҵ��Ѵ�.
	pWaitObj = get_thread_empty_wait_obj( pThread );
	if( pWaitObj == NULL )
	{
		kdbg_printf( "wait_for_kmesg: wait obj failed!\n" );
		return( -1 );
	}
	
	// Ÿ�Ӿƿ��� �����Ѵ�.
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
		pTimeOut->byPeriodic  = 0;		// �� ���� �߻���ų ��.
		pTimeOut->pWaitObj	  = pWaitObj;
		// add timeout to sys_timer list
		add_to_timeout_list( pTimeOut ); // �׻� 0�� �����Ѵ�.
	}
	else
		pTimeOut = NULL;

	// WaitObj�� �� �ʵ带 �����Ѵ�.
	// WaitObj�� ��ũ������ �ʴ´�.
	pWaitObj->wWaitType        = WAIT_TYPE_KMESG;
	pWaitObj->wWaitSubType     = wType;
	pWaitObj->pThread          = pThread;
	pWaitObj->pTimeOut         = pTimeOut;
	pThread->nPrevThreadState  = pThread->nState;

	// �����带 Wait ���·� �����.
	nR = change_thread_state( NULL, pThread, TS_WAIT );		

	pThread->nWaitResult = 0;

	// �����췯�� ȣ���Ͽ� �ٸ� �����带 �����층�Ѵ�.
	_asm STI;
	kernel_scheduler();

	// TimeOut Object�� ���� �ɷ� �ִ°�?
	if( pWaitObj->pTimeOut != NULL )
	{	// TimeOut�� �����Ѵ�.
		sub_from_timeout_list( pWaitObj->pTimeOut );
		free_timeout( pWaitObj->pTimeOut );
		pWaitObj->pTimeOut = NULL;
	}

	// Wait Obj�� FREE�� �����.
	set_wait_obj_free( pWaitObj );

	nReturn = pThread->nWaitResult;
	
BACK:
	_asm POPFD;

	return( nReturn );
}

// KMESG�� �����⸦ ��ٸ���. �̹� ���� ������ �׳� �����Ѵ�.
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
	{	// �޽����� ���Դ��� ã�� ����.
		nI = find_kmesg( &pT->kmesgq, wType );
		if( nI >= 0 )
		    break;		// ���ϴ� �޽����� ���Դ�.
		    
		// ��� ���·� ����.
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
	// ���� ���� ��Ʈ�������� wStart ���� ������Ų��.
	if( nI == pT->kmesgq.wStart )
	{
		pT->kmesgq.wStart++;
		if( pT->kmesgq.wStart >= MAX_KMESG )
			pT->kmesgq.wStart = 0;
	}
	pT->kmesgq.wTotalMesg--;

	_asm POPFD
	
	// �޽��� Ÿ���� �����Ѵ�.
	return( wType );
}
