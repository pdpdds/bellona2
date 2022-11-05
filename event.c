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

	// 링크에 연결된 것이 없으면 새로 할당한다.
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

	// 아래 것은 절대 NULL로 만들면 안된다.
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

// WaitObj를 이벤트 구조체에서 테어낸다.
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

// 타이머 콜백함수를 설정한다.
void *set_timer_callback( DWORD dwTimerCount, TIMEOUT_CALLBACK pCB, DWORD dwParam )
{
	int			nR;
	TimeOutStt	*pTimeOut;

	_asm {
		PUSHFD
		CLI
	} 

	// 타임아웃 오브젝트를 할당한다.
	pTimeOut = alloc_timeout();
	if( pTimeOut == NULL )
		goto BACK;

	// set timeout fields
	pTimeOut->dwTick	 = dwTimerCount;
	pTimeOut->pCallBack  = pCB;
	pTimeOut->nReady     = 1;
	pTimeOut->byPeriodic = 1;	          // 타임아웃이 발생해도 제거하지 말것.
	pTimeOut->dwCallBackParam = dwParam;
	// add timeout to sys_timer list
	nR = add_to_timeout_list( pTimeOut ); // 항상 0을 리턴한다.

BACK:
	_asm POPFD

	return( pTimeOut );
}

// 타이머 콜백함수를 제거한다.
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

	// 빈 WaitObj를 할당받는다.
	pWaitObj = get_thread_empty_wait_obj( pThread );
	if( pWaitObj == NULL )
	{
		nReturn = -1;
		goto BACK;
	}

	pWaitObj->pThread   = pThread;
	pWaitObj->wWaitType = WAIT_TYPE_EVENT;
	
	// Wait Obj를 Event 구조체에 추가한다.
	nR = link_waitobj( pEvent, pWaitObj );	// 0-ok, 1-event already occurred
	if( nR < 0 )
	{
		nReturn = -1;
		goto BACK;
	}
	else if( nR == 1 )
	{	// TimeOut Object를 해제한다.
		if( pTimeOut != NULL )
			free_timeout( pTimeOut );

		// Wait Obj를 해제한다.
		set_wait_obj_free( pWaitObj );	// 이벤트가 이미 발생함.  대기할 필요가 없음.
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

// 커널 메시지를 대기한다.
// 대기하고 있는 메시지가 도착할 때까지 계속 Wait 상태로 남는다.
// 더이상 사용하지 않는다.
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

	// WaitObj를 할당한다.
	pWaitObj = get_thread_empty_wait_obj( pThread );
	if( pWaitObj == NULL )
	{
		nReturn = -1;
		goto BACK;
	}

	// WaitObj의 각 필드를 설정한다.
	pWaitObj->pThread          = pThread;
	pWaitObj->pTimeOut         = NULL;
	pWaitObj->wKMesg           = wKMesg;
	pWaitObj->dwKMesgParam     = 0;
	pWaitObj->nPrevThreadState = pThread->nState;

	// 쓰레드를 Wait 상태로 만든다.
	if( pThread->nState != TS_WAIT )
		nR = change_thread_state( NULL, pThread, TS_WAIT );		

	pThread->nWaitResult = 0;

	// 스케쥴러를 호출하여 다른 쓰레드를 스케쥴링한다.
	_asm STI;
	kernel_scheduler();

	if( pParam != NULL )
		pParam[0] = pWaitObj->dwKMesgParam;

	// Wait Obj를 FREE로 만든다.
	set_wait_obj_free( pWaitObj );	// 이벤트가 이미 발생함.  대기할 필요가 없음.

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
	{	// 타임아웃이 설정되어 있었으면 제거한다.		
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
		// 쓰레드가 WaitObj를 더이상 갖고 있지 않으면 이전 상태를 복원한다.
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

// 특정 이벤트에 연결된 Thread의 WaitObj를 찾는다.
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
	{	// 타임아웃 구조체를 타임아웃 핸들러의 링크에서 제거한다.
		sub_from_timeout_list( pTimeOut );
		free_timeout( pTimeOut );			
	}

	// WaitObj를 Event Link에서 제거한다.
	unlink_waitobj_from_event( pObj->pE, pObj );

	// WaitObj를 쓰레드에서 제거한다.
	unlink_waitobj_from_thread( pObj->pThread, pObj );

	// 어차피 해제할 것이므로 clear안해줘도 된다.
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
		// 쓰레드의 WaitObj 가운데 Key Event의 WaitObj를 찾는다.
		pWaitObj = find_thread_wait_obj( pThread, pEvent );
		if( pWaitObj != NULL )
		{
			nPrevState = pThread->nPrevThreadState;
	
			// WaitObj를 Free로 만든다.
			unlink_waitobj_from_event( pEvent, pWaitObj );
			nR = set_wait_obj_free( pWaitObj );

			// WaitObj가 더이상 없으면 Activate 시킨다.
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







