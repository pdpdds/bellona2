#include "lib.h"

int printf( char *pFmt, ... )
{
	va_list		va;
	int			nR;
	char		szT[1024];
	
	va_start( va, pFmt );
	nR = vsprintf( szT, pFmt, va );
	va_end( va ); 

	// system call
	nR = syscall_stub( SCTYPE_TTYOUT, (DWORD)szT );
	
	return( nR );
}

int getch()
{
	int nR;

	nR = system_call( SCTYPE_GETCH, 0 );

	return( nR );
}

int kbhit()
{
    int nR;
    nR = syscall_stub( SCTYPE_KBHIT );
    return( nR );
}

int get_time( TTimeStt *pT )
{
	int nR;
	nR = syscall_stub( SCTYPE_GET_TIME, pT );
	return( nR );
}
