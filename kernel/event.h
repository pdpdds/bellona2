#ifndef BELLONA2_EVENT_HEADER_jj
#define BELLONA2_EVENT_HEADER_jj

struct ThreadTag;
struct EventTag;
struct TimeOutStt;

#define _SEMAPHORE_ERROR		-2
#define _TIME_OUT				-1
#define _EVENT_OCCURRED			1
#define _SEMAPHORE_RELEASED		2

typedef enum {
	SIMPLE_EVENT_WAITPID = 1, 
	END_OF_SIMPE_EVENT
} SIMPLE_EVENT_TAG;

// wait type  /////////////////////////////////////////////////////////
#define WAIT_TYPE_KMESG			1
#define WAIT_TYPE_EVENT			2
///////////////////////////////////////////////////////////////////////

// wait object
typedef struct WaitObjTag {
	UINT16				wWaitType;					// wait type
	UINT16				wWaitSubType;				// wait sub type
	struct ThreadTag	*pThread;					// owner thread
	struct EventTag		*pE;						// blocking event
	struct WaitObjTag	*pPreObj, *pNextObj;		// link from thread
	struct WaitObjTag	*pPreELink, *pNextELink;	// link from event
	struct TimeOutTag	*pTimeOut;					// pointer to timout structure
};
typedef struct WaitObjTag WaitObjStt;

// single event structure
typedef struct EventTag {
	char				szName[32];					// event name
	int					nCount;						// increase nCount whenever event occurs
	int					nTotalObj;					// total object linked to this event
	WaitObjStt			*pStartELink, *pEndELink;	// link to wait entry
	struct EventTag		*pNextEvent, *pPreEvent;	// pre next link
};
typedef struct EventTag EventStt;

// system event manager
typedef struct SystemEventTag{
	int				nTotalEvent;					// total registered event
	EventStt		*pStartEvent, *pEndEvent;		// event links
};
typedef struct SystemEventTag SystemEventStt;

////////////////////////////////////////////////////////////////////
extern EventStt	kbd_event;
extern EventStt	fdd_event;
extern EventStt	delay_event;
extern EventStt	signal_event;
extern EventStt	primary_ide_event;
extern EventStt	secondary_ide_event;
extern EventStt	com1_event, com2_event, com3_event, com4_event;
////////////////////////////////////////////////////////////////////
extern BELL_EXPORT int		kpause();
extern BELL_EXPORT int		close_event				( EventStt *pE );
extern BELL_EXPORT int		inc_event_count			( EventStt *pEvent );
extern BELL_EXPORT int		kill_timer_callback		( void *pTimeOut );
extern BELL_EXPORT int		clear_event_count		( EventStt *pEvent );
extern BELL_EXPORT int		wait_event				( EventStt *pEvent, DWORD dwTimeOut );
extern BELL_EXPORT void		init_event				( EventStt *pE, char *pName );
extern BELL_EXPORT void		*set_timer_callback		( DWORD dwTimerCount, TIMEOUT_CALLBACK pCB, DWORD dwParam );
extern BELL_EXPORT EventStt *create_event			( char *pName );
////////////////////////////////////////////////////////////////////
extern int			kdelay							( int nTick );
extern int			init_system_event				();
extern int			release_thread_wait_object		( ThreadStt *pThread );
extern int			set_wait_obj_free				( WaitObjStt *pWaitObj );
extern int			is_no_active_wait_obj			( struct ThreadTag *pThread );
extern int			register_event					( EventStt *pEvent, char *pName );
extern int			unlink_waitobj_from_event		( EventStt *pEvent, WaitObjStt *pObj );
extern int			awake_the_first_waiting_thread	( EventStt *pEvent, ThreadStt *pThread );
extern WaitObjStt	*get_thread_empty_wait_obj		( ThreadStt *pThread );
extern WaitObjStt	*find_thread_wait_obj			( ThreadStt *pThread, EventStt *pEvent );

#endif