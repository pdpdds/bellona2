#include "lib.h"

char *getcwd( char *pB, int nSize )
{
	UAreaStt *pU;

	if( pB == NULL )
		return( NULL );

	pU = get_uarea();
	strcpy( pB, pU->szCWD );

	return( pB );
}

// ���ڿ� ��� '/' ������ �����Ѵ�.
static char *get_next_path_ent( char *pEnt, char *pS )
{
	int nI;

	pEnt[0] = 0;

	for( nI = 0; ; )
	{
		pEnt[nI] = pS[nI];
		if( pEnt[nI] == 0 )
			return( &pS[nI] );

		nI++;
		pEnt[nI] = 0;
		if( pEnt[nI-1] == '/' )
			return( &pS[nI] );
	}

	return( &pS[nI] );
}

// �н��� ��� ��� './', '../' ���� ó���Ѵ�.
char *refine_path( char *pPath )
{
	int 	nLevel, nI;
	char	*pNext, *pCur, *pPrev, *pT, szT[260], szA[128], szB[128];
	
	if( pPath == NULL || pPath[0] == 0 )
		return( pPath );
	
	pNext = pPath;
	pCur  = szA;
	pPrev = szB;
	szT[0] = pCur[0] = pPrev[0] = 0;
	for( nLevel = 0; ; nLevel++ )
	{
		pNext = get_next_path_ent( pCur, pNext );
		if( pCur[0] == 0 )
			break;

		if( strcmp( pCur, "/" ) == 0 && nLevel > 0 )
			continue;

		if( strcmp( pCur, "./" ) == 0 )
			continue;
			
		if( pPrev[0] == 0 )
			goto SET_PREV;
			
		if( strcmp( pCur, "../" ) == 0 || strcmp( pCur, ".." ) == 0 )
		{
			if( strcmp( pPrev, "../" ) != 0 )
			{
				pPrev[0] = 0;
				continue;
			}
		}
		
		strcat( szT, pPrev );		
		
SET_PREV:		
		pT    = pPrev;
		pPrev = pCur;
		pCur  = pT;
	}

	// �������� '.'�� ���� ������ �ʴ´�.
	if( strcmp( pPrev, "." ) != 0 )
		strcat( szT, pPrev );

	strcpy( pPath, szT );	

	// �������� '/'���� ������ '/'�� �����Ѵ�.
	nI = strlen( pPath );
	if( nI > 1 && pPath[nI-1] == '/' )
		pPath[nI-1] = 0;

	return( pPath );
}

// ��� �н��� ���� �н��� �����Ѵ�.
char *get_abs_path( char *pRPath, int nSize, char *pPath )
{
	char	szT[260];

	if( pPath[0] == '/' )
		strcpy( pRPath, pPath );
	else
	{
		getcwd( szT, sizeof( szT ) -1 );
		if( szT[0] == '/' && szT[1] == 0 )
			sprintf( pRPath, "/%s", pPath );
		else
			sprintf( pRPath, "%s/%s", szT, pPath );
	}

	// path�� refine �Ѵ�.
	refine_path( pRPath );
	
	return( pRPath );
}

int chdir( char *pTarget )
{
	UAreaStt	*pU;
	DIR			*pDir;
	char		szABSPath[260];

	// ��� �н��� �ϴ� ���� �н��� �����Ѵ�.
	get_abs_path( szABSPath, sizeof( szABSPath ) -1, pTarget );

	// ���丮�� �����Ѵ�.
	pDir = opendir( szABSPath );
	if( pDir == NULL )
	{
		kdbg_printf( "chdir: invalid path\n" );
		return( -1 );
	}

	closedir( pDir );

	// ���ο� ��θ� �����Ѵ�.
	pU = get_uarea();
	strcpy( pU->szCWD, szABSPath );

	return( 0 );
}	

void exit( int nExitCode )
{
    int nR;

  	// User Area�� �����Ѵ�.
	nR = close_uarea();

	syscall_stub( SCTYPE_EXIT, nExitCode );
}

void *malloc( DWORD dwSize )
{
	void *pV;

	pV = (void*)system_call( SCTYPE_MALLOC, dwSize );

	return( pV );
}

void free( void *pAddr )
{
	int nR;
		
	nR = system_call( SCTYPE_FREE, (DWORD)pAddr );
}

int delay( DWORD dwMiliSecond )
{
	int nR;

	nR = system_call( SCTYPE_DELAY, dwMiliSecond );

	return( nR );
}

void Sleep( DWORD dwMiliSecond )  // W32 ȣȯ��.
{
    delay( dwMiliSecond );
}

int wait( int *pExitCode )
{
	int nPID;

	nPID = (int)syscall_stub( SCTYPE_WAIT, pExitCode );

	return( nPID );
}

int waitpid( int nPID, int *pExitCode )
{
	int nResultPID;

	nResultPID = (int)syscall_stub( SCTYPE_WAITPID, nPID, pExitCode );

	return( nResultPID );
}

int waittid( int nTID, int *pExitCode )
{
	int nResultTID;

	nResultTID = (int)syscall_stub( SCTYPE_WAITTID, nTID, pExitCode );

	return( nResultTID );
}

int execve( char *pFile, char *argv[], char *envp[] )
{
	int nR;

	nR = (int)syscall_stub( SCTYPE_EXECVE, pFile, argv, envp );

	return( nR );
}	

#define MVAL  3346925945
#define AVAL  65531
static DWORD dwSeed = 1;
DWORD rand()
{
    DWORD dwR;
    dwR = (dwSeed * MVAL) + AVAL;
    
    dwSeed = dwR % 65536;//   dwSeed;

	dwR &= 0x7FFFFFFF;
	
    return( dwR );
}

void srand( DWORD dwS )
{
    dwSeed = dwS;
}

int abs( int nValue )
{
	if( nValue < 0 )
		return( nValue * (-1) );

	return( nValue );
}

