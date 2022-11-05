#include <bellona2.h>
#include "gui.h"

static SysColorEntStt sys_color[] = {
	{ COLOR_INDEX_MENU_DK_TEXT	, RGB16(  60,  60,  60 ) },
	{ COLOR_INDEX_MENU_LT_TEXT	, RGB16( 220, 220, 220 ) },
	{ COLOR_INDEX_MENU_BACK		, COLOR_MENU_BACK        },
	{ COLOR_INDEX_MENU_DK_BACK	, RGB16( 140,  90, 100 ) },
	{ COLOR_INDEX_MENU_DK		, RGB16( 115,  76,  84 ) },
	{ COLOR_INDEX_MENU_LT		, RGB16( 242, 218, 235 ) },
	{ -1, 0 }
};

UINT16 get_sys_color( int nIndex )
{
	int nI;

	for( nI = 0; nI < END_OF_COLOR_INDEX; nI++ )
	{
		if( sys_color[nI].nIndex == nIndex )
			return( sys_color[nI].wColor );
	}
	
	return( 0 );
}

