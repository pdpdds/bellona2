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

// �ý��� �������� ����ü�� �ʱ�ȭ �Ѵ�.
extern int init_system_semaphore();

// ���ο� ������� �����Ѵ�.
extern SemaphoreStt *ksem_create( char *pName, int nInitCounter, DWORD dwAttrib );

// ������ ������� �����Ѵ�.
extern SemaphoreStt *ksem_open( char *pName );

// ������� �ݴ´�.
extern int	ksem_close( SemaphoreStt *pSemaphore );

// ���� Ǭ��.
extern int ksem_post( SemaphoreStt *pSemaphore, DWORD dwOption );

// ���� �Ǵ�.
extern int ksem_wait( SemaphoreStt *pSemaphore );

extern BELL_EXPORT int down_cond_value( DWORD *pDest );
extern BELL_EXPORT int init_cond_value( DWORD *pDest );

#endif