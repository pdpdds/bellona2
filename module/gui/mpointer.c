#include <bellona2.h>
#include "gui.h"

static MouseDataStt	mdata;

static void mouse_move_state();
static int forward_mouse_button_mesg();
static int mouse_button_state( BYTE *pMData );

// 시스템 마우스 포인터의 구조체 주소를 리턴한다.
MousePointerStt *get_system_mpointer()
{
	GuiStt		*pGui;

	pGui = get_gui_stt();
	
	return( &pGui->mpointer );	
}

// 마우스의 그려진 상태를 리턴한다. 
int get_system_mouse_draw()
{
	MousePointerStt	*pMP;

	pMP = get_system_mpointer();

	return( pMP->nDrawFalg );
}
 
//  마우스 포인터를 그린다. 
static void internal_draw_mpointer( int nFlag )
{
	MousePointerStt		*pMP;
	GuiStt				*pGui;
	int					nSrcX, nSrcY, nSrcH, nSrcV, nX, nY;

	// 시스템의 마우스 포인터를 구한다. 
	pGui = get_gui_stt();
	pMP  = get_system_mpointer();
	
	// 그려져 있는데 또 그리거나 지워져 있는데 또 지우지 않는다. 
	if( pMP->nDrawFalg == nFlag )
		return;

	nX = pMP->nX;
	nY = pMP->nY;
	nX -= pMP->pCurrentCursor->wHotX;
	nY -= pMP->pCurrentCursor->wHotY;
	nSrcX = 0;
	nSrcY = 0;
	
	// X, Y Limit를 다시 계산한다. 
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

	// 커서를 그리기 전에 원래 이미지를 보관한다. 
	if( nFlag != 0 )
	{
		draw_image16( pMP->pCurrentCursor->pVoidImg, pMP->pVoidBackImg, nX, nY, nSrcX, nSrcY, nSrcH, nSrcV );
	}
	else	// 커서를 원래 이미지로 지운다. 
	{
		draw_image16( pMP->pVoidBackImg, NULL, nX, nY, nSrcX, nSrcY, nSrcH, nSrcV );
	}

	// 그려짐 상태를 저장한다. 
	pMP->nDrawFalg = nFlag;
}
static int nMpointerCounter = 0;
// enter_gui()에서 호출된다.
// 이것을 호출해 주지 않으면 ALT-F4로 나갔다가 다시 진입햇을 때 마우스 포인터가 안보이게 된다.
void init_mpointer()
{
	nMpointerCounter = 0;
}
void draw_mouse_pointer( int nFlag )
{
	_asm {
		PUSHFD
		CLI						// Interrupt를 Disable해주지 않으면 마우스 포인터가 번진다.
	}
	if( nFlag == 0 )
	{
		nMpointerCounter--;	 // 중첩호출 되면 마우스 포인터가 번진다.
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
// pMData는 3바이트
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
{	// 마우스 포인터의 정보
	MousePointerStt		*pMP;
	GuiStt				*pGui;
	int					nDelta, nNewX, nNewY;

	pGui = get_gui_stt();
	pMP = &pGui->mpointer;

	// 마우스 포인터를 지운다. 
	draw_mouse_pointer( 0 );

	// 버튼 클릭 상태
	pMP->byState = pMData[0];

	nNewX = pMP->nX;
	nNewY = pMP->nY;

	// X 좌표 이동
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
	// Y 좌표 이동
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

	// 화면을 벗어난 경우 위치 보정. 
	if( nNewX >= pGui->wall.obj.r.nH )
		nNewX =  pGui->wall.obj.r.nH - 1;
	if( nNewY >= pGui->wall.obj.r.nV )
		nNewY =  pGui->wall.obj.r.nV - 1;
	if( nNewX < 0 )
		nNewX = 0;
	if( nNewY < 0 )
		nNewY = 0;

	// 마우스의 움직임 상태를 처리한다. 
	if( nNewX != pMP->nX || nNewY != pMP->nY )
	{	// 마우스가 움직인 경우에만 Move Message가 날아간다. 
		pMP->nX = nNewX;
		pMP->nY = nNewY;
		mouse_move_state();
	}
	
	// 마우스 버튼의 상태를 처리한다. 
	mouse_button_state( pMData );

	// 마우스 포인터를 그린다. 
	draw_mouse_pointer( 1 );

	// 마우스 데이터를 갱신한다. 
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

// 현재 마우스를 소유하고 있는 윈도우를 찾는다.
struct WinTag *get_mouse_owner_win()
{
	MousePointerStt	*pMP;
	
	pMP  = get_system_mpointer();
	
	return( pMP->pIncludeWin );
}

// 마우스의 움직임 상태를 처리한다. 
static void mouse_move_state()
{
	MousePointerStt	*pMP;
	WinStt			*pWin;
	GuiStt			*pGui;

	// 시스템의 마우스 포인터를 구한다. 
	pMP  = get_system_mpointer();
	pGui = get_gui_stt();

	// MOVE MODE 인가?
	if( pGui->nMoveResizeFlag == GUI_MOVE_MODE )
	{	// Move Rect의 위치를 이동한다. 
		move_rect_change_pos( pMP->nX, pMP->nY );
		return;
	}
	else if( pGui->nMoveResizeFlag == GUI_RESIZE_MODE )
	{
		// Resize Rect의 위치를 이동한다. 
		resize_rect_change_shape( pMP->nX, pMP->nY );
		return;
	}

	// 커서 아래의 윈도우를 찾는다. 
	pWin = find_window_by_pos( pMP->nX, pMP->nY );
	if( pMP->pIncludeWin != pWin )
	{	
		// Include Win이 바뀐것  (이전 윈도우에 이탈 메시지를 보낸다.) 
		if( pMP->pIncludeWin != NULL )
			kpost_message( pMP->pIncludeWin, WMESG_MOUSE_MOVE_OUT, 0, 0 );

		// 새 윈도우에 진입 메시지를 보낸다. 
		pMP->pIncludeWin = pWin;
		if( pMP->pIncludeWin != NULL )
			kpost_message( pMP->pIncludeWin, WMESG_MOUSE_MOVE_IN, 0, 0 );
	}

	// MOUSE MOVE 메시지 생성
	if( pWin != NULL )
	{
		int nX, nY;
		inner_pos( &pWin->obj.r, pMP->nX, pMP->nY, &nX, &nY );
		kpost_message( pWin, WMESG_MOUSE_MOVE, nX, nY );
	}	
	else
	{
		// 마우스 밑에 윈도우가 없다면 스크린쪽으로 MOVE 메시지를 보내야하지 않을까?
		//...
	}

	return;
}

// 마우스 버튼의 상태를 처리한다. 
static int mouse_button_state( BYTE *pMData )
{
	GuiStt	*pGui;

	// MOVE MODE 인가?
	pGui = get_gui_stt();
	if( pGui->nMoveResizeFlag == GUI_MOVE_MODE )
	{
		if( !(pMData[0] & MOUSE_LBTN_DOWN ) )
		{	// 윈도우 이동 모드를 벗어난다.
			leave_move_mode();
		}	
		return( 0 );
	}	
	else if( pGui->nMoveResizeFlag == GUI_RESIZE_MODE )
	{
		if( !(pMData[0] & MOUSE_LBTN_DOWN ) )
		{	// 윈도우 이동 모드를 벗어난다.
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

	// 커서 아래의 윈도우를 찾는다. 
	pMP = get_system_mpointer();
	pWin = find_window_by_pos( pMP->nX, pMP->nY );
	if( pWin != NULL )
	{	// 스크린 좌표를 윈도우 기준으로 변환한다. 
		inner_pos( &pWin->obj.r, pMP->nX, pMP->nY, &nX, &nY );

		// 윈도우 쪽으로 메시지를 보낸다. 
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


