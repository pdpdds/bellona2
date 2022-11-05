#include "lib.h"

DWORD sem_create( char *pName, int nMaxCounter, DWORD dwAttrib )
{
	DWORD dwSem;
	
	dwSem = (DWORD)syscall_stub( SCTYPE_CREATE_SEMAPHORE, pName, nMaxCounter, dwAttrib );

	return( dwSem );
}

DWORD sem_open( char *pName )
{
	DWORD dwSem;
	
	dwSem = syscall_stub( SCTYPE_OPEN_SEMAPHORE, (DWORD)pName );
	
	return( dwSem );
}

int	sem_close( DWORD dwSem )
{
	int nR;

	nR = syscall_stub( SCTYPE_CLOSE_SEMAPHORE, dwSem );

	return( nR );
}

int sem_post( DWORD dwSem, DWORD dwOption )
{
	int    nR;

	nR = syscall_stub( SCTYPE_INC_SEMAPHORE, dwSem, dwOption );

	return( nR );
}
int sem_wait( DWORD dwSem )
{
	int nR;

	nR = syscall_stub( SCTYPE_DEC_SEMAPHORE, dwSem );

	return( nR );
}
