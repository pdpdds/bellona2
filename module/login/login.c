#include "login.h"

#include <process.h>

// 아이디와 패스워드를 체크한다.
static int chk_id_pass()
{
	char	szID[64];
	char	szPass[64];
	
	szID[0] = szPass[0] = 0;

	// ID를 입력 받는다.
	for( ;; )
	{
		// ID를 입력할 때까지 루프를 돈다.
		for( ; szID[0] == 0; )
		{
			printf( "user:" );
			input_str( szID, sizeof( szID ) -1 );
			printf( "\n" );
		}
		break;
	}	

	// Pass를 입력 받는다.
	printf( "pass:" );
	input_pass( szPass, sizeof( szPass ) -1 );
	
	printf( "\nhi~ %s.\n", szID );

	return( 0 );
}

// 쉘을 실행한다.
static int exec_shell()
{
	int nResultPID, nPID, nExitCode;

	nPID = fork();
	if( nPID == 0 )
	{
		//printf( "execve( /c/ush.exe )\n" );
		execve( "/c/ush.exe", NULL, NULL );
	}

	//printf( "waitpid( %d )...\n", nPID );
	
	// child가 종료할 때까지 대기한다.
	nResultPID = waitpid( nPID, &nExitCode );

	printf( "waitpid: nResultPID = %d, exit_code = %d\n", nResultPID, nExitCode );

	return( 0 );
}				

int main( int argc, char *argv[] )
{
	for( ;; )
	{
		// 아이디와 패스워드를 체크한다.
		chk_id_pass();

		// 쉘을 실행한다.
		exec_shell();
	}	
	
	return( 0 );
}