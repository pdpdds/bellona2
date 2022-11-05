#include "ush.h"

// 디렉토리 엔트리를 출력한다.
static int print_dirent( int nI, struct dirent *pEnt, char *pPattern )
{
	printf( "%-19s", pEnt->d_name );
	if( nI > 0 && ( (nI+1) % 4) == 0 )
		printf( "\n" );

	return( 0 );
}

// path에서 경로와 파일명 혹은 패턴을 분리해 낸다.
static int split_path_and_filename( char *pRPath, char *pRName, char *pPath )
{
	int nI;

	pRPath[0] = pRName[0] = 0;

	for( nI = strlen( pPath ) -1; nI >= 0; nI-- )
	{
		if( pPath[nI] == '\\' || pPath[nI] == '/' )
			break;
	}
	strcpy( pRName, &pPath[nI+1] );
	strcpy( pRPath, pPath );
	if( nI > 0 )
		pRPath[nI] = 0;
	else
		pRPath[1] = 0;

	return( 0 );
}

// 디렉토리를 오픈하여 파일 목록을 출력한다.
int list_directory( char *pPath, char *pOption )
{
	int				nI;
	DIR				*pDir;
	struct dirent	*pEnt;
	char			szT[260], szPath[260], szPattern[128];

	// 상대 패스이면 절대 패스로 변경한다.
	pPath = get_abs_path( szT, sizeof( szT ) -1, pPath );

	// pRPath가 디렉토리인가?
	pDir = opendir( pPath );
	if( pDir != NULL )
	{
		for( nI = 0; ; nI++ )
		{
			pEnt = readdir( pDir );
			if( pEnt == NULL )
				break;
			// 패턴 없이 디렉토리 엔트리의 내용을 출력한다.
			print_dirent( nI, pEnt, NULL );
		}
		closedir( pDir );  
		goto QUIT;
	}

	// 패스와 파일 패턴으로 분리한다.
	split_path_and_filename( szPath, szPattern, pPath );
 	pDir = opendir( szPath );
	if( pDir != NULL )
	{
		for( nI = 0; ; nI++ )
		{
			pEnt = readdir( pDir );
			if( pEnt == NULL )
				break;
			// 패턴 없이 디렉토리 엔트리의 내용을 출력한다.
			print_dirent( nI, pEnt, szPattern );
		}
		closedir( pDir );
	}
QUIT:
	printf( "\n" );
	return( 0 );
}