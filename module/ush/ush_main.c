#include "ush.h"

int main( int argc, char *argv[] )
{
	print_version();

	// Ű �Է��� �޾Ƶ��̱� ���� FG�� �����Ѵ�.
	set_fg_proc( 0 );

	interactive_mode();

    return( 0 );
}

