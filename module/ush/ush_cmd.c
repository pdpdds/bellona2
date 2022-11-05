#include "ush.h"

// ���丮 ��Ʈ���� ����Ѵ�.
static int print_dirent( int nI, struct dirent *pEnt, char *pPattern )
{
	printf( "%-19s", pEnt->d_name );
	if( nI > 0 && ( (nI+1) % 4) == 0 )
		printf( "\n" );

	return( 0 );
}

// path���� ��ο� ���ϸ� Ȥ�� ������ �и��� ����.
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

// ���丮�� �����Ͽ� ���� ����� ����Ѵ�.
int list_directory( char *pPath, char *pOption )
{
	int				nI;
	DIR				*pDir;
	struct dirent	*pEnt;
	char			szT[260], szPath[260], szPattern[128];

	// ��� �н��̸� ���� �н��� �����Ѵ�.
	pPath = get_abs_path( szT, sizeof( szT ) -1, pPath );

	// pRPath�� ���丮�ΰ�?
	pDir = opendir( pPath );
	if( pDir != NULL )
	{
		for( nI = 0; ; nI++ )
		{
			pEnt = readdir( pDir );
			if( pEnt == NULL )
				break;
			// ���� ���� ���丮 ��Ʈ���� ������ ����Ѵ�.
			print_dirent( nI, pEnt, NULL );
		}
		closedir( pDir );  
		goto QUIT;
	}

	// �н��� ���� �������� �и��Ѵ�.
	split_path_and_filename( szPath, szPattern, pPath );
 	pDir = opendir( szPath );
	if( pDir != NULL )
	{
		for( nI = 0; ; nI++ )
		{
			pEnt = readdir( pDir );
			if( pEnt == NULL )
				break;
			// ���� ���� ���丮 ��Ʈ���� ������ ����Ѵ�.
			print_dirent( nI, pEnt, szPattern );
		}
		closedir( pDir );
	}
QUIT:
	printf( "\n" );
	return( 0 );
}