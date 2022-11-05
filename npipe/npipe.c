/*
VMWare 4.0 기준.
Serial Port를 생성할 때 pipe 모드로 생성하면 \\\\.\\pipe\\com_1으로 생성된다.
속도를 설정하는 부분은 없고 별도로 고려할 필요 없다.
다른 프로그램에서 해당 pipe를 오픈하고 있으면 VMWare 부팅 시 com_1을 오픈할 수
없다는 에러를 출력한다.  따라서 VMWare가 종료되거나 재부팅될 때 오픈된 com_1
pipe를 닫았다가 VMWare에서 create한 후 open해서 사용해야 한다.
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
// 버퍼에 쌓여 있는 문든 문자를 출력한다.
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
				move_back_cursor();	// 커서를 한 칸 뒤로 움직인다.

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

	// 오픈될 때까지 루프를 돈다.
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
			break;		// VMWare가 재부팅되거나 종료됨.
		
		buff[dwRead] = 0;

		// 한 문자씩 버퍼링 후 출력한다.
		for( dwI = 0; dwI < dwRead; dwI++ )
			output_char( buff[dwI] );

		// 쌓여있는 문자를 출력한다.
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