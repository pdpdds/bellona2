#include "login.h"

#include <process.h>

// ���̵�� �н����带 üũ�Ѵ�.
static int chk_id_pass()
{
	char	szID[64];
	char	szPass[64];
	
	szID[0] = szPass[0] = 0;

	// ID�� �Է� �޴´�.
	for( ;; )
	{
		// ID�� �Է��� ������ ������ ����.
		for( ; szID[0] == 0; )
		{
			printf( "user:" );
			input_str( szID, sizeof( szID ) -1 );
			printf( "\n" );
		}
		break;
	}	

	// Pass�� �Է� �޴´�.
	printf( "pass:" );
	input_pass( szPass, sizeof( szPass ) -1 );
	
	printf( "\nhi~ %s.\n", szID );

	return( 0 );
}

// ���� �����Ѵ�.
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
	
	// child�� ������ ������ ����Ѵ�.
	nResultPID = waitpid( nPID, &nExitCode );

	printf( "waitpid: nResultPID = %d, exit_code = %d\n", nResultPID, nExitCode );

	return( 0 );
}				

int main( int argc, char *argv[] )
{
	for( ;; )
	{
		// ���̵�� �н����带 üũ�Ѵ�.
		chk_id_pass();

		// ���� �����Ѵ�.
		exec_shell();
	}	
	
	return( 0 );
}