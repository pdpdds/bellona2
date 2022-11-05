#include <bellona2.h>
#include "gui.h"

static MouseDataStt	mdata;

static void mouse_move_state();
static int forward_mouse_button_mesg();
static int mouse_button_state( BYTE *pMData );

// �ý��� ���콺 �������� ����ü �ּҸ� �����Ѵ�.
MousePointerStt *get_system_mpointer()
{
	GuiStt		*pGui;

	pGui = get_gui_stt();
	
	return( &pGui->mpointer );	
}

// ���콺�� �׷��� ���¸� �����Ѵ�. 
int get_system_mouse_draw()
{
	MousePointerStt	*pMP;

	pMP = get_system_mpointer();

	return( pMP->nDrawFalg );
}
 
//  ���콺 �����͸� �׸���. 
static void internal_draw_mpointer( int nFlag )
{
	MousePointerStt		*pMP;
	GuiStt				*pGui;
	int					nSrcX, nSrcY, nSrcH, nSrcV, nX, nY;

	// �ý����� ���콺 �����͸� ���Ѵ�. 
	pGui = get_gui_stt();
	pMP  = get_system_mpointer();
	
	// �׷��� �ִµ� �� �׸��ų� ������ �ִµ� �� ������ �ʴ´�. 
	if( pMP->nDrawFalg == nFlag )
		return;

	nX = pMP->nX;
	nY = pMP->nY;
	nX -= pMP->pCurrentCursor->wHotX;
	nY -= pMP->pCurrentCursor->wHotY;
	nSrcX = 0;
	nSrcY = 0;
	
	// X, Y Limit�� �ٽ� ����Ѵ�. 
	nSrcH = pGui->wall.obj.r.nH - nX;
	nSrcV = pGui->wall.obj.r.nV - nY;
	if( nSrcH > pMP->nH )
		nSrcH = pMP->nH;
	if( nSrcV > pMP->nV )
		nSrcV = pMP->nV;

	if( nX < 0 )
	{
		nSrcX -= nX;
		nSrcH += nX;
		nX = 0;
	}
	if( nY < 0 )
	{
		nSrcY -= nY;
		nSrcV += nY;
		nY= 0;
	}
	
	//kdbg_printf( "mpointer: nX,nY,nSrcX,nSrcY,nSrcH,nSrcV = %d,%d,%d,%d,%d,%d\n", nX,nY,nSrcX,nSrcY,nSrcH,nSrcV );

	// Ŀ���� �׸��� ���� ���� �̹����� �����Ѵ�. 
	if( nFlag != 0 )
	{
		draw_image16( pMP->pCurrentCursor->pVoidImg, pMP->pVoidBackImg, nX, nY, nSrcX, nSrcY, nSrcH, nSrcV );
	}
	else	// Ŀ���� ���� �̹����� �����. 
	{
		draw_image16( pMP->pVoidBackImg, NULL, nX, nY, nSrcX, nSrcY, nSrcH, nSrcV );
	}

	// �׷��� ���¸� �����Ѵ�. 
	pMP->nDrawFalg = nFlag;
}
static int nMpointerCounter = 0;
// enter_gui()���� ȣ��ȴ�.
// �̰��� ȣ���� ���� ������ ALT-F4�� �����ٰ� �ٽ� �������� �� ���콺 �����Ͱ� �Ⱥ��̰� �ȴ�.
void init_mpointer()
{
	nMpointerCounter = 0;
}
void draw_mouse_pointer( int nFlag )
{
	_asm {
		PUSHFD
		CLI						// Interrupt�� Disable������ ������ ���콺 �����Ͱ� ������.
	}
	if( nFlag == 0 )
	{
		nMpointerCounter--;	 // ��øȣ�� �Ǹ� ���콺 �����Ͱ� ������.
		if( nMpointerCounter == 0 )
			internal_draw_mpointer( nFlag );
	}
	else 
	{
		nMpointerCounter++;
		if( nMpointerCounter == 1 )
			internal_draw_mpointer( nFlag );
	}
	_asm POPFD
}		 
// pMData�� 3����Ʈ
//////////////////////////
// LBTN Click 09 00 00	//
// RBTN Click 0A 00 00	//
// BTNUP      08 00 00	//
// UP         08 00 01	//
// DOWN       28 00 FF	//
// RIGHT      08 01 00	//
// LEFT       18 FF 00	//
//////////////////////////
#define M_SENSITIVITY  3
int gui_mouse_call_back( BYTE *pMData )
{	// ���콺 �������� ����
	MousePointerStt		*pMP;
	GuiStt				*pGui;
	int					nDelta, nNewX, nNewY;

	pGui = get_gui_stt();
	pMP = &pGui->mpointer;

	// ���콺 �����͸� �����. 
	draw_mouse_pointer( 0 );

	// ��ư Ŭ�� ����
	pMP->byState = pMData[0];

	nNewX = pMP->nX;
	nNewY = pMP->nY;

	// X ��ǥ �̵�
	if( pMData[1] < (BYTE)0x80 )
	{
		nNewX += (int)pMData[1];
		if( pMData[1] > M_SENSITIVITY )
			nNewX += ((int)pMData[1] * 3)/2;
	}
	else 
	{
		nDelta = (int)( (UINT16)0x100 - (UINT16)pMData[1] );
		nNewX -= nDelta;
		if( nDelta > 3 )
			nNewX -= (nDelta*3)/2;
	}
	// Y ��ǥ �̵�
	if( pMData[2] < (BYTE)0x80 )
	{
		nNewY -= (int)pMData[2];
		if( pMData[2] > M_SENSITIVITY )
			nNewY -= ((int)pMData[2]*3)/2;
	}
	else 
	{
		nDelta = (int)( (UINT16)0x100 - (UINT16)pMData[2] );
		nNewY += nDelta;
		if( nDelta > 3 )
			nNewY += (nDelta*3)/2;
	}

	// ȭ���� ��� ��� ��ġ ����. 
	if( nNewX >= pGui->wall.obj.r.nH )
		nNewX =  pGui->wall.obj.r.nH - 1;
	if( nNewY >= pGui->wall.obj.r.nV )
		nNewY =  pGui->wall.obj.r.nV - 1;
	if( nNewX < 0 )
		nNewX = 0;
	if( nNewY < 0 )
		nNewY = 0;

	// ���콺�� ������ ���¸� ó���Ѵ�. 
	if( nNewX != pMP->nX || nNewY != pMP->nY )
	{	// ���콺�� ������ ��쿡�� Move Message�� ���ư���. 
		pMP->nX = nNewX;
		pMP->nY = nNewY;
		mouse_move_state();
	}
	
	// ���콺 ��ư�� ���¸� ó���Ѵ�. 
	mouse_button_state( pMData );

	// ���콺 �����͸� �׸���. 
	draw_mouse_pointer( 1 );

	// ���콺 �����͸� �����Ѵ�. 
	memcpy( mdata.data, pMData, 3 );

	return( 0 );
}

int invalidate_mouse_owner( WinStt * pWin )
{
	MousePointerStt	*pMP;

	pMP  = get_system_mpointer();
	if( pMP->pIncludeWin == pWin )
		pMP->pIncludeWin = NULL;

	return( 0 );
}

// ���� ���콺�� �����ϰ� �ִ� �����츦 ã�´�.
struct WinTag *get_mouse_owner_win()
{
	MousePointerStt	*pMP;
	
	pMP  = get_system_mpointer();
	
	return( pMP->pIncludeWin );
}

// ���콺�� ������ ���¸� ó���Ѵ�. 
static void mouse_move_state()
{
	MousePointerStt	*pMP;
	WinStt			*pWin;
	GuiStt			*pGui;

	// �ý����� ���콺 �����͸� ���Ѵ�. 
	pMP  = get_system_mpointer();
	pGui = get_gui_stt();

	// MOVE MODE �ΰ�?
	if( pGui->nMoveResizeFlag == GUI_MOVE_MODE )
	{	// Move Rect�� ��ġ�� �̵��Ѵ�. 
		move_rect_change_pos( pMP->nX, pMP->nY );
		return;
	}
	else if( pGui->nMoveResizeFlag == GUI_RESIZE_MODE )
	{
		// Resize Rect�� ��ġ�� �̵��Ѵ�. 
		resize_rect_change_shape( pMP->nX, pMP->nY );
		return;
	}

	// Ŀ�� �Ʒ��� �����츦 ã�´�. 
	pWin = find_window_by_pos( pMP->nX, pMP->nY );
	if( pMP->pIncludeWin != pWin )
	{	
		// Include Win�� �ٲ��  (���� �����쿡 ��Ż �޽����� ������.) 
		if( pMP->pIncludeWin != NULL )
			kpost_message( pMP->pIncludeWin, WMESG_MOUSE_MOVE_OUT, 0, 0 );

		// �� �����쿡 ���� �޽����� ������. 
		pMP->pIncludeWin = pWin;
		if( pMP->pIncludeWin != NULL )
			kpost_message( pMP->pIncludeWin, WMESG_MOUSE_MOVE_IN, 0, 0 );
	}

	// MOUSE MOVE �޽��� ����
	if( pWin != NULL )
	{
		int nX, nY;
		inner_pos( &pWin->obj.r, pMP->nX, pMP->nY, &nX, &nY );
		kpost_message( pWin, WMESG_MOUSE_MOVE, nX, nY );
	}	
	else
	{
		// ���콺 �ؿ� �����찡 ���ٸ� ��ũ�������� MOVE �޽����� ���������� ������?
		//...
	}

	return;
}

// ���콺 ��ư�� ���¸� ó���Ѵ�. 
static int mouse_button_state( BYTE *pMData )
{
	GuiStt	*pGui;

	// MOVE MODE �ΰ�?
	pGui = get_gui_stt();
	if( pGui->nMoveResizeFlag == GUI_MOVE_MODE )
	{
		if( !(pMData[0] & MOUSE_LBTN_DOWN ) )
		{	// ������ �̵� ��带 �����.
			leave_move_mode();
		}	
		return( 0 );
	}	
	else if( pGui->nMoveResizeFlag == GUI_RESIZE_MODE )
	{
		if( !(pMData[0] & MOUSE_LBTN_DOWN ) )
		{	// ������ �̵� ��带 �����.
			leave_resize_mode();
		}	
		return( 0 );
	}
	
	if( mdata.data[0] & MOUSE_LBTN_DOWN )
	{	// LBUTTON UP
		if( !(pMData[0] & MOUSE_LBTN_DOWN ) )
			forward_mouse_button_mesg( WMESG_LBTN_UP );
	}
	else if( mdata.data[0] & MOUSE_RBTN_DOWN )
	{	// RBUTTON UP
		if( !(pMData[0] & MOUSE_RBTN_DOWN ) )
			forward_mouse_button_mesg( WMESG_RBTN_UP );
	}
	else
	{	// LBUTTON DN
		if( pMData[0] & MOUSE_LBTN_DOWN )
			forward_mouse_button_mesg( WMESG_LBTN_DN );
		// RBUTTON DN
		if( pMData[0] & MOUSE_RBTN_DOWN )
			forward_mouse_button_mesg( WMESG_RBTN_DN );
	}

	return( 0 );
}

static int forward_mouse_button_mesg( DWORD dwMesg )
{
	MousePointerStt	*pMP;
	WinStt			*pWin;
	int				nX, nY;

	// Ŀ�� �Ʒ��� �����츦 ã�´�. 
	pMP = get_system_mpointer();
	pWin = find_window_by_pos( pMP->nX, pMP->nY );
	if( pWin != NULL )
	{	// ��ũ�� ��ǥ�� ������ �������� ��ȯ�Ѵ�. 
		inner_pos( &pWin->obj.r, pMP->nX, pMP->nY, &nX, &nY );

		// ������ ������ �޽����� ������. 
		kpost_message( pWin, dwMesg, nX, nY );
	}
	
	return( 0 );
}

int get_mouse_pointer_index()
{
	GuiStt *pGui;
	pGui = get_gui_stt();
	return( pGui->mpointer.pCurrentCursor->nIndex );
}

int set_mouse_pointer( int nCSIndex )
{
	GuiStt *pGui;

	if( nCSIndex < 0 || nCSIndex >= MAX_CURSOR_SET_ENT )
		return( -1 );

	pGui = get_gui_stt();

	draw_mouse_pointer( 0 );
	pGui->mpointer.pCurrentCursor = &pGui->mpointer.cursor_set.ent[ nCSIndex ];
	draw_mouse_pointer( 1 );

	return( 0 );
}


