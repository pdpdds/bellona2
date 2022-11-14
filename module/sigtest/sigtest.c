#include <types.h>
#include <stdio.h>
#include <stdlib.h>
int main( int argc, char *argv[] )
{
	printf( "signal test -> pause()\n" );
	pause();
	printf( "signal arrived: <- pause().\n" );
	return( 0 );
}							  