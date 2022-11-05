#include "bellona2.h"

// GUI.MOD에서 마우스 CALL BACK 함수를 심으면 주소가 저장된다. 
static MouseStt	mouse;

// 키보드로 명령을 보낼 수 있는가?
static int kbd_ibuff_empty()
{
	int		nI;
	BYTE	byTe;

	for( nI = 0; nI < 65536; nI++ )
	{
		byTe = 0;
		vReadPort( 0x64, &byTe );
		if( ( byTe & 2 ) == 0 )
			return( 1 );	// 데이터를 날릴 수 있다.
	}
	return( 0 );
}

// 키보드에서 들어온 데이터가 있는가?
static int kbd_obuff_full()
{
	int		nI;
	BYTE	byTe;

	for( nI = 0; nI < 65536; nI++ )
	{
		byTe = 0;
		vReadPort( 0x64, &byTe );
		if( ( byTe & 1 ) == 1 )
			return( 1 );	// 데이터가 날아왔다.
	}
	return( 0 );
}

// 키보드로부터 상태 값을 읽는다.
static int read_kbd_data( BYTE *pData )
{
	int		nI;
	BYTE	dummy;

	// 들어온 데이터가 있는지 확인
	if( kbd_obuff_full() == 0 )
		return( -1 );

	// 이딴 지연이 왜 필요한 거지??
	for( nI = 0; nI < 32; nI++ ) ;

	if( pData == NULL )
		pData = &dummy;

	// 데이터를 읽는다.
	vReadPort( 0x60, pData );

	return( 0 );
}

// 키보드로 데이터를 보낸다.
static int write_kbd_data( BYTE byTe )
{
	// 버퍼가 비었는지 확인
	if( kbd_ibuff_empty() == 0 )
		return( -1 );

	// 데이터를 날린다.
	vWritePort( 0x60, byTe );

	// 날린 값이 처리되었는지 한 번 더 확인.
	if( kbd_ibuff_empty() == 0 )
		return( -1 );

	return( 0 );
}

// 키보드 컨트롤러로 명령을 보낸다.
static int send_kbd_command( BYTE byCode )
{
	// 버퍼가 비었는지 확인
	if( kbd_ibuff_empty() == 0 )
		return( -1 );

	// 데이터를 날린다.
	vWritePort( 0x64, byCode );

	// 날린 값이 처리되었는지 한 번 더 확인.
	if( kbd_ibuff_empty() == 0 )
		return( -1 );

	return( 0 );
}

int init_kbd_and_ps2_mouse()
{
	BYTE	byTe;

	// PS2 마우스를 활성화 한다.
	send_kbd_command( 0xa8 );
	
    // 상태를 읽는다.
	read_kbd_data( NULL );			

	send_kbd_command( 0x20 );
	read_kbd_data( &byTe );			
	// byTe |= 3;
	
	// 2002-11-26
	// 원래 값이 0xA7이 들어가 있는데 이걸 0x47로 
	// 바꿔 주어야 Scan Code Conversion이 일어난다.
    byTe = 0x47;

	send_kbd_command( 0x60 ); 	
	write_kbd_data( byTe );

	send_kbd_command( 0xd4 );
	write_kbd_data( 0xf4 );

	read_kbd_data( NULL );
	
	// 마우스 구조체를 초기화한다. 
	memset( &mouse, 0, sizeof( mouse ) );

	return( 0 );
}

void set_mouse_callback( MOUSE_CALL_BACK pCB )
{
	mouse.pCB = pCB;
}

// 마우스 인터럽트 핸들러.
void _declspec(naked) ps2_mouse_handler()
{		
	static BYTE byCode;

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

	// 데이터를 한 바이트 받아들인다. 
	vReadPort( (DWORD)0x60, &byCode ); 
	mouse.packet[ mouse.nPacketCount ] = byCode;
	mouse.nPacketCount++;

	// 3바이트가 되었을 때 GUI.MOD의 gui_mouse_call_back()을 호출한다. 	mpointer.c
	if( mouse.nPacketCount >= 3 )
	{	// CALL_BACK이 있으면 호출한다. 
		if( mouse.pCB != NULL )
			mouse.pCB( mouse.packet );
		// 데이터를 지운다. 
		mouse.nPacketCount = 0;
		memset( mouse.packet, 0, sizeof( mouse.packet ) );
	}						

	// send EOI signal
	vSendEOI( 12 );

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
