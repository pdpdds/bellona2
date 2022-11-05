#ifndef BELLONA2_KBD_HEADER_oh
#define BELLONA2_KBD_HEADER_oh

struct ThreadTag;

// bit mask of the control keys
#define BM_LEFT_SHIFT		1
#define BM_LEFT_CTRL		2
#define BM_LEFT_ALT			4
#define BM_RIGHT_SHIFT		8
#define BM_RIGHT_CTRL	   16
#define BM_RIGHT_ALT	   32
#define BM_CAPS_LOCK	   64
#define BM_SCROLL_LOCK	  128
#define BM_NUM_LOCK		  256

// control key scan code
#define BSCODE_CAPS_LOCK		 0x3A
#define BSCODE_NUM_LOCK			 0x45
#define BSCODE_SCROLL_LOCK		 0x46
#define BSCODE_LEFT_SHIFT_ON	 0x2A
#define BSCODE_LEFT_SHIFT_OFF	 0xAA
#define BSCODE_RIGHT_SHIFT_ON	 0x36
#define BSCODE_RIGHT_SHIFT_OFF	 0xB6
#define BSCODE_LEFT_CTRL_ON      0x71
#define BSCODE_LEFT_CTRL_OFF	 0xF1
#define BSCODE_RIGHT_CTRL_ON	 0x1D
#define BSCODE_RIGHT_CTRL_OFF	 0x9D
#define BSCODE_LEFT_ALT_ON		 0x38
#define BSCODE_LEFT_ALT_OFF		 0xB8
#define BSCODE_RIGHT_ALT_ON		 0x70
#define BSCODE_RIGHT_ALT_OFF	 0xF0

//  function keys and others
#include <funckey.h>

typedef struct KbdTblTag{
    UCHAR byPrimary;   	// Primary Code
    UCHAR bySecondary; 	// Secondary Code
    UCHAR byShift;     	// Shift Caps Lock 조합에 의한 코드
};
typedef struct KbdTblTag KbdTblStt;

// 키보드로부터 입력된 하나의 키에 대한 값.
typedef struct BKeyTag{
	int     nCtrl;  	// 키가 눌려질 당시의 특수키
	UCHAR   byCode; 	// 키값
};
typedef struct BKeyTag BKeyStt;

#define MAX_KEY					16
typedef struct KbdQTag {
	int		nTotalKey;
	int		nNextIn, nNextOut;		// next insert, delete index
	BKeyStt	key[ MAX_KEY ];			// kbd q
};
typedef struct KbdQTag KbdQStt;				   

typedef int (*KBD_CALLBACK)( BKeyStt *pKey );

extern KbdTblStt KbdTable[];

struct ThreadTag;

extern BELL_EXPORT int  getchar();

extern void vInitKBDLed();
extern void kbd_handler();
extern int thread_kbhit();
extern char *gets( char *pS );
extern void vSetKBDLed( int nState );
extern void vDispKbdState( int nState );
extern void vSetKBDCallBack( KBD_CALLBACK pCb );
extern int  alloc_thread_kbd_q( struct ThreadTag *pThread );
extern int  free_thread_kbd_q( struct ThreadTag *pThread );
extern int  sub_key_from_thread( struct ThreadTag *pThread, BKeyStt *pKey );
extern int  add_key_to_thread( struct ThreadTag *pThread, BKeyStt *pKey );

#endif
