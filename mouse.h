#ifndef BELLONA2_MOUSE_HEADER_jj
#define BELLONA2_MOUSE_HEADER_jj

typedef int (*MOUSE_CALL_BACK)( BYTE *pMousePacket );

typedef struct {
	int					nPacketCount;
	BYTE				packet[3];
	MOUSE_CALL_BACK		pCB;
} MouseStt;

extern BELL_EXPORT void set_mouse_callback( MOUSE_CALL_BACK pCB );

extern int init_kbd_and_ps2_mouse();
extern void ps2_mouse_handler();

#endif