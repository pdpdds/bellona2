#ifndef BELLONA2_CURSOR_HEADER
#define BELLONA2_CURSOR_HEADER

// dwOffset의 위치로 커서를 옮긴다. 
extern void move_cursor( DWORD dwOffs );

// XY의 위치로 커서를 옮긴다.
extern void set_cursor_xy( int x, int y );

// 커서의 XY, 위치를 알아낸다.
extern void get_cursor_xy( int *pX, int *pY );

#endif