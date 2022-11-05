#include <windows.h>
#include <io.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "codemap.h"

int main( int argc, char *argv[] )
{
	int		nR;
	DWORD	dwTick;

	printf( "CODEMAP (BELLONA2 Symbol Extractor for VC6) (c) Copyright 2001-2003 by OHJJ\n" );

	if( argc <= 1 )
	{
		printf( "USAGE : CODEMAP <MAP_FILE> [DBG_FILE]\n" );
		return( 0 );
	}

	dwTick = GetTickCount();
	
	// Map file, Dbg file
	nR = codemap( argv[1], argv[2] );

	dwTick = GetTickCount() - dwTick;

	printf( "Ok. [Elapsed Time: %d.%d sec]\n", dwTick / 1000, dwTick % 1000 );
    return( 0 );
}