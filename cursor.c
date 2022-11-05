#include "bellona2.h"

extern void gui_cursor_redirect( int nCursorX, int nCursorY );

// ���� �������� Ŀ���� ��ġ�� �̵���Ų��.
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
	vWritePort( (ULONG)0x3D4, 0x0E );	// ���� ������
	vWritePort( (ULONG)0x3D5, dwT );

	dwT = (DWORD)( dwOffs & (DWORD)0xFF );
	vWritePort( (ULONG)0x3D4, 0x0F );   // ����	������
	vWritePort( (ULONG)0x3D5, dwT );
}

// Ŀ���� X, Y��ġ�� �����Ѵ�.
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

// Ŀ���� ���� X,Y��ġ��  �����Ѵ�.
void set_cursor_xy( int x, int y )
{
	DWORD  dwT;

	dwT = (DWORD)( x + y*80 );

	// ���� Ŀ���� �����δ�.
	move_cursor( dwT );
}

