#include "bellona2.h"

static SemaphoreManStt sema_man;

/*	Note: This instruction used to support semaphores

CMPXCHG	 - Compare and exchange
CPU:  i486+
Type of Instruction: User
Instruction: CMPXCHG dest,sorc
Description:
	Acc = if OperationSize(8)  -> AL
		 OperationSize(16) -> AX
		 OperationSize(32) -> EAX

	if( Acc = dest)
    {
	     ZF <- 1;
	     dest <- sorc;
	}
	ELSE
	{
	     ZF <- 0;
	     Acc <- dest;
	}
END
*/

// pDest[0]의 값이 0이면 에러 리턴, 1이며 0으로 바꾸고 0리턴.
int down_cond_value( DWORD *pDest )
{
	_asm {
		PUSH ESI
		MOV  EAX, 1
		XOR  EBX, EBX
		MOV  ESI, pDest
		CMPXCHG DWORD PTR [ESI], EBX	// *pDest == EAX (1)이면 pDest <- EBX (0) 
		POP  ESI
		JZ   DOWN_OK
	}

	return( -1 );

DOWN_OK:
	return( 0 );
}

// down_cond_value와는 반대로 초기 값이 0이면 1로 설정하고 0리턴.
// 초기 값이 1이면 -1리턴.
int init_cond_value( DWORD *pDest )
{
	_asm {
		PUSH ESI
			 MOV  EBX, 1
			 XOR  EAX, EAX
			 MOV  ESI, pDest
			 CMPXCHG DWORD PTR [ESI], EBX	// *pDest == EAX '0'이면 pDest <- EBX '1' 
		POP  ESI
		JZ   INIT_OK
	}

	return( -1 );

INIT_OK:
	return( 0 );
}

// find semaphore by name
static SemaphoreStt *find_semaphore_by_name( char *pS )
{
	SemaphoreStt *pSema;

	for( pSema = sema_man.pStart; pSema != NULL; pSema = pSema->pNext )
	{
		if( strcmp( pS, pSema->szName ) == 0 )
			return( pSema );
	}

	return( NULL );
}	

int init_system_semaphore()
{
	memset( &sema_man, 0, sizeof( sema_man ) );
	return( 0 );
}

// create new semaphore
SemaphoreStt *ksem_create( char *pName, int nInitCounter, DWORD dwAttrib )
{
	EventStt		*pE;
	SemaphoreStt	*pSema;
	char			szEventName[64];

	// 이름으로 세마포어를 찾아 본다.
	pSema = find_semaphore_by_name( pName );
	if( pSema != NULL )
		return( NULL );			// 이미 존재하면 생성할 수 없다.

	// 메모리를 할당한다.
	pSema = (SemaphoreStt*)kmalloc( sizeof( SemaphoreStt ) );
	if( pSema == NULL )
		return( NULL );			// 메모리를 할당할 수 없다.

	// 이름, 초기 값, 속성을 설정한다.
	memset( pSema, 0, sizeof( SemaphoreStt ) );
	strcpy( pSema->szName, pName );
	pSema->nValue		= nInitCounter;
	pSema->dwAttrib		= dwAttrib;
	pSema->nRefCounter	= 1;

	// 새로운 이벤트를 생성한다.
	sprintf( szEventName, "%s_event", pName );
	pE = create_event( szEventName );
	if( pE == NULL )
	{	// 이벤트를 생성할 수 없으면 에러.
		kfree( pSema );
		return( NULL );
	}

	pSema->pE = pE;

	// 세마포어를 시스템 구조체에 추가한다.
	if( sema_man.pStart == NULL )
	{
		sema_man.pStart = sema_man.pEnd = pSema;
	}
	else
	{	// 링크의 가장 끝에 추가한다.
		pSema->pPre				= sema_man.pEnd;
		sema_man.pEnd->pNext	= pSema;
		sema_man.pEnd			= pSema;
	}

	sema_man.nTotal++;
	
	return( pSema );
}

// 기존의 세마포어를 오픈한다.
SemaphoreStt *ksem_open( char *pName )
{
	SemaphoreStt *pSema;

	// 세마포어를 찾아 본다.
	pSema = find_semaphore_by_name( pName );
	if( pSema == NULL )
		return( NULL );			// 존재하지 않으면 에러.

	// Reference 값만 1증가 시킨다.
	pSema->nRefCounter++;
	
	return( pSema );
}

// close semaphore
int	ksem_close( SemaphoreStt *pSemaphore )
{
	// Reference 값을 1 줄인다.
	pSemaphore->nRefCounter--;
	if( pSemaphore->nRefCounter > 0 )
		return( 0 );

	_asm {
		PUSHFD
		CLI
	}

	// 세마포어를 시스테 구조체에서 떼어낸다.
	if( sema_man.nTotal > 0 )
	{
		if( pSemaphore->pPre != NULL )
			pSemaphore->pPre->pNext = pSemaphore->pNext;
		else
			sema_man.pStart = pSemaphore->pNext;
		if( pSemaphore->pNext != NULL )
			pSemaphore->pNext->pPre = pSemaphore->pPre;
		else
			sema_man.pEnd = pSemaphore->pPre;

		sema_man.nTotal--;
	}

	// 이벤트를 닫는다.
	close_event( pSemaphore->pE );

	// 세마포어를 해제한 후 리턴한다.
	kfree( pSemaphore );

	_asm POPFD;

	return( 0 );
}

// increase semaphore counter
int ksem_wait( SemaphoreStt *pSemaphore )
{
	int			nR;

	_asm {
		PUSHFD
		CLI
	}
	pSemaphore->nValue--;

	if( pSemaphore->nValue < 0 )
	{
		_asm POPFD
		nR = wait_event( pSemaphore->pE, 0 ); 
	}
	else
		_asm POPFD
		
	return( 1 );  
}


// decrease semaphore counter
int ksem_post( SemaphoreStt *pSemaphore, DWORD dwOption )
{
	int nR;

	_asm {
		PUSHFD
		CLI
	}
	
	pSemaphore->nValue++;

	if( pSemaphore->nValue <= 0 )
	{
		_asm POPFD
		nR = inc_event_count( pSemaphore->pE );
	}
	else
		_asm POPFD
		
	return( 1 ); 
}


