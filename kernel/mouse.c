#include "bellona2.h"

// GUI.MOD���� ���콺 CALL BACK �Լ��� ������ �ּҰ� ����ȴ�. 
static MouseStt	mouse;

// Ű����� ����� ���� �� �ִ°�?
static int kbd_ibuff_empty()
{
	int		nI;
	BYTE	byTe;

	for( nI = 0; nI < 65536; nI++ )
	{
		byTe = 0;
		vReadPort( 0x64, &byTe );
		if( ( byTe & 2 ) == 0 )
			return( 1 );	// �����͸� ���� �� �ִ�.
	}
	return( 0 );
}

// Ű���忡�� ���� �����Ͱ� �ִ°�?
static int kbd_obuff_full()
{
	int		nI;
	BYTE	byTe;

	for( nI = 0; nI < 65536; nI++ )
	{
		byTe = 0;
		vReadPort( 0x64, &byTe );
		if( ( byTe & 1 ) == 1 )
			return( 1 );	// �����Ͱ� ���ƿԴ�.
	}
	return( 0 );
}

// Ű����κ��� ���� ���� �д´�.
static int read_kbd_data( BYTE *pData )
{
	int		nI;
	BYTE	dummy;

	// ���� �����Ͱ� �ִ��� Ȯ��
	if( kbd_obuff_full() == 0 )
		return( -1 );

	// �̵� ������ �� �ʿ��� ����??
	for( nI = 0; nI < 32; nI++ ) ;

	if( pData == NULL )
		pData = &dummy;

	// �����͸� �д´�.
	vReadPort( 0x60, pData );

	return( 0 );
}

// Ű����� �����͸� ������.
static int write_kbd_data( BYTE byTe )
{
	// ���۰� ������� Ȯ��
	if( kbd_ibuff_empty() == 0 )
		return( -1 );

	// �����͸� ������.
	vWritePort( 0x60, byTe );

	// ���� ���� ó���Ǿ����� �� �� �� Ȯ��.
	if( kbd_ibuff_empty() == 0 )
		return( -1 );

	return( 0 );
}

// Ű���� ��Ʈ�ѷ��� ����� ������.
static int send_kbd_command( BYTE byCode )
{
	// ���۰� ������� Ȯ��
	if( kbd_ibuff_empty() == 0 )
		return( -1 );

	// �����͸� ������.
	vWritePort( 0x64, byCode );

	// ���� ���� ó���Ǿ����� �� �� �� Ȯ��.
	if( kbd_ibuff_empty() == 0 )
		return( -1 );

	return( 0 );
}

int init_kbd_and_ps2_mouse()
{
	BYTE	byTe;

	// PS2 ���콺�� Ȱ��ȭ �Ѵ�.
	send_kbd_command( 0xa8 );
	
    // ���¸� �д´�.
	read_kbd_data( NULL );			

	send_kbd_command( 0x20 );
	read_kbd_data( &byTe );			
	// byTe |= 3;
	
	// 2002-11-26
	// ���� ���� 0xA7�� �� �ִµ� �̰� 0x47�� 
	// �ٲ� �־�� Scan Code Conversion�� �Ͼ��.
    byTe = 0x47;

	send_kbd_command( 0x60 ); 	
	write_kbd_data( byTe );

	send_kbd_command( 0xd4 );
	write_kbd_data( 0xf4 );

	read_kbd_data( NULL );
	
	// ���콺 ����ü�� �ʱ�ȭ�Ѵ�. 
	memset( &mouse, 0, sizeof( mouse ) );

	return( 0 );
}

void set_mouse_callback( MOUSE_CALL_BACK pCB )
{
	mouse.pCB = pCB;
}

// ���콺 ���ͷ�Ʈ �ڵ鷯.
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

	// �����͸� �� ����Ʈ �޾Ƶ��δ�. 
	vReadPort( (DWORD)0x60, &byCode ); 
	mouse.packet[ mouse.nPacketCount ] = byCode;
	mouse.nPacketCount++;

	// 3����Ʈ�� �Ǿ��� �� GUI.MOD�� gui_mouse_call_back()�� ȣ���Ѵ�. 	mpointer.c
	if( mouse.nPacketCount >= 3 )
	{	// CALL_BACK�� ������ ȣ���Ѵ�. 
		if( mouse.pCB != NULL )
			mouse.pCB( mouse.packet );
		// �����͸� �����. 
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
