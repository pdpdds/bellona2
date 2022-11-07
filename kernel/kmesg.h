#ifndef KMESG_HEADER_JJ
#define KMESG_HEADER_JJ

#define MAX_KMESG	16

typedef enum {
	KMESG_PROCESSED = 0,
	KMESG_ANY,
	KMESG_CHILD_PROCESS_EXIT,
	KMESG_CHILD_THREAD_EXIT,
	KMESG_KILL_PROCESS,
	KMESG_KILL_THREAD,
	
	TOTAL_KMESG
} KMESG_TAG;

typedef struct KMesgTag {
	UINT16		wType;
	DWORD		dwAParam;
	DWORD		dwBParam;
} KMesgStt;

typedef struct KMesgQTag
{
	UINT16		wTotalMesg, wStart, wEnd;
	KMesgStt	ent[ MAX_KMESG ];
} KMesgQStt;


extern int ksend_kmesg		( DWORD dwTID, UINT16 wType, DWORD dwAParam, DWORD dwBParam );
extern int kwait_kmesg		( UINT16 wType, DWORD *pAParam, DWORD *pBParam, DWORD dwTimeOut );


#endif
