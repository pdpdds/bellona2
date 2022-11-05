#include "bellona2.h"

static int nCtrlKey = 0, nPrefix = 0;

KbdTblStt KbdTable[] = {
   {0x00,         0x00,      0x00},         // 00
   {0x50,         0x1B,      0x1B},         // 01     Esc         Esc
   {0x60,         '1',       '!'},          // 02     1           !
   {0x60,         '2',       '@'},          // 03     2           @
   {0x60,         '3',       '#'},          // 04     3           #
   {0x60,         '4',       '$'},          // 05     4           $
   {0x60,         '5',       '%'},          // 06     5           %
   {0x60,         '6',       '^'},          // 07     6           ^
   {0x60,         '7',       '&'},          // 08     7           &
   {0x60,         '8',       '*'},          // 09     8           *
   {0x60,         '9',       '('},          // 0A     9           (
   {0x60,         '0',       ')'},          // 0B     0           )
   {0x60,         '-',       '_'},          // 0C     -           _
   {0x60,         '=',       '+'},          // 0D     =           +
   {0x50,         0x08,      0x08},         // 0E     BkSpc
   {0x50,         0x09,      0x90},         // 0F     TAB
   {0x70,         'q' ,      'Q'},          // 10     q
   {0x70,         'w' ,      'W'},          // 11     w
   {0x70,         'e' ,      'E'},          // 12     e
   {0x70,         'r' ,      'R'},          // 13     r
   {0x70,         't' ,      'T'},          // 14     t
   {0x70,         'y' ,      'Y'},          // 15     y
   {0x70,         'u' ,      'U'},          // 16     u
   {0x70,         'i' ,      'I'},          // 17     i
   {0x70,         'o' ,      'O'},          // 18     o
   {0x70,         'p' ,      'P'},          // 19     p
   {0x60,         '[',       '{'},          // 1A     [
   {0x60,         ']',       '}'},          // 1B     ]
   {0x50,         BK_ENTER,  BK_ENTER},     // 1C     CR
   {0x00,         0x00,      0x00},         // 1D     LCtrl
   {0x70,         'a' ,      'A' },         // 1E     a
   {0x70,         's' ,      'S' },         // 1F     s
   {0x70,         'd' ,      'D' },         // 20     d
   {0x70,         'f' ,      'F' },         // 21     f
   {0x70,         'g' ,      'G' },         // 22     g
   {0x70,         'h' ,      'H' },         // 23     h
   {0x70,         'j' ,      'J' },         // 24     j
   {0x70,         'k' ,      'K' },         // 25     k
   {0x70,         'l' ,      'L' },         // 26     l (L)
   {0x60,         ';',       ':' },         // 27     ;
   {0x60,         0x27,      '"' },         // 28     '
   {0x60,         '`',       '~' },         // 29     `
   {0x00,         0x00,      0x00},         // 2A     LfShf
   {0x60,         '\\',      '|' },         // 2B     "\"
   {0x70,         'z' ,      'Z'},          // 2C     z
   {0x70,         'x' ,      'X'},          // 2D     x
   {0x70,         'c' ,      'C'},          // 2E     c
   {0x70,         'v' ,      'V'},          // 2F     v
   {0x70,         'b' ,      'B'},          // 30     b
   {0x70,         'n' ,      'N'},          // 31     n
   {0x70,         'm' ,      'M'},          // 32     m
   {0x60,         ',',       '<' },         // 33     ,
   {0x60,         '.',       '>' },         // 34     .
   {0x60,         '/',       '?' },         // 35     /
   {0x00,         0x00,      0x00},         // 36     RtShf
   {0x50,         '*',       '*' },         // 37     Num *
   {0x00,         0x00,      0x00},         // 38     LAlt
   {0x50,         ' ',       ' ' },         // 39     Space
   {0x00,         0x00,      0x00},         // 3A     CpsLk
   {0x50,         BK_F1,     BK_F1},        // 3B     F1
   {0x50,         BK_F2,     BK_F2},        // 3C     F2
   {0x50,         BK_F3,     BK_F3},        // 3D     F3
   {0x50,         BK_F4,     BK_F4},        // 3E     F4
   {0x50,         BK_F5,     BK_F5},        // 3F     F5
   {0x50,         BK_F6,     BK_F6},        // 40     F6
   {0x50,         BK_F7,     BK_F7},        // 41     F7
   {0x50,         BK_F8,     BK_F8},        // 42     F8
   {0x50,         BK_F9,     BK_F9},        // 43     F9
   {0x50,         BK_F0,     BK_F0},        // 44     F10
   {0x00,         0x00,      0x00},         // 45     NumLk
   {0x00,         0x00,      0x00},         // 46     ScrLk
   {0x80,         '7',       BK_HOME},      // 47     Num 7      Home
   {0x80,         '8',       BK_UP},        // 48     Num 8      Up
   {0x80,         '9',       BK_PGUP},      // 49     Num 9      Pg Up
   {0x80,         '-',       '-'},          // 4A     Num -      Pad
   {0x80,         '4',       BK_LEFT},      // 4B     Num 4      Left
   {0x80,         '5',       '5'},          // 4C     Num 5      (Extra code)
   {0x80,         '6',       BK_RIGHT},     // 4D     Num 6      Right
   {0x80,         '+',       '+'},          // 4E     Num +      Pad
   {0x80,         '1',       BK_END},       // 4F     Num 1      End
   {0x80,         '2',       BK_DOWN},      // 50     Num 2      Down
   {0x80,         '3',       BK_PGDN},      // 51     Num 3      Pg Dn
   {0x80,         '0',       BK_INS},       // 52     Num 0      Insert
   {0x80,         '.',       BK_DEL},       // 53     Num .      Del
   {0x1C,         BK_SYSRQ,  BK_PRTSC },    // 54     SysRq      PrtSc
   {0x00,         0x00,      0x00},         // 55
   {0x00,         0x00,      0x00},         // 56
   {0x19,         BK_F11,    BK_F11 },      // 57     F11
   {0x1A,         BK_F12,    BK_F12 },      // 58     F12
   {0x00,         0x00,      0x00},         // 59
   {0x00,         0x00,      0x00},         // 5A
   {0x00,         0x00,      0x00},         // 5B
   {0x00,         0x00,      0x00},         // 5C
   {0x00,         0x00,      0x00},         // 5D
   {0x00,         0x00,      0x00},         // 5E
   {0x00,         0x00,      0x00},         // 5F
   {0x0E,         BK_INS,    BK_INS  },     // 60     Ins
   {0x0B,         BK_END,    BK_END  },     // 61     End
   {0x02,         BK_DOWN,   BK_DOWN },     // 62     Down
   {0x0C,         BK_PGDN,   BK_PGDN },     // 63     PgDn
   {0x03,         BK_LEFT,   BK_LEFT },     // 64     Left
   {0x00,         0x00,      0x00},         // 65
   {0x04,         BK_RIGHT,  BK_RIGHT},     // 66     Right
   {0x06,         BK_HOME,   BK_HOME },     // 67     Home
   {0x01,         BK_UP,     BK_UP   },     // 68     Up
   {0x05,         BK_PGUP,   BK_PGUP },     // 69     PgUp
   {0x7F,         BK_DEL,    BK_DEL  },     // 6A     Delete
   {0x80,         '/',       '/' },         // 6B     /
   {0x0D,         BK_ENTER,  BK_ENTER},     // 6C     ENTER
   {0x00,         0x00,      0x00},         // 6D
   {0x00,         0x00,      0x00},         // 6E
   {0x00,         0x00,      0x00},         // 6F
   {0x00,         0x00,      0x00},         // 70
   {0x00,         0x00,      0x00},         // 71
   {0x00,         0x00,      0x00},         // 72
   {0x00,         0x00,      0x00},         // 73
   {0x00,         0x00,      0x00},         // 74
   {0x00,         0x00,      0x00},         // 75
   {0x00,         0x00,      0x00},         // 76
   {0x00,         0x00,      0x00},         // 77
   {0x00,         0x00,      0x00},         // 78
   {0x00,         0x00,      0x00},         // 79
   {0x00,         0x00,      0x00},         // 7A
   {0x00,         0x00,      0x00},         // 7B
   {0x00,         0x00,      0x00},         // 7C
   {0x00,         0x00,      0x00},         // 7D
   {0x00,         0x00,      0x00},         // 7E
   {0x00,         0x00,      0x00}          // 7F
};

// 키보드의 발광다이오드의 상태를 변경한다.
void vSetKBDLed( int nState )
{
	DWORD dwCode = 0;

	if( nState & BM_CAPS_LOCK )
		dwCode |= 4;
	if( nState & BM_NUM_LOCK )
		dwCode |= 2;
	if( nState & BM_SCROLL_LOCK )
		dwCode |= 1;

	// 어셈블리 설정 루틴을 콜한다.
	vAsmSetKBDLed( dwCode );
}	
void vDispKbdState( int nState )
{
	UCHAR szState[3];	

	szState[0] = szState[1] = szState[2] = 0xDB;
													   
	if( nState & BM_CAPS_LOCK )
		szState[1] = 'C';				  
	if( nState & BM_NUM_LOCK )
		szState[0] = 'N';
	if( nState & BM_SCROLL_LOCK )
		szState[2] = 'S';
	
	// 화면 우측 상단에 CAPS,NUM,SCROLL LOCK의 상태를 뿌린다.
	//B_nWriteToVideoMem_Len( 77, 0, szState, 3);
}

void vInitKBDLed()
{
	// Keyboard Led를 초기화한다.
	//nCtrlKey = BM_NUM_LOCK;
	vSetKBDLed( nCtrlKey );
	// 변경된 상태를 화면 우측 상탄에 나타낸다.
	vDispKbdState( nCtrlKey );
}

// Foreground 쓰레드로 키를 전달한다.
int deliver_key( BKeyStt *pK )
{
	ThreadStt	*pThread;
	ProcessStt	*pProcess;

	// 디버거 활성상태이면 그쪽으로 키를 보낸다.
	if( is_debugger_active() )
	{
		kdbg_key_input( pK );
		return( 0 );
	}	

    // FG 프로세스를 구한다.
	pProcess = get_sys_fg_process();
	if( pProcess == NULL )
	{
		kdbg_printf( "fg process = NULL!\n" );
		return( -1 );
	}

    // FG Process의 FG Thread를 구한다.
	pThread = get_fg_thread( pProcess );
	if( pThread == NULL )
	{
		kdbg_printf( "fg thread == NULL\n" );
		return( -1 );
	}

	add_key_to_thread( pThread, pK );

	return( 0 );
}		

// 커널에서 처리할 특수키 조합을 처리한다.
static int special_key_combination( BKeyStt *pKey )
{
	ThreadStt *pInit;

	// RIGHT CTRL+ESC  
	if( pKey->nCtrl & BM_RIGHT_CTRL && pKey->byCode == BK_ESC )
	{
		// set the init thread to be the foreground thread.
		pInit = get_init_thread();
        if( pInit == NULL ) // 2002-11-22
            return( -1 );   // error
		set_fg_process( NULL, pInit->pProcess );
		set_foreground_thread ( pInit );

		return( 1 );
	}
	// RIGHT CTRL + TAB  
	else if( pKey->nCtrl & BM_RIGHT_CTRL && pKey->byCode == BK_TAB )
	{	// 가상 콘솔을 전환한다.
		change_next_vconsole();
		return( 1 );
	}
	else if( pKey->nCtrl & (BM_RIGHT_ALT | BM_LEFT_ALT) && pKey->byCode == BK_F4 )
	{	// gui 모드에서 빠져 나온다.
		end_gui();  // vesa.c
		return( 1 );
	}	

	return( 0 );		    // no special key
}

// real keyboard handler
static void internal_kbd_handler( UCHAR byCode )
{
	BKeyStt	key;
	UCHAR	byChar;
	int		nTemp, nCtrlKeyChanged = 0;;

	if( byCode == 0 )  // 0은 잘못된 코드이므로 그냥 돌아간다.
	{
		byChar = 0;
		goto End_Key;
	}

    if( byCode == (UCHAR)0xE0 || byCode == (UCHAR)0xE1 )
	{
		nPrefix = 1;
		goto End_Key;
	}

	// Control Key 입력인지 확인해 본다.
    if( byCode == BSCODE_CAPS_LOCK )
    {
		nTemp = BM_CAPS_LOCK;
		nCtrlKey ^= nTemp;
		nCtrlKeyChanged = 1;
    }
    else if( byCode == BSCODE_NUM_LOCK )
    {
		nTemp = BM_NUM_LOCK;
		nCtrlKey ^= nTemp;
		nCtrlKeyChanged = 1;
    }
    else if( byCode == BSCODE_SCROLL_LOCK )
    {
		nTemp = BM_SCROLL_LOCK;
		nCtrlKey ^= nTemp;
		nCtrlKeyChanged = 1;
    }
    else if( byCode == BSCODE_LEFT_SHIFT_ON )
    {
		nTemp = BM_LEFT_SHIFT;
		nCtrlKey |= nTemp;
		nCtrlKeyChanged = 1;
    }
    else if( byCode == BSCODE_LEFT_SHIFT_OFF )
    {
		nTemp = BM_LEFT_SHIFT;
		nCtrlKey &= ~nTemp;
		nCtrlKeyChanged = 1;
    }
    else if( byCode == BSCODE_RIGHT_SHIFT_ON )
    {
		nTemp = BM_RIGHT_SHIFT;
		nCtrlKey |= nTemp;
		nCtrlKeyChanged = 1;
    }
    else if( byCode == BSCODE_RIGHT_SHIFT_OFF )
    {
		nTemp = BM_RIGHT_SHIFT;
		nCtrlKey &= ~nTemp;
		nCtrlKeyChanged = 1;
    }
    else if( byCode == BSCODE_LEFT_CTRL_ON )
    {
		nTemp = BM_LEFT_CTRL;
		nCtrlKey |= nTemp;
		nCtrlKeyChanged = 1;
    }
    else if( byCode == BSCODE_LEFT_CTRL_OFF )
    {
		nTemp = BM_LEFT_CTRL;
		nCtrlKey &= ~nTemp;
		nCtrlKeyChanged = 1;
    }
    else if( byCode == BSCODE_RIGHT_CTRL_ON )
    {
		nTemp = BM_RIGHT_CTRL;
		nCtrlKey |= nTemp;
		nCtrlKeyChanged = 1;
    }
    else if( byCode == BSCODE_RIGHT_CTRL_OFF )
    {
		nTemp = BM_RIGHT_CTRL;
		nCtrlKey &= ~nTemp;
		nCtrlKeyChanged = 1;
    }
    else if( byCode == BSCODE_LEFT_ALT_ON )
    {
		nTemp = BM_LEFT_ALT;
		nCtrlKey |= nTemp;
		nCtrlKeyChanged = 1;
    }
    else if( byCode == BSCODE_LEFT_ALT_OFF )
    {
		nTemp = BM_LEFT_ALT;
		nCtrlKey &= ~nTemp;
		nCtrlKeyChanged = 1;
    }
    else if( byCode == BSCODE_RIGHT_ALT_ON )
    {
		nTemp = BM_RIGHT_ALT;
		nCtrlKey |= nTemp;
		nCtrlKeyChanged = 1;
    }
    else if( byCode == BSCODE_RIGHT_ALT_OFF )
    {
		nTemp = BM_RIGHT_ALT;
		nCtrlKey &= ~nTemp;
		nCtrlKeyChanged = 1;
    }					

	if( nCtrlKeyChanged != 0 )
	{
		// Keyboard Led를 변경한다.
		vSetKBDLed( nCtrlKey );
		// 변경된 상태를 화면 우측 상탄에 나타낸다.
		vDispKbdState( nCtrlKey );
		goto End_Key;	  // 특수키 등은 여기서 돌아간다.
	}
	//....................................................//
	//일반키 처리
	if( byCode & 0x80 )  // Break Code (키를 뗏을 때)
        goto End_Key;          // 그냥 돌아간다.

	// 키보드 테이블에서 스캔코드에 해당하는 키값을 가져온다.
	byChar = KbdTable[byCode].byPrimary;
	if( byChar == 0 )  // 이미 처리된 특수키 이므로 이쪽으로 타고들어올 일은 없다.
		goto End_Key;

	// byChar의 MSB가 1인지 확인해 본다.
	if( byChar == 0x80 )  // Numeric Pad로부터 입력된 키다.
	{
		// INS, HOME, PGUP, DEL, END, PGDN, UP,LEFT,RIGHT,DOWN은 앞에 E0가 오고 다음에 
		// 키코드가 따라온다.
		// 숫자패드의 방향키들은 E0가 오지않고 바로 코드가 온다.
		// 따라서 프리픽스가 붙어 있으면 NUM LOCK에 상관없이 방향키로써 출력한다.
		if( nCtrlKey & BM_NUM_LOCK && nPrefix == 0 )  // NUM LOCK이 눌려 있는지 확인해 본다.
		    byChar = KbdTable[byCode].bySecondary;   
		else
			byChar = KbdTable[byCode].byShift;
	}
	else if( byChar == 0x70 )  // 'a부터 'z'까지의 영문자
	{
		if( (nCtrlKey & BM_LEFT_SHIFT) || (nCtrlKey & BM_RIGHT_SHIFT) )
		{	// 쉬프트가 눌려져 있다.
			if( nCtrlKey & BM_CAPS_LOCK )
				byChar = KbdTable[byCode].bySecondary;
			else
				byChar = KbdTable[byCode].byShift;
		}
		else
		{
			if( nCtrlKey & BM_CAPS_LOCK )
				byChar = KbdTable[byCode].byShift;
			else
				byChar = KbdTable[byCode].bySecondary;
		}
	}
	else if( byChar == 0x60 )  // 1-0 등 나머지 Shift에만 영향을 받는 것들.
	{
		if( (nCtrlKey & BM_LEFT_SHIFT) || (nCtrlKey & BM_RIGHT_SHIFT) )
			byChar = KbdTable[byCode].byShift;
		else
			byChar = KbdTable[byCode].bySecondary;
	}
    else if( byChar == 0x50 ) // 나머지 Shift등에 영향을 받지 않는 키들
		byChar = KbdTable[byCode].bySecondary;						   

	nPrefix = 0;

	key.nCtrl  = nCtrlKey;
	key.byCode = byChar;

	// 커널에서 처리할 특수 키.
	if( special_key_combination( &key ) != 1 )
		deliver_key( &key );		// deliver key

End_Key:
	return;
}

_declspec(naked) void kbd_handler()
{			 
	static UCHAR byCode;

	_asm { 
		PUSHAD
	    PUSHFD
	    CLI
		PUSH DS				
		PUSH ES				
		PUSH FS				
		PUSH GS				
		MOV AX, GSEL_DATA32
		MOV DS, AX
		MOV ES, AX
		MOV FS, AX
		MOV GS, AX
	}

	vReadPort( (DWORD)0x60,  &byCode );

	internal_kbd_handler( byCode );

	// send EOI signal
	vSendEOI( 1 );

	_asm {
		POP GS
		POP FS
		POP ES
		POP DS
		POPFD
		POPAD
		IRETD
	}
}

// allocate thread's keyboard q
int  alloc_thread_kbd_q( ThreadStt *pThread )
{
	KbdQStt	*pQ;

	if( pThread->pKbdQ != NULL )
		return( -1 );

	pQ = (KbdQStt*)kmalloc( sizeof( KbdQStt ) );
	if( pQ == NULL )
		return( -1 );

	// clear q with 0
	memset( pQ, 0, sizeof( KbdQStt ) );

	pThread->pKbdQ = pQ;

	return( 0 );
}

// free thread's keyboard q
int  free_thread_kbd_q( ThreadStt *pThread )
{
	if( pThread->pKbdQ == NULL )
		return( -1 );

	kfree( pThread->pKbdQ );
	pThread->pKbdQ = NULL;

	return( 0 );
}

// add key to the thread
int add_key_to_thread( ThreadStt *pThread, BKeyStt *pKey )
{
	KbdQStt		*pQ;

	pQ = pThread->pKbdQ;
	if( pQ == NULL || pQ->nTotalKey >= MAX_KEY )
		return( -1 );
	
	_asm PUSHFD
	_asm CLI
	
	memcpy( &pQ->key[ pQ->nNextIn++ ], pKey, sizeof( BKeyStt ) );
	if( pQ->nNextIn == MAX_KEY )
		pQ->nNextIn = 0;
	pQ->nTotalKey++;

	_asm POPFD

	awake_the_first_waiting_thread( &kbd_event, pThread );

	return( 0 );
}

// 쓰레드의 KBD Q에 입력이 있는지 확인한다.
int thread_kbhit()
{
	KbdQStt		*pQ;
	ThreadStt	*pThread;

	pThread = get_current_thread();
	if( pThread == NULL )
		return( 0 );

	pQ = pThread->pKbdQ;
	if( pQ == NULL )
		return( 0 );		

	if( pQ->nTotalKey == 0 )
		return( 0 );
	else
		return( 1 );
}

// sub key from the thread
int sub_key_from_thread( ThreadStt *pThread, BKeyStt *pKey )
{
	KbdQStt *pQ;

	pQ = pThread->pKbdQ;
	if( pQ == NULL )
		return( -1 );			// no kbd q.

	if( pQ->nTotalKey <= 0 )
		return( 0 );			// thread kbd q is empty.

	_asm PUSHFD	//===============================================//
	_asm CLI

	memcpy( pKey, &pQ->key[ pQ->nNextOut++ ], sizeof( BKeyStt ) );
	if( pQ->nNextOut == MAX_KEY )
		pQ->nNextOut = 0;
	
	pQ->nTotalKey--;	

	_asm POPFD //===============================================//

	return( 1 );
}

// get one character
int getchar()
{
	int			nR;
	BKeyStt		key;
	ThreadStt	*pThread;

	// if debugger is active
	if( is_debugger_active() )
	{
		return( debugger_getchar() );
	}

GET_KEY:	
	pThread = get_current_thread();

	// get key from thread key buffer
	nR = sub_key_from_thread( pThread, &key );
	if( nR == 1 )
		return( (int)key.byCode );
	else if( nR == -1 )
		return( -1 );		// no kbd q
	else
	{
		// if no character is entered then enter into blocking
		nR = wait_event( &kbd_event, 0 );
		goto GET_KEY;
	}
}

// get string
char *gets( char *pS )
{
	int		nI, nX, nY;
	char	szX[2];

	szX[1] = pS[0] = 0;

	for( ;; )
	{
		szX[0] = (char)getchar();
		if( szX[0] < 0 )				// 2004-03-25
			continue;					// 
		if( szX[0] == (char)BK_ENTER )
			break;
		else if( szX[0] == (char)BK_BACKSPACE )
		{
			nI = strlen( pS );
			if( nI > 0 )
			{
				pS[nI-1] = 0;
				get_cursor_xy( &nX, &nY );
				set_cursor_xy( nX-1,  nY );
				kdbg_printf( " " );
				get_cursor_xy( &nX, &nY );
				set_cursor_xy( nX-1,  nY );
			}
		}
		else if( szX[0] == (char)BK_RIGHT || szX[0] == (char)BK_LEFT || 
			        szX[0] == (char)BK_HOME || szX[0] == (char)BK_END )
			continue;
		
		kdbg_printf( szX );

		strcat( pS, szX );
	}	

	return( pS );
}
