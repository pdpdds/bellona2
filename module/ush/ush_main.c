#include "ush.h"

int main( int argc, char *argv[] )
{
	print_version();

	// 키 입력을 받아들이기 위해 FG로 설정한다.
	set_fg_proc( 0 );

	interactive_mode();

    return( 0 );
}

