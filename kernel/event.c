#include "bellona2.h"

///////////////////////////////////////////
EventStt	delay_event;
EventStt	primary_ide_event;
EventStt	secondary_ide_event;
EventStt	kbd_event;
EventStt	fdd_event;
EventStt	com1_event, com2_event, com3_event, com4_event;
EventStt	signal_event;
///////////////////////////////////////////

typedef struct EventArrayTag {
	EventStt	*pE;
	char		*pName;
};
typedef struct EventArrayTag EventArrayEntStt;

static EventArrayEntStt	system_events[] = {
	{ &delay_event,				"delay"				},
	{ &kbd_event,				"Keyboard"			},
	{ &primary_ide_event,		"primary ide hdd"	},
	{ &secondary_ide_event,		"secondary ide hdd"	},
	{ &fdd_event,				"fdd 3.5"			},
	{ &com1_event,				"com port 1"		},
	{ &com2_event,				"com port 2"		},
	{ &com3_event,				"com port 3"		},
	{ &com4_event,				"com port 4"		},
	{ &signal_event,			"signal"			},

	{ NULL, NULL }
};

static SystemEventStt *pSysEvent, sys_event;

// init system event
int init_system_event()
{
	int nI;

	memset( &sys_event, 0, sizeof( sys_event ) );
	pSysEvent = &sys_event;

	// register events
	for( nI = 0; system_events[nI].pE != NULL; nI++ )
		register_event( system_events[nI].pE, system_events[nI].pName );

	return( 0 );
}	

// initialize event and register to system event manager
int register_event( EventStt *pEvent, char *pName )
{
	memset( pEvent, 0, sizeof( EventStt ) );

	strcpy( pEvent->szName, pName );

	if( pSysEvent->nTotalEvent == 0 )
	{	// set to the first event
		pSysEvent->pStartEvent = pSysEvent->pEndEvent = pEvent;
		pEvent->pPreEvent = pEvent->pNextEvent = NULL;
	}
	else
	{	// append to the end of the link
		pSysEvent->pEndEvent->pNextEvent	= pEvent;
		pEvent->pPreEvent					= pSysEvent->pEndEvent;
		pSysEvent->pEndEvent				= pEvent;
		pEvent->pNextEvent					= NULL;
	}

	pSysEvent->nTotalEvent++;
	
	return( 0 );
}

// get empty wait object of the thread
WaitObjStt *get_thread_empty_wait_obj( ThreadStt *pThread )
{
	WaitObjStt	*pO;

	if( pThread == NULL )
		return( NULL );

	for( pO = pThread->pStartWaitObj; pO != NULL; pO = pO->pNextObj )
	{
		if( pO->pThread == NULL )	// is it an empty object?
			goto RETURN;
	}

	// ��ũ�� ����� ���� ������ ���� �Ҵ��Ѵ�.
	pO = (WaitObjStt*)kmalloc( sizeof( WaitObjStt ) );
	if( pO == NULL )
		return( NULL );		// allocation failed!

	// clear with 0
	memset( pO, 0, sizeof( WaitObjStt ) );

	// insert new object to thread's wait object link
	if( pThread->pEndWaitObj != NULL )
	{
		pThread->pEndWaitObj->pNextObj = pO;
		pO->pPreObj = pThread->pEndWaitObj;
		pThread->pEndWaitObj = pO;
	}
	else
		pThread->pStartWaitObj = pThread->pEndWaitObj = pO;

	pThread->nTotalWaitObj++;

RETURN:
	// clear all other fields with 0
	pO->pThread		= pThread;
	pO->pE			= NULL;
	pO->pPreELink	= NULL;
	pO->pNextELink	= NULL;

	return( pO );
}

// set wait obj free
int set_wait_obj_free( WaitObjStt *pWaitObj )
{
	pWaitObj->pThread				= NULL;
	pWaitObj->pE    				= NULL;
	pWaitObj->pPreELink				= NULL;
	pWaitObj->pNextELink			= NULL;
	pWaitObj->pTimeOut				= NULL;

	// �Ʒ� ���� ���� NULL�� ����� �ȵȴ�.
	//pWaitObj->pPreObj, pWaitObj->pNextObj;

	return( 0 );
}

// link wait object to event structure
static link_waitobj( EventStt *pEvent, WaitObjStt *pObj )
{
	if( pEvent == NULL || pObj == NULL )
		return( -1 );		// linking error

	// event already occurred?
	if( pEvent->nCount > 0 )
	{
		pEvent->nCount--;
		return( 1 );	// event already occurred
	}

	pObj->pE = pEvent;

	if( pEvent->pEndELink == NULL )
	{
		pEvent->pStartELink = pEvent->pEndELink = pObj;
		pObj->pPreELink = pObj->pNextELink = NULL;
	}
	else
	{
		pEvent->pEndELink->pNextELink = pObj;
		pObj->pPreELink   = pEvent->pEndELink;
		pObj->pNextELink  = NULL;
		pEvent->pEndELink = pObj;
	}

	pEvent->nTotalObj++;

	return( 0 );		// successfully linked
}

// WaitObj�� �̺�Ʈ ����ü���� �׾��.
int unlink_waitobj_from_event( EventStt *pEvent, WaitObjStt *pObj )
{
	if( pEvent == NULL || pObj == NULL )
		return( -1 );

	if( pObj->pPreELink == NULL )
		pEvent->pStartELink = pObj->pNextELink;
	else
		pObj->pPreELink->pNextELink = pObj->pNextELink;

	if( pObj->pNextELink == NULL )
		pEvent->pEndELink = pObj->pPreELink;
	else
		pObj->pNextELink->pPreELink = pObj->pPreELink;

	pEvent->nTotalObj--;

	return( 0 );
}

// Ÿ�̸� �ݹ��Լ��� �����Ѵ�.
void *set_timer_callback( DWORD dwTimerCount, TIMEOUT_CALLBACK pCB, DWORD dwParam )
{
	int			nR;
	TimeOutStt	*pTimeOut;

	_asm {
		PUSHFD
		CLI
	} 

	// Ÿ�Ӿƿ� ������Ʈ�� �Ҵ��Ѵ�.
	pTimeOut = alloc_timeout();
	if( pTimeOut == NULL )
		goto BACK;

	// set timeout fields
	pTimeOut->dwTick	 = dwTimerCount;
	pTimeOut->pCallBack  = pCB;
	pTimeOut->nReady     = 1;
	pTimeOut->byPeriodic = 1;	          // Ÿ�Ӿƿ��� �߻��ص� �������� ����.
	pTimeOut->dwCallBackParam = dwParam;
	// add timeout to sys_timer list
	nR = add_to_timeout_list( pTimeOut ); // �׻� 0�� �����Ѵ�.

BACK:
	_asm POPFD

	return( pTimeOut );
}

// Ÿ�̸� �ݹ��Լ��� �����Ѵ�.
int kill_timer_callback( void *pTimeOut )
{
	int nR;

	_asm {
		PUSHFD
		CLI
	} 
	nR = sub_from_timeout_list( (TimeOutStt*)pTimeOut );
	nR = free_timeout( (TimeOutStt*)pTimeOut );
	_asm POPFD

	return( 0 );
}

// wait event
int wait_event( EventStt *pEvent, DWORD dwTimeOut )
{
	ThreadStt	*pThread;
	WaitObjStt	*pWaitObj;
	TimeOutStt	*pTimeOut;
	int			nR, nReturn;

	_asm {
		PUSHFD
		CLI
	} 

	pTimeOut = NULL;
	nReturn  = 0;

	// set Time out
	if( dwTimeOut > 0 )
	{	// allocate timeout structure
		pTimeOut = alloc_timeout();
		if( pTimeOut == NULL )
		{
			nReturn = -1;
			goto BACK;
		}
	}	
	
	pThread = get_current_thread();

	// �� WaitObj�� �Ҵ�޴´�.
	pWaitObj = get_thread_empty_wait_obj( pThread );
	if( pWaitObj == NULL )
	{
		nReturn = -1;
		goto BACK;
	}

	pWaitObj->pThread   = pThread;
	pWaitObj->wWaitType = WAIT_TYPE_EVENT;
	
	// Wait Obj�� Event ����ü�� �߰��Ѵ�.
	nR = link_waitobj( pEvent, pWaitObj );	// 0-ok, 1-event already occurred
	if( nR < 0 )
	{
		nReturn = -1;
		goto BACK;
	}
	else if( nR == 1 )
	{	// TimeOut Object�� �����Ѵ�.
		if( pTimeOut != NULL )
			free_timeout( pTimeOut );

		// Wait Obj�� �����Ѵ�.
		set_wait_obj_free( pWaitObj );	// �̺�Ʈ�� �̹� �߻���.  ����� �ʿ䰡 ����.
		goto BACK;
	}

	pWaitObj->pTimeOut = NULL;
	if( pTimeOut != NULL )
	{	
		// set timeout fields
		pTimeOut->dwTick	= dwTimeOut;
		pTimeOut->dwCurTick	= 0;
		pTimeOut->nReady	= 0;
		pTimeOut->pWaitObj	= pWaitObj;
		pTimeOut->nReady    = 1;
		pWaitObj->pTimeOut	= pTimeOut;
		// add timeout to sys_timer list
		add_to_timeout_list( pTimeOut );
	}

	// save thread's current state
	if( pThread->nState != TS_WAIT )
		pThread->nPrevThreadState = pThread->nState;

	// change thread state
	nR = change_thread_state( NULL, pThread, TS_WAIT );		// insert thread into wait q

	// set timeout to ready and switch to next thread
	pThread->nWaitResult = 0;

	_asm STI;
	kernel_scheduler();

	// check thread's nWait result
	if( pThread->nWaitResult != _EVENT_OCCURRED )
		nReturn = -1;		// maybe the timeout woke up the thread...
	
BACK:
	_asm POPFD;

	return( nReturn );
}

// Ŀ�� �޽����� ����Ѵ�.
// ����ϰ� �ִ� �޽����� ������ ������ ��� Wait ���·� ���´�.
// ���̻� ������� �ʴ´�.
/*  
int wait_kmesg( UINT16 wKMesg, DWORD *pParam )
{
	ThreadStt	*pThread;
	WaitObjStt	*pWaitObj;
	int			nR, nReturn;

	pThread = get_current_thread();
	if( pThread == NULL )
		return( -1 );

	nReturn  = 0;
	_asm {
		PUSHFD
		CLI
	} 

	// WaitObj�� �Ҵ��Ѵ�.
	pWaitObj = get_thread_empty_wait_obj( pThread );
	if( pWaitObj == NULL )
	{
		nReturn = -1;
		goto BACK;
	}

	// WaitObj�� �� �ʵ带 �����Ѵ�.
	pWaitObj->pThread          = pThread;
	pWaitObj->pTimeOut         = NULL;
	pWaitObj->wKMesg           = wKMesg;
	pWaitObj->dwKMesgParam     = 0;
	pWaitObj->nPrevThreadState = pThread->nState;

	// �����带 Wait ���·� �����.
	if( pThread->nState != TS_WAIT )
		nR = change_thread_state( NULL, pThread, TS_WAIT );		

	pThread->nWaitResult = 0;

	// �����췯�� ȣ���Ͽ� �ٸ� �����带 �����층�Ѵ�.
	_asm STI;
	kernel_scheduler();

	if( pParam != NULL )
		pParam[0] = pWaitObj->dwKMesgParam;

	// Wait Obj�� FREE�� �����.
	set_wait_obj_free( pWaitObj );	// �̺�Ʈ�� �̹� �߻���.  ����� �ʿ䰡 ����.

	nReturn = pThread->nWaitResult;
	
BACK:
	_asm POPFD;

	return( nReturn );
}
*/

// delay for nTick
int kdelay( int nTick )
{
	int nR;

	nR = wait_event( &delay_event, nTick );

	return( 0 );
}

// delay for signal forever
int kpause()
{
	int nR;

	nR = wait_event( &signal_event, -1 );

	return( 0 );
}

// clear event count
int clear_event_count( EventStt *pEvent )
{
	pEvent->nCount = 0;
	return(0);
}

// increase event count when the event occurs
int inc_event_count( EventStt *pEvent )
{
	ThreadStt	*pThread;
	TimeOutStt	*pTimeOut;
	WaitObjStt	*pWaitObj;
	int			nPrevState;
	int			nR, nReturn;

	_asm {
		PUSHFD
		CLI
	}

	nReturn = 0;
	if( pEvent->nTotalObj == 0 )
	{	// just increase counter and return
		pEvent->nCount++;
		nReturn = 1;
		goto BACK;
	}

	// unlink the first wait object
	pWaitObj = pEvent->pStartELink;
	nR = unlink_waitobj_from_event( pEvent, pWaitObj );
	if( nR < 0 )
	{
		nReturn = -1;
		goto BACK;
	}

	pTimeOut = pWaitObj->pTimeOut;
	if( pTimeOut != NULL )
	{	// Ÿ�Ӿƿ��� �����Ǿ� �־����� �����Ѵ�.		
		nR = sub_from_timeout_list( pTimeOut );
		nR = free_timeout( pTimeOut );
	}

	pThread = pWaitObj->pThread;
	nPrevState = pThread->nPrevThreadState;
	
	// set wait obj free
	nR = set_wait_obj_free( pWaitObj );

	if( pThread != NULL )
	{
		// set result with timeout
		pThread->nWaitResult = _EVENT_OCCURRED;
		// �����尡 WaitObj�� ���̻� ���� ���� ������ ���� ���¸� �����Ѵ�.
		if( is_no_active_wait_obj( pThread ) )
			nR = change_thread_state( NULL, pThread, nPrevState );	// NULL - use default system schedule structure
	}

BACK:
	_asm POPFD;

	return( nReturn );
}

// is there any active wait obj?
int is_no_active_wait_obj( ThreadStt *pThread )
{
	WaitObjStt	*pO;

	for( pO = pThread->pStartWaitObj; pO != NULL; pO = pO->pNextObj )
	{
		if( pO->pThread != NULL )
			return( 0 );		
	}

	return( 1 );	//	 this thread has no active wait obj
}

// Ư�� �̺�Ʈ�� ����� Thread�� WaitObj�� ã�´�.
WaitObjStt *find_thread_wait_obj( ThreadStt *pThread, EventStt *pEvent )
{
	WaitObjStt	*pWO;

	for( pWO = pThread->pStartWaitObj; pWO != NULL; pWO = pWO->pNextObj )
	{
		if( pWO->pE == pEvent )
			return( pWO );
	}					  

	return( NULL );
}

static int unlink_waitobj_from_thread( ThreadStt *pT, WaitObjStt *pObj )
{
	if( pObj->pPreObj == NULL )
		pT->pStartWaitObj = pObj->pNextObj;
	else
		pObj->pPreObj->pNextObj = pObj->pNextObj;

	if( pObj->pNextObj == NULL )
		pT->pEndWaitObj = pObj->pPreObj;
	else
		pObj->pNextObj->pPreObj = pObj->pPreObj;

	pT->nTotalWaitObj--;

	return( 0 );
}

static int destroy_waitobj( WaitObjStt *pObj )
{
	TimeOutStt	*pTimeOut;

	pTimeOut = pObj->pTimeOut;

	if( pTimeOut != NULL )
	{	// Ÿ�Ӿƿ� ����ü�� Ÿ�Ӿƿ� �ڵ鷯�� ��ũ���� �����Ѵ�.
		sub_from_timeout_list( pTimeOut );
		free_timeout( pTimeOut );			
	}

	// WaitObj�� Event Link���� �����Ѵ�.
	unlink_waitobj_from_event( pObj->pE, pObj );

	// WaitObj�� �����忡�� �����Ѵ�.
	unlink_waitobj_from_thread( pObj->pThread, pObj );

	// ������ ������ ���̹Ƿ� clear�����൵ �ȴ�.
	//	set_wait_obj_free( pObj );		// just clear 

	// release memory
	kfree( pObj );

	return( 0 );
}

// release all the objects which linked to the thread.
// return the total number of released objects.
int release_thread_wait_object( ThreadStt *pThread )
{
	int			nTotal;
	WaitObjStt	*pObj, *pNextObj;

	_asm {
		PUSHFD
		CLI
	}

	nTotal = 0;
	for( pObj = pThread->pStartWaitObj; pObj != NULL; pObj = pNextObj )
	{
		pNextObj = pObj->pNextObj;

		destroy_waitobj( pObj );

		nTotal++;
	}

	_asm POPFD;

	return( nTotal );
}

static ThreadStt *find_first_thread_in_event( EventStt *pEvent )
{
	WaitObjStt *pObj;

	if( pEvent == NULL || pEvent->pStartELink == NULL )
		return( NULL );

	pObj = pEvent->pStartELink;

	return( pObj->pThread );
}

int awake_the_first_waiting_thread( EventStt *pEvent, ThreadStt *pThread )
{
	int			nR;
	WaitObjStt	*pWaitObj;
	int			nPrevState;

	if( pThread == NULL )
	{
		pThread = find_first_thread_in_event( pEvent );
		if( pThread == NULL )
			return( -1 );	
	}
	
	if( pThread->nState == TS_WAIT )
	{	
		// �������� WaitObj ��� Key Event�� WaitObj�� ã�´�.
		pWaitObj = find_thread_wait_obj( pThread, pEvent );
		if( pWaitObj != NULL )
		{
			nPrevState = pThread->nPrevThreadState;
	
			// WaitObj�� Free�� �����.
			unlink_waitobj_from_event( pEvent, pWaitObj );
			nR = set_wait_obj_free( pWaitObj );

			// WaitObj�� ���̻� ������ Activate ��Ų��.
			if( is_no_active_wait_obj( pThread ) )
			{	// NULL - use default system schedule structure
				nR = change_thread_state( NULL, pThread, nPrevState );	
			}
		}
	}
	
	return( 0 );
}

typedef struct { 
	char		*pName;
	UINT16		wType;
} SimpleEventEntStt;

static SimpleEventEntStt se[] = {
	{ "WAITPID", SIMPLE_EVENT_WAITPID },
	{ NULL, 0 }
};

static char *get_simple_event_name( UINT16 wEventType )
{
	int nI;

	for( nI = 0; se[nI].pName != NULL; nI++ )
	{
		if( se[nI].wType == wEventType  )
			return( se[nI].pName );

	}

	return( 0 );
}

/*
char *get_event_name_of_wait_obj( char *pS, WaitObjStt *pWaitObj )
{
	pS[0] = 0;

	if( pWaitObj == NULL )
		return( NULL );

	if( pWaitObj->pE == NULL )
	{	
		if( pWaitObj->wKMesg & KMESG_CHILD_PROCESS_EXIT )
			strcat( pS, "CHILD_EXIT " );
		if( pWaitObj->wKMesg & KMESG_KILL_PROCESS )
			strcat( pS, "KILL_PROCESS " );
		if(  pWaitObj->wKMesg & KMESG_KILL_THREAD )
			strcat( pS, "KILL_THREAD " );
	}
	else
		strcpy( pS, pWaitObj->pE->szName );

	return( pS );
}
*/

void init_event( EventStt *pE, char *pName )
{
	memset( pE, 0, sizeof( EventStt ) );
	if( pName != NULL )
		strcpy( pE->szName, pName );
}

// create event
EventStt *create_event( char *pName )
{
	EventStt *pE;

	pE = (EventStt*)kmalloc( sizeof( EventStt ) );
	if( pE == NULL )
		return( NULL );

    init_event( pE, pName );

	// do not register!

	return( pE );
}

// close event
int close_event( EventStt *pE )
{
	ThreadStt		*pThread;
	WaitObjStt		*pW, *pNextW;
	int				nPrevState, nR;

	if( pE == NULL )
		return( -1 );

	// release all wait objects
	for( pNextW = pE->pStartELink; pNextW != NULL; )
	{
		pW = pNextW;
		pNextW = pW->pNextELink;
	
		pThread		= pW->pThread;
		nPrevState	= pThread->nPrevThreadState;
	
		// set wait obj free
		nR = set_wait_obj_free( pW );

		if( pThread != NULL )
		{
			pThread->nWaitResult = _SEMAPHORE_ERROR;
			// if the thread has no active wait obj then change its state to ready.
			if( is_no_active_wait_obj( pThread ) )
				nR = change_thread_state( NULL, pThread, nPrevState );	
		}		
	}
	
	// free structure
	kfree( pE );

	return( 0 );
}







