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

// pDest[0]�� ���� 0�̸� ���� ����, 1�̸� 0���� �ٲٰ� 0����.
int down_cond_value( DWORD *pDest )
{
	_asm {
		PUSH ESI
		MOV  EAX, 1
		XOR  EBX, EBX
		MOV  ESI, pDest
		CMPXCHG DWORD PTR [ESI], EBX	// *pDest == EAX (1)�̸� pDest <- EBX (0) 
		POP  ESI
		JZ   DOWN_OK
	}

	return( -1 );

DOWN_OK:
	return( 0 );
}

// down_cond_value�ʹ� �ݴ�� �ʱ� ���� 0�̸� 1�� �����ϰ� 0����.
// �ʱ� ���� 1�̸� -1����.
int init_cond_value( DWORD *pDest )
{
	_asm {
		PUSH ESI
			 MOV  EBX, 1
			 XOR  EAX, EAX
			 MOV  ESI, pDest
			 CMPXCHG DWORD PTR [ESI], EBX	// *pDest == EAX '0'�̸� pDest <- EBX '1' 
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

	// �̸����� ������� ã�� ����.
	pSema = find_semaphore_by_name( pName );
	if( pSema != NULL )
		return( NULL );			// �̹� �����ϸ� ������ �� ����.

	// �޸𸮸� �Ҵ��Ѵ�.
	pSema = (SemaphoreStt*)kmalloc( sizeof( SemaphoreStt ) );
	if( pSema == NULL )
		return( NULL );			// �޸𸮸� �Ҵ��� �� ����.

	// �̸�, �ʱ� ��, �Ӽ��� �����Ѵ�.
	memset( pSema, 0, sizeof( SemaphoreStt ) );
	strcpy( pSema->szName, pName );
	pSema->nValue		= nInitCounter;
	pSema->dwAttrib		= dwAttrib;
	pSema->nRefCounter	= 1;

	// ���ο� �̺�Ʈ�� �����Ѵ�.
	sprintf( szEventName, "%s_event", pName );
	pE = create_event( szEventName );
	if( pE == NULL )
	{	// �̺�Ʈ�� ������ �� ������ ����.
		kfree( pSema );
		return( NULL );
	}

	pSema->pE = pE;

	// ������� �ý��� ����ü�� �߰��Ѵ�.
	if( sema_man.pStart == NULL )
	{
		sema_man.pStart = sema_man.pEnd = pSema;
	}
	else
	{	// ��ũ�� ���� ���� �߰��Ѵ�.
		pSema->pPre				= sema_man.pEnd;
		sema_man.pEnd->pNext	= pSema;
		sema_man.pEnd			= pSema;
	}

	sema_man.nTotal++;
	
	return( pSema );
}

// ������ ������� �����Ѵ�.
SemaphoreStt *ksem_open( char *pName )
{
	SemaphoreStt *pSema;

	// ������� ã�� ����.
	pSema = find_semaphore_by_name( pName );
	if( pSema == NULL )
		return( NULL );			// �������� ������ ����.

	// Reference ���� 1���� ��Ų��.
	pSema->nRefCounter++;
	
	return( pSema );
}

// close semaphore
int	ksem_close( SemaphoreStt *pSemaphore )
{
	// Reference ���� 1 ���δ�.
	pSemaphore->nRefCounter--;
	if( pSemaphore->nRefCounter > 0 )
		return( 0 );

	_asm {
		PUSHFD
		CLI
	}

	// ������� �ý��� ����ü���� �����.
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

	// �̺�Ʈ�� �ݴ´�.
	close_event( pSemaphore->pE );

	// ������� ������ �� �����Ѵ�.
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


