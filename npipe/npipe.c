/*
VMWare 4.0 ����.
Serial Port�� ������ �� pipe ���� �����ϸ� \\\\.\\pipe\\com_1���� �����ȴ�.
�ӵ��� �����ϴ� �κ��� ���� ������ ����� �ʿ� ����.
�ٸ� ���α׷����� �ش� pipe�� �����ϰ� ������ VMWare ���� �� com_1�� ������ ��
���ٴ� ������ ����Ѵ�.  ���� VMWare�� ����ǰų� ����õ� �� ���µ� com_1
pipe�� �ݾҴٰ� VMWare���� create�� �� open�ؼ� ����ؾ� �Ѵ�.
*/

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

static char buff[1024];
static char carr[] = "|/-\\";

static void move_back_cursor()
{
	CONSOLE_SCREEN_BUFFER_INFO	ci;
    BOOL						bR;
	COORD                       pos;
    HANDLE                      hHandle;

    hHandle = GetStdHandle( STD_OUTPUT_HANDLE );
    if( hHandle == NULL )
        return;

	bR = GetConsoleScreenBufferInfo( hHandle, &ci ); 
	if( bR == FALSE )
		return;

	pos.X = ci.dwCursorPosition.X - 1;;
	pos.Y = ci.dwCursorPosition.Y;
    
	if( pos.X >= 0 )
		SetConsoleCursorPosition( hHandle, pos );
}

// clear screen "<-[2J"  move cursor "<-[1;1H" cursor to right "<-[1@"	"backspace <-[1D"
static char cb[128], escb[16];
static int  nCBIndex  = 0;
static int  nESCMode  = 0;
static int  nESCIndex = 0;
// ���ۿ� �׿� �ִ� ���� ���ڸ� ����Ѵ�.
static void flush_output()
{
	if( nCBIndex <= 0 )
		return;

	printf( cb );
	cb[0] = 0;
	nCBIndex = 0;
}

static void output_char( char ch )
{
	if( nESCMode != 0 )
	{
		escb[ nESCIndex++ ] = ch;
		escb[ nESCIndex   ] = 0;
		if( ch == '@' || ( 'A' <= ch && ch <= 'Z' ) )
		{
			if( strcmpi( escb, "[1D" ) == 0 ) 
				move_back_cursor();	// Ŀ���� �� ĭ �ڷ� �����δ�.

			nESCMode  = 0;
			nESCIndex = 0;
			escb[0] = 0;
		}
		return;
	}

	if( ch == 27 )
	{
		nESCMode = 1;
		return;
	}

	cb[ nCBIndex++ ] = ch;
	cb[ nCBIndex   ] = 0;	
	
	if( nCBIndex >= sizeof( cb ) -1 )
		flush_output();
}

static int npipe()
{
	int		nI;
	BOOL	bR;
	HANDLE	hFile;
	DWORD	dwRead, dwI;

	// ���µ� ������ ������ ����.
	for( nI = 0;; )
	{
		hFile = CreateFile( "\\\\.\\pipe\\com_1", GENERIC_READ, FILE_SHARE_READ,
				NULL, OPEN_EXISTING, 0, NULL );
		if( hFile == NULL || hFile == (HANDLE)0xFFFFFFFF )
		{
			//if( nI > 3 )
			//	nI = 0;
			//printf( "[WAIT] %c\r", carr[nI++] );
			Sleep(500);
			continue;
		}
		else
			break;
	}
	printf( "\r[START]  \n" );
		
	for( ;; )
	{
		bR = ReadFile( hFile, buff, sizeof( buff )-1, &dwRead, NULL ); 
		if( bR == FALSE || dwRead == 0 )
			break;		// VMWare�� ����õǰų� �����.
		
		buff[dwRead] = 0;

		// �� ���ھ� ���۸� �� ����Ѵ�.
		for( dwI = 0; dwI < dwRead; dwI++ )
			output_char( buff[dwI] );

		// �׿��ִ� ���ڸ� ����Ѵ�.
		flush_output();
	}	

	CloseHandle( hFile );
	printf( "[END]\n" );

	return( 0 );
}

int main( int argc, char *argv[] )
{
	printf( "NPipe bellona2 message console.\n"	);
	printf( "Press CTRL-C to exit.\n" );

	for( ;; )
		npipe();

	return( 0 );
}