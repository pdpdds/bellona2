#include "lib.h"

static ModuleStt *pModule = NULL;

int stdlib_main( ModuleStt *pCurModule, int argc, char* argv[] )
{
	pModule = pCurModule;
	printf( "B2OS Standard C Library (c) 2003 by OHJJ\n" );

	return( 0 );
}
															