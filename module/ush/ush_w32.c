//
// ush functions on win32 platform.
//

// windows
#include <windows.h>
#include <conio.h>
#include <direct.h>
#include <io.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// h/common
#include <env.h>
#include <funckey.h>
#include <dirent.h>

static int finddata_to_dirent( struct dirent *pEnt, struct _finddata_t *pFi )
{
	strcpy( pEnt->d_name, pFi->name );
	pEnt->d_size   = pFi->size;
	pEnt->d_attr   = pFi->attrib;
	pEnt->t_access = pFi->time_access;
	pEnt->t_write  = pFi->time_write;
	pEnt->t_create = pFi->time_create;
	
	return( 0 );
}

DIR *opendir( char *pPath )
{
	struct	_finddata_t		fi;
	char					szT[260];
	DIR						dir, *pDir;

	memset( &dir, 0, sizeof( dir ) );
	
	// ���丮�� �����Ѵ�.
	dir.lHandle = _findfirst( pPath, &fi );
	if( dir.lHandle < 0 )
		return( NULL );

	if( ( fi.attrib & _A_SUBDIR ) != 0 )
	{	// ���丮��.  �ڿ� *.*�� �ٿ��� �ٽ� �����Ѵ�.
		if( pPath[strlen( pPath ) -1] == '\\' )
			sprintf( szT, "%s%s", pPath, "*.*" );
		else
			sprintf( szT, "%s\\%s", pPath, "*.*" );
		
		// ���丮�� �ݰ� �ٽ� ����.
		_findclose( dir.lHandle );
		dir.lHandle = _findfirst( szT, &fi );
		if( dir.lHandle < 0 )
			return( NULL );
	}

	dir.wDirType = DIR_TYPE_FAT;

	finddata_to_dirent( &dir.ent, &fi );
	dir.byValidEnt = 1;

	pDir = (DIR*)malloc( sizeof( DIR ) );
	if( pDir == NULL )
		return( NULL );

	memcpy( pDir, &dir, sizeof( DIR ) );

	return( pDir );
}

struct dirent *readdir( DIR *pDir )
{
	int						nR;
	struct	_finddata_t		fi;

	if( pDir->byValidEnt != 0 )
	{
		pDir->byValidEnt = 0;
		return( &pDir->ent );
	}

	nR = _findnext( pDir->lHandle, &fi );
	if( nR < 0 )
		return( NULL );

	finddata_to_dirent( &pDir->ent, &fi );

	return( &pDir->ent );
}

int closedir( DIR *pDir ) 
{
	_findclose( pDir->lHandle );

	free( pDir );

	return( 0 );
}				

// ��� �н��� ���� �н��� �����Ѵ�.
char *get_abs_path( char *pRPath, int nSize, char *pPath )
{
	if( pPath[1] == ':' )
	{
		strcpy( pRPath, pPath );
		if( pPath[2] == '\\' && pPath[3] == 0 )
			strcat( pRPath, "*.*" );
	}
	else
	{
		_getcwd( pRPath, nSize );
		if( pPath != NULL && pPath[0] != 0 )
		{
			strcat( pRPath, "\\" );
			strcat( pRPath, pPath );
		}
	}

	return( pRPath );
}

// �� ���ڸ� �о���δ�.
int getch()
{
	int nCh;

	nCh = _getch();
    if( nCh == (int)0xe0 )
    {   // ���Ű.
        nCh = _getch();
        switch( nCh )
        {
        case 0x53 : nCh = BK_DEL;   break;
        case 0x4B : nCh = BK_LEFT;  break;
        case 0x4D : nCh = BK_RIGHT; break;
        case 0x47 : nCh = BK_HOME;  break;
        case 0x4F : nCh = BK_END;   break;
        case 0x48 : nCh = BK_UP;    break;
        case 0x50 : nCh = BK_DOWN;  break;
        }
    }

	return( nCh );
}

// Ŀ���� ��ġ�� ���Ѵ�.
void get_xy( int *pX, int *pY )
{
    CONSOLE_SCREEN_BUFFER_INFO  si;
    BOOL                        bR;
    HANDLE                      hHandle;
    int                         nX, nY;

    if( pX == NULL ) pX = &nX;
    if( pY == NULL ) pY = &nY;

    pX[0] = pY[0] = 0;

    hHandle = GetStdHandle( STD_OUTPUT_HANDLE );
    if( hHandle == NULL )
        return;

    // Ŀ�� ������ ���Ѵ�.
    bR = GetConsoleScreenBufferInfo( hHandle, &si );
    if( bR == FALSE )
        return;

    pX[0] = si.dwCursorPosition.X;
    pY[0] = si.dwCursorPosition.Y;
}

// Ŀ���� ��ġ�� �����Ѵ�.
void set_xy( int nX, int nY )
{
    COORD                       pos;
    HANDLE                      hHandle;

    hHandle = GetStdHandle( STD_OUTPUT_HANDLE );
    if( hHandle == NULL )
        return;

    pos.X = nX;
    pos.Y = nY;
    SetConsoleCursorPosition( hHandle, pos );
}

int set_fg_proc( DWORD dwID )
{
	return( 0 );
}

extern UEnvStt *get_proc_env();
int init_ush_md()
{
    UEnvStt *pEnv;
    pEnv = get_proc_env();
    init_env_buff( pEnv );

    return( 0 );
}

int close_ush_md()
{
    UEnvStt *pEnv;
    pEnv = get_proc_env();
    close_env_buff( pEnv );
    
    return( 0 );
}

static int insert_char( char *pS, int nI, int nChar )
{
    char    ch, ch_next;
    int     nLength, nX;

    nLength = strlen( pS );
    if( nI >= nLength )
    {   // ���� ���� �߰��ϸ� �ȴ�.
        pS[nI] = (char)nChar;
        pS[nI+1] = 0;
        return( nI +1 );
    }

    // �߰��� �����Ѵ�.
    ch = (char)nChar;
    for( nX = nI;; )
    {
        ch_next = pS[nX];
        pS[nX] = ch;
        if( ch == 0 )
            break;
        ch = ch_next;
        nX++;
    }

    return( nI+1 );
}

static int delete_char( char *pS, int nI )
{
    int nX;

    if( pS[nI] == 0 )
        return( 0 );

    for( nX = nI; ; nX++ )
    {
        pS[nX] = pS[nX+1];
        if( pS[nX] == 0 )
            break;    
    }

    return( 0 );
}

// nStartX ���� ���� ������ �����.
static int clear_line( int nStartX, int nY )
{
    char szT[128], nTotal;

    // ' '�� ����� ����.
    nTotal = 80 - nStartX;
    memset( szT, ' ', nTotal );
    szT[nTotal] = 0;
    
    printf( szT );

    return( 0 );
}

//pStr�� ���ݱ��� �Էµ� ���ڿ�
int input_str( char *pStr, int nSize )
{
	char	szT[260];
    int		nLineSize, nIPos, nDispX, nDispLen, nX, nY, nR, nChar, nLength;

    // Ŀ���� ���� ��ġ�� �˾Ƴ���.
    get_xy( &nX, &nY );

	nIPos = strlen( pStr );
	nLineSize = 79 - nX;
	nDispX = 0;

    for( ;; )
    {
		nLength = strlen( pStr );

		if( nDispX + nLength > nLineSize )
			nDispX = nLength - nLineSize;

		if( nIPos < nDispX )
			nDispX = nIPos;

		if( nIPos - nDispX > nLineSize )
			nDispX += nLineSize - (nIPos - nDispX);

        // ���ݱ����� ��Ʈ���� ����Ѵ�.
		set_xy( nX, nY );
		if( (int)strlen( &pStr[nDispX] ) > nLineSize )
		{
			memcpy( szT, &pStr[nDispX], nLineSize );
			szT[nLineSize] = 0;
			printf( szT );
		}
		else
			printf( &pStr[nDispX] );

        // ���� ������ �����. 
		nDispLen = strlen( &pStr[nDispX] );
		if( nX+nDispLen < 79 )
			clear_line( nX+nDispLen, nY ); 

        // Ŀ�� ��ġ�� �ٽ� ��´�.
		if( nIPos - nDispX > nLineSize )
			set_xy( nX + nLineSize, nY );
		else
			set_xy( nX + nIPos - nDispX , nY );

        // �� ���ڸ� �о���δ�.
        nChar = getch();
        if( nChar == 9 )
            nChar = ' ';              // TAB -> SPACE

        if( nChar == BK_DEL )
        {
            delete_char( pStr, nIPos );
        }
        else if( nChar == BK_LEFT )
        {
            if( nIPos > 0 )
                nIPos--;
        }
        else if( nChar == BK_RIGHT )
        {
            if( nIPos < nLength )
                nIPos++;            
        }
        else if( nChar == BK_HOME )
        {
            nIPos = 0;
        }
        else if( nChar == BK_END )
        {
            nIPos = nLength;            
        }
        else if( nChar == BK_UP )
        {

        }
        else if( nChar == BK_DOWN )
        {
        
        }
        else if( nChar == 13 )			// ENTER
        {  
            return( 0 );				// �Էµ� ���� ó���Ѵ�.
        }
        else if( nChar == 27 )			// ESC
        {
			pStr[0] = 0;
			nDispX  = 0;
			nIPos   = 0;
        }
        else if( nChar == 8 )			// BACKSPACE
        {
            if( nIPos > 0 )
            {
                delete_char( pStr, nIPos-1);
                nIPos--;
            }
        }
        else // ���ۿ� �߰��Ѵ�.
        {   
			if( nLength < nSize )
			{	// �Է¹��� �� �ִ� ������ �ִ� ��쿡�� �߰��Ѵ�.
	            nR = insert_char( pStr, nIPos, nChar );
		        nIPos++;
			}
        }
    }   

    return( 0 );
}
