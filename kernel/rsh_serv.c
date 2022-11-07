// remote shell kernel part
#include "bellona2.h"

static CharDevObjStt com1;

int rshell_thread( void *pParam )
{
	int		nR;
	BKeyStt	key;
	ThreadStt *pThread;
	
	// open serial port driver
	open_char_device( &com1, LOGICAL_KBD_MAJOR, 1 );

	memset( &key, 0, sizeof( key ) );

	for( ;; )
	{
		// read one char from com port
		nR = read_char_device( &com1 );
		if( nR > 0 )
		{
			key.byCode = (UCHAR)nR;

			// send key to the foreground thread
			pThread = get_fg_thread( get_fg_process() );
			// push the arrived character to the kbd buffer
			add_key_to_thread( pThread, &key );
		}
	}

	return( 0 );
}

int send_string_to_remote_shell( char *pS )
{
	int		nI, nJ;
	char	buff[2048];

	for( nJ = nI = 0; pS[nJ] != 0;  )
	{
		buff[nI++] = pS[nJ++];
		if( pS[nJ-1] == 10 )
		{
			buff[nI++] = 13;			
		}
	} 	
	buff[nI] = 0;
	
    write_char_device( &com1, buff, strlen( buff ) );

	return( 0 );
}