#include "bellona2.h"

extern void gui_cursor_redirect( int nCursorX, int nCursorY );

// 실제 물리적인 커서의 위치를 이동시킨다.
void move_cursor( DWORD dwOffs )
{
	DWORD			dwT;
	VConsoleStt 	*pVC;
	int				nCursorX, nCursorY;

	nCursorY = (int)( dwOffs / 80 );
	nCursorX = (int)( dwOffs % 80 );

	pVC = (VConsoleStt*)get_active_vconsole();
	if( pVC != NULL )
	{
		pVC->wCursorX = (UINT16)nCursorX;
		pVC->wCursorY = (UINT16)nCursorY;
	}							

	//VGA
	dwT = (DWORD)( dwOffs >> 8 );
	vWritePort( (ULONG)0x3D4, 0x0E );	// 상위 오프셋
	vWritePort( (ULONG)0x3D5, dwT );

	dwT = (DWORD)( dwOffs & (DWORD)0xFF );
	vWritePort( (ULONG)0x3D4, 0x0F );   // 하위	오프셋
	vWritePort( (ULONG)0x3D5, dwT );
}

// 커서의 X, Y위치를 리턴한다.
void get_cursor_xy( int *pX, int *pY )
{
	VConsoleStt 	*pVC;

	pVC = (VConsoleStt*)get_active_vconsole();
	if( pVC != NULL )
	{
		pX[0] = pVC->wCursorX;
		pY[0] = pVC->wCursorY;
	}
	else
	{
		pX[0] = 0;
		pY[0] = 0;
	}
}

// 커서를 임의 X,Y위치에  세팅한다.
void set_cursor_xy( int x, int y )
{
	DWORD  dwT;

	dwT = (DWORD)( x + y*80 );

	// 실제 커서를 움직인다.
	move_cursor( dwT );
}

