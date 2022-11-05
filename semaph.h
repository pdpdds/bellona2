#ifndef BELLONA2_SEMAPHORE_HEADER_jj
#define BELLONA2_SEMAPHORE_HEADER_jj

struct EventTag;

struct SemaphoreTag {
	char					szName[32];						// semaphore name
	int						nValue;							// current semaphore value
	int						nRefCounter;					// reference counter
	DWORD					dwTID;							// owner thread id
	DWORD					dwAttrib;						// semaphore attribute for future use
	struct SemaphoreTag		*pPre, *pNext;					// semaphore link from SemaphoreManStt
	struct EventTag			*pE;							// event pointer
};
typedef struct SemaphoreTag SemaphoreStt;

typedef struct {
	int				nTotal;									// ttal semaphore
	SemaphoreStt	*pStart, *pEnd;							// link to semaphores
} SemaphoreManStt;

// 시스템 세마포어 구조체를 초기화 한다.
extern int init_system_semaphore();

// 새로운 세마포어를 생성한다.
extern SemaphoreStt *ksem_create( char *pName, int nInitCounter, DWORD dwAttrib );

// 기존의 세마포어를 오픈한다.
extern SemaphoreStt *ksem_open( char *pName );

// 세마포어를 닫는다.
extern int	ksem_close( SemaphoreStt *pSemaphore );

// 락을 푼다.
extern int ksem_post( SemaphoreStt *pSemaphore, DWORD dwOption );

// 락을 건다.
extern int ksem_wait( SemaphoreStt *pSemaphore );

extern BELL_EXPORT int down_cond_value( DWORD *pDest );
extern BELL_EXPORT int init_cond_value( DWORD *pDest );

#endif