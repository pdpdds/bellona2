#ifndef BELLONA2_CURSOR_HEADER
#define BELLONA2_CURSOR_HEADER

// dwOffset�� ��ġ�� Ŀ���� �ű��. 
extern void move_cursor( DWORD dwOffs );

// XY�� ��ġ�� Ŀ���� �ű��.
extern void set_cursor_xy( int x, int y );

// Ŀ���� XY, ��ġ�� �˾Ƴ���.
extern void get_cursor_xy( int *pX, int *pY );

#endif