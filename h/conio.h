#ifndef BELLONA2_CONIO_h
#define BELLONA2_CONIO_h

extern BELL_EXPORT int  del_line( int nX, int nY );
extern BELL_EXPORT int  is_gui_mode();

extern BELL_EXPORT void set_xy( int nX, int nY );
extern BELL_EXPORT void get_xy( int *pX, int *pY );

extern int direct_disp_str( int nX, int nY, char *pS );

#endif
