//  GUI Extension Module for Bellona2 Kernel (2002-05-12 )
//  링크할 때 __chkesp 심볼이 없다고 난리를 지기면 "/GZ" 컴파일 옵션을 제거해야 한다.

#include <bellona2.h>
#include "gui.h"

extern WMFuncStt iconwin_marray[];
extern WMFuncStt taskbar_marray[];
extern WMFuncStt about_marray[];
extern WMFuncStt KCmdWin_marray[];


// 스크린 윈도우 정보
static GuiStt	    gui;			// 스크린, 해상도, Video Mode에 관한 정보.
static WinResStt	winres;			// 모듈의 리소스(아이콘, 비트멥, 커서, 폰트...)
static ModuleStt	*pModule;		// 모듈 자신의 핸들.
static WinStt		*pAboutWin;		// Bellona2 Gui의 About window
static WinStt		*pTaskBar;		// Bellona2 Gui의 Task Bar.
static WinStt		*pKCmdWin;		// Kernel Shell Window.
static WinThreadStt	*pKWinThread;	// 커널 윈도우들의 메시지를 처리하는 쓰레드.
static ThreadStt	*pGuiThread;	// GUI System 전체를 관할하는 쓰레드.

extern void set_grx_syscall();
extern void reset_grx_syscall();

int leave_gui();

GuiStt *get_gui_stt() { return( &gui ); }
WinResStt *get_winres_stt() { return( &winres ); }
WinThreadStt *get_kwin_thread() { return( pKWinThread ); }

/////////////////////////////////////////////////////////
// ALT-F4를 눌렀을 때 시그널을 받아서 처리하는 쓰레드  //
/////////////////////////////////////////////////////////

// 시그널 전달 방식의 변경으로 현재 ALT-F4는 동작하지 않는다.
int gui_thread( void *pPram )
{
//	DWORD dwSig;
	
	for( ;; )
	{	
		// 쓰레드의 모든 시그널을 클리어 한다.
		//clear_thread_signal_bits( NULL, 0xFFFFFFFF );

		// 시그널이 들어오기를 기다린다.
		kpause();		

		// 들어온 시그널을 처리한다.
		//dwSig = get_thread_signal_bits( NULL );

		// 시그널 비트를 클리어한다.
		//clear_thread_signal_bits( NULL, dwSig );
		//if( dwSig & SIG_USER_0 )
		//{	
		//	// Wall Window로 CLOSE 메시지를 보낸다.
		//	kpost_message( get_wall_win(), WMESG_CLOSE, 0, 0 );
		//}		
		//else if( dwSig & SIG_USER_1 )
		//{
		//	// GUI system 종료.
		//	leave_gui();	
		//}
	}	
}

// 모듈이 로딩되는 시점에서 호출된다.  초기화를 담당한다.
/////////////////////////////// GUI MAIN ////////////////////////////
/**/ int gui_main( ModuleStt *pCurModule, int argc, char* argv[] ) //
/////////////////////////////////////////////////////////////////////
{
	WinResStt		*pWR;
	WinResEntStt	*pEnt;
	int				nI, nR;

	pModule = pCurModule;
	kdbg_printf( "B2OS GUI Module (c) 2003 OHJJ\n" );

	// 자료구조를 0으로 클리어한다. 
	memset( &gui, 0, sizeof( gui ) );
	
	// 리소스를 인식한다. 
	memset( &winres, 0, sizeof( winres ) );
	nR = win_resource( pCurModule->dwLoadAddr, &winres );
	if( nR < 0 )
		kdbg_printf( "No resource.\n" );
	else
	{	// 리소스를 출력한다. 
		pWR = &winres;
		for( nI = 0; nI < pWR->nTotal; nI++ )
		{
			pEnt = &pWR->ent[nI];
			//kdbg_printf( "[%2d] Type=0x%03X, ResID=%-4d, Offset=0x%08X, Size=%d\n", 
			//	nI, pEnt->wType, pEnt->wID, pEnt->dwAddr, pEnt->dwSize );
		}
		//kdbg_printf( "Total %d resources.\n", pWR->nTotal );
	}		
	
	kdbg_printf("gui thread!\n");
	// gui thread를 생성한다. (ALT-F4를 눌렀을 때 이 쓰레드에서 처리된다.)
	pGuiThread = kcreate_thread( NULL, 0, (DWORD)gui_thread,  0, TS_READY_NORMAL );	
	if( pGuiThread == NULL )
	    kdbg_printf( "create gui thread failed!\n" );
	else
	{
		k_set_thread_alias( pGuiThread, GUI_THREAD_NAME );
	}

	
	kdbg_printf("gui thread success!\n");
	return( 0 );
}

typedef struct {
	WinStt	*pWin;
	char	*pTitle;
	int		nIconID;
} WallIconEntStt;

static WallIconEntStt iwin[] = {
	{ NULL, "SYSTEM", IDI_MY_COM },
	{ NULL, "CMD",    IDI_CMD    },
	{ NULL, NULL, 0 }
};

static int close_icon_windows()
{
	int nI;

	for( nI = 0; iwin[nI].pTitle != NULL; nI++ )
	{
		if( iwin[nI].pWin != NULL )
		{
			ksend_message( iwin[nI].pWin, WMESG_DESTROY, 0, 0 );
			iwin[nI].pWin = NULL;
		}
	}

	return( 0 );
}

static int create_icon_windows( WinThreadStt *pWinThread, WinStt *pParentWin )
{
	RectStt r;
	int 	nI;

	r.nH = ICON_WIN_H;
	r.nV = ICON_WIN_V;
	r.nX = 10;
	r.nY = 10;

	for( nI = 0; iwin[nI].pTitle != NULL; nI++ )
	{
		iwin[nI].pWin = kcreate_window( pWinThread, pParentWin, WSTYLE_FLAT, &r, iconwin_marray, 0, 0, 0, (DWORD)iwin[nI].pTitle, (DWORD)iwin[nI].nIconID );
		if( iwin[nI].pWin == NULL )
		{			
			kdbg_printf( "create icon window( %d ) : failed!\n", nI );
			continue;
		}

		//initialize_iconwin( iwin[nI].pWin, iwin[nI].pTitle, iwin[nI].nIconID );

		r.nY += ICON_WIN_V + 5;
	}

	return( 0 );
}

// x, y가 Rect 내에 포함되어 있는가?
int is_in_rect( RectStt *pR, int nX, int nY )
{
	if( pR == 0 )
		return( 0 );

	if( pR->nX <= nX && nX < pR->nX + pR->nH )// pBtn->pImg->nH )
	{
		if( pR->nY <= nY && nY < pR->nY + pR->nV )//pBtn->pImg->nV )
			return( 1 );
	}

	return( 0 );
}

static int gui_timer_mesg_post_func( DWORD dwWinID, DWORD dwTimerID, DWORD dwParamB )
{
	int		nR;
	WinStt	*pWin;

	pWin = find_window_by_id( dwWinID );
	if( pWin == NULL )
	{
		kdbg_printf( "gui_timer_mesg_post_func: Window(%d) not found!\n", dwWinID );
		return( -1 );
	}

	nR = kpost_message( pWin, WMESG_TIMER, dwTimerID, dwParamB );
	return( nR );	
}

// 비디오 메모리 전체를 wColor로 채운다.
static int fill_gui_screen( DWORD dwAddr, UINT16 wH, UINT16 wV, DWORD dwLineAdder, UINT16 wColor )
{
	UINT16	*pW;
	int 	nX, nY;

	pW = (UINT16*)dwAddr;
	for( nY = 0; nY < (int)wV; nY++ )
	{
		for( nX = 0; nX < (int)wH; nX++ )
			pW[nX] = wColor;
		pW = (UINT16*)( (DWORD)pW + dwLineAdder );
	}
	return( 0 );	
}

static int sys_cursor_id[] = {
	IDC_CURSOR_ARROW,
	IDC_CURSOR_RS_H,	
	IDC_CURSOR_RS_V,	
	IDC_CURSOR_RS_UR,	
	IDC_CURSOR_RS_UL,
	0
};

static int load_cursor_set( WinResStt *pWR, BCursorSetStt *pCS, int *pCSID )
{
	BCursorStt	*pCE;
	int 		nR, nI;

	memset( pCS, 0, sizeof( BCursorSetStt ) );
	
	for( nI = 0; nI < MAX_CURSOR_SET_ENT; nI++ )
	{
		pCE = &pCS->ent[nI];
		nR = load_cursor( pWR, pCE, pCSID[nI] );
		if( nR < 0 || pCE->pBit == NULL )
			continue;

		// 빈 이미지를 할당하여 bitmap을 이미지로 변환한다.  (kfree()로 해제하면 된다.)
		pCE->pVoidImg = bitmap_to_image16( pCE->pBit, MPOINTER_H, MPOINTER_V );
		// 커서 인덱스를 설정한다.
		pCE->nIndex = nI;
	}
	
	// 커서가 그려질 곳의 배경 이미지를 저장해 둘 곳을 할당한다. (kfree()로 해제하면 된다.) 
	gui.mpointer.pVoidBackImg = alloc_blank_image16( MPOINTER_H, MPOINTER_V );

	return( 0 );
}

static int free_cursor_set( BCursorSetStt *pCS )
{
	int nI;

	for( nI = 0; nI < MAX_CURSOR_SET_ENT; nI++ )
	{
		if( pCS->ent[nI].pVoidImg == NULL )
			continue;

		kfree( pCS->ent[nI].pVoidImg );
		pCS->ent[nI].pVoidImg = NULL;
	}
	
	kfree( gui.mpointer.pVoidBackImg );
	gui.mpointer.pVoidBackImg = NULL;
	return( 0 );
}

// GUI 모드로 진입한다. 
int enter_gui( VESAModeStt *pVM )
{
	RectStt		r;
	int			nI, nR;
	BYTE		*pBuff;
	DWORD		dwSize;
		
	kdbg_printf( "enter_gui\n" );

	// 그래픽 시스템 콜 테이블을 설정한다.
	set_grx_syscall();
	
	// 스크린 구조체를 0으로 초기화 하고 video mode 구조체를 복사한다.
	memset( &gui, 0, sizeof( gui ) );
	memcpy( &gui.vmode, pVM, sizeof( VESAModeStt ) );

	// 마우스 포인터
	init_mpointer();
	gui.mpointer.nH = MPOINTER_H;
	gui.mpointer.nV = MPOINTER_V;

	// gui timer를 초기화 한다.
	init_gui_timer( gui_timer_mesg_post_func );

	// load_cursor에서 동적 메모리 할당이 일어나지는 않는다.
	nI = load_cursor_set( &winres, &gui.mpointer.cursor_set, sys_cursor_id );
	// 최초로 디폴트 커서를 설정한다.
	gui.mpointer.pCurrentCursor = &gui.mpointer.cursor_set.ent[CSINDEX_ARROW];

	// 배경 무늬를 로드한다.  (kfree()로 해제하면 된다.)
	gui.pWallPaper = load_bitmap_image16( &winres, IDB_WALL_BLOCK );
	if( gui.pWallPaper == NULL )
	{
		kdbg_printf( "load wallpaper(IDB_WALL_BLOCK) failed!\n" );
		nR = -4;
		goto FREE_VOID_IMG;
	}  

	// system 아이콘을 로드한다.
	preload_sys_icon();

	// 화면 모드를 Text에서 Graphic 모드로 변환한다. 
	pBuff = set_vesa_mode( pVM, &dwSize );
	if( pBuff == NULL )
	{
		nR = -5;
		goto FREE_WALL;
	}

	set_system_gui_info( 1, &gui_exp );
	
	// VMWare에서 이 값이 0으로 넘어오는 수가 있다.  !!!
	if( gui.vmode.LinBytesPerScanLine == 0 )
	    gui.vmode.LinBytesPerScanLine = gui.vmode.wX * 2;  // 직접 계산해 준다.
	    
	// 비디오 메모리 주소와 크기 정보를 저장한다. 
	gui.dwMemSize = dwSize;
	gui.dwMemAddr = (DWORD)pBuff;

	// 비디오 메모리를 기본 색상으로 지운다.
	fill_gui_screen( (DWORD)pBuff, gui.vmode.wX, gui.vmode.wY, gui.vmode.LinBytesPerScanLine, WALL_BK_COLOR );

	// 커널 윈도우 쓰레드를 생성한다.
	pKWinThread = kcreate_win_thread( NULL );
	if( pKWinThread == NULL )
		goto FREE_WALL;

	// Predefined window style을 설정한다.  (별도의 해제 작업이 필요없고 여러번 콜해도 됨.)
	init_predef_winstyle();

	// 바탕화면 윈도우를 초기화한다.
	init_wall_win( pKWinThread , gui.vmode.wX, gui.vmode.wY );

	// 마우스를 최초로 한 번 그려준다. 	
	draw_mouse_pointer( 1 );
	
	// 마우스 CALL_BACK을 심는다.
	set_mouse_callback( gui_mouse_call_back );

	// 폰트 리소스를 로드한다. (특별히 해제할 필요가 없다.)
	load_system_fonts();

	// 바탕화면에 윈도우를 생성한다.
	create_icon_windows( pKWinThread, NULL );

	// Task Bar를 생성한다.
	r.nH = gui.wall.obj.r.nH;
	r.nV = TASKBAR_V;
	r.nX = 0;
	r.nY = gui.wall.obj.r.nV - r.nV;
	pTaskBar = kcreate_window( pKWinThread, NULL, WSTYLE_FLAT, &r, taskbar_marray, 0, 0, 0, 0, 0 );
	if( pTaskBar == NULL )
		kdbg_printf( "kcreate_window( TASKBAR ) : failed!\n" );

	// KCmd Window를 생성한다.
	// RECT는 안쪽에서 계산된다.
	calc_KCmdWin_size( &r, WSTYLE_SIMPLE );
	pKCmdWin = kcreate_window( pKWinThread, NULL, WSTYLE_SIMPLE, &r, 
		                       KCmdWin_marray, 0, 0, IDI_CMD_ICON, 0, 0 );
	if( pKCmdWin == NULL )
	{
		kdbg_printf( "kcreate_window( KCMD ) : failed!\n" );
		nR = -7;
		goto CLOSE_WIN_THREAD;
	}

	// About Window를 생성한다. 
	r.nH = GUI_ABOUT_H;
	r.nV = GUI_ABOUT_V;
	r.nX = ( gui.wall.obj.r.nH - r.nH ) / 2;
	r.nY = ( gui.wall.obj.r.nV - r.nV ) / 2;
	pAboutWin = kcreate_window( pKWinThread, NULL, WSTYLE_SIMPLE, &r, 
		        about_marray, WIN_STATE_MINIMIZED, WIN_ATTR_TRANSPARENT, IDI_ABOUT_ICON, 0, 0 );
	if( pAboutWin == NULL )
	{
		kdbg_printf( "kcreate_window( ABOUT ) : failed!\n" );
		nR = -8;
		goto CLOSE_WIN_THREAD;
	}
	
	// 바당화면 윈도우를 그려준다.
	flush_wall( NULL );

	return( 0 );

CLOSE_WIN_THREAD:
	kclose_win_thread( pKWinThread );
	pKWinThread = NULL;

FREE_WALL:
	kfree( gui.pWallPaper );
	gui.pWallPaper = NULL;

FREE_VOID_IMG:
	free_cursor_set( &gui.mpointer.cursor_set );

	return( nR );
}

// GUI 모드로부터 이탈한다.
int leave_gui()
{
	kdbg_printf( "leave_gui\n" );

	// gui timer를 종료한다.
	close_all_gui_timer();

	// 그래픽 시스템 콜 테이블을 설정한다.
	reset_grx_syscall();

	// Kernel Shell Window를 닫는다.
	call_win_message_handler( pKCmdWin, WMESG_DESTROY, 0, 0 );
	pKCmdWin = NULL;

	// 바탕화면의 아이콘 윈도우들을 닫는다.
	close_icon_windows();

	// Task Bar를 닫는다.
	call_win_message_handler( pTaskBar, WMESG_DESTROY, 0, 0 );
	pTaskBar = NULL;

	// About Window를 닫는다.
	call_win_message_handler( pAboutWin, WMESG_DESTROY, 0, 0 );
	pAboutWin = NULL;
	
	// kernel winq를 해제한다.
	kclose_win_thread( pKWinThread );
	pKWinThread = NULL;
		
	// 시스템 폰트를 해제한다.
	unload_system_fonts();	

	// 시스템 아이콘들을 해제한다.
	release_preload_sys_icon();
	
	// 마우스 콜백을 해제한다.
	set_mouse_callback( NULL );
	
	// 배경화면 타일
	if( gui.pWallPaper != NULL )
	{
		kfree( gui.pWallPaper );
		gui.pWallPaper = NULL;
	}
	
	// 커서 이미지
	free_cursor_set( &gui.mpointer.cursor_set );

	// 바탕화면 윈도우를 해제한다.
	close_gui_wall();
	
	// 화면을 text 50 line으로 설정한다.
	lines_xx( 50 );

	// gui 모드 플래그를 해제한다.
	set_system_gui_info( 0, NULL );

	return( 0 );
}

static int nMrff = 0;  // Move Rect Flip Flop

static int dot_rect( int nX, int nY, int nH, int nV )
{
	UINT16	*pW;
	GuiStt	*pGui;
	int		nXIndex, nYIndex, nFx;

	pGui = get_gui_stt();

	if( nX < 0 )
	{
		nH += nX;
		if( nH <= 0 )
			return( -1 );		// 그릴 수 없다.
		nX = 0;
	}
	else if( nX+nH >= (int)pGui->vmode.wX )
	{
		nH = (int)pGui->vmode.wX - nX;
		if( nH <= 0 )
			return( -1 );
	}
	
	if( nY < 0 )
	{
		nV += nY;
		if( nV <= 0 )
			return( -1 );
		nY = 0;
	}
	else if( nY+nV >= (int)pGui->vmode.wY )
	{
		nV = (int)pGui->vmode.wY - nY;
		if( nV <= 0 )
			return( -1 );
	}
	
	pW = get_video_mem_addr16( nX, nY );
	nFx = ( (nX+nY) & 1 );
	
	for( nYIndex = 0; nYIndex < nV; nYIndex++ )
	{
		for( nXIndex = 0; nXIndex < nH; nXIndex++ )
		{
			// 몽땅 그린다.
			if( nFx == 0 )
			{
				pW[ nXIndex ] = (UINT16)(~pW[nXIndex]);
				nFx = 1;
			}
			else
				nFx = 0;
		}
		
		if( nFx == 0 )
			nFx = 1;
		else
			nFx = 0;
			
		pW	= (UINT16*)( (DWORD)pW + (DWORD)pGui->vmode.LinBytesPerScanLine );
	}				 
	return( 0 );
}

// MOVE RECT를 보이거나 지운다. 
int show_move_rect( int nShowFlag )
{
	RectStt		*pR;
	GuiStt		*pGui;
	int			nFrameSize;

	pGui = get_gui_stt();
	if( pGui->nShowMoveFlag == nShowFlag )
		return( 0 );

	pGui->nShowMoveFlag = nShowFlag;

	// FlipFlop을 변경한다.
	if( nShowFlag != 0 )
	{
		if( nMrff == 0 ) 
			nMrff = 1;	
		else 
			nMrff = 0;
	}

	nFrameSize = 4;
	pR = &pGui->mr;	

	dot_rect( pR->nX, 					pR->nY, 					pR->nH, 	nFrameSize 			);
	dot_rect( pR->nX, 					pR->nY+pR->nV-nFrameSize,	pR->nH, 	nFrameSize 			);
	dot_rect( pR->nX, 					pR->nY+nFrameSize, 			nFrameSize, pR->nV-nFrameSize*2	);
	dot_rect( pR->nX+pR->nH-nFrameSize, pR->nY+nFrameSize, 			nFrameSize, pR->nV-nFrameSize*2	);

	return( 0 );
}

// 윈도우 이동 모드를 벗어난다. 
int leave_move_mode()
{					  	
	GuiStt	*pGui;

	pGui = get_gui_stt();

	// 현재 MOVE FRAME가 보여지고 있으면 지운다. 
	if( pGui->nShowMoveFlag != 0 )
		show_move_rect( 0 );

	pGui->nMoveResizeFlag = 0;
	pGui->nLockFlag       = 0;

	// 이동 메시지를 보낸다.
	kpost_message( pGui->pMoveWin, WMESG_WIN_MOVE, (DWORD)pGui->mr.nX, (DWORD)pGui->mr.nY );

	return( 0 );
}

// 윈도우 이동 모드로 들어간다.
int enter_move_mode( WinStt *pWin )
{
	MousePointerStt	*pMP;
	GuiStt			*pGui;
	WinFrameStt		*pFrm;

	pMP  = get_system_mpointer();
	pGui = get_gui_stt();
	pFrm = &pWin->pWStyle->frame;

	// 윈도우가 갱신되지 못하도록 락을 건다.
	pGui->nLockFlag 		= 1;
	pGui->nMoveResizeFlag 	= GUI_MOVE_MODE;
	pGui->pMoveWin  		= pWin;
	pGui->nMPX      		= pMP->nX;
	pGui->nMPY      		= pMP->nY;
	
	// 윈도우 크기를 복사한다. 
	memcpy( &pGui->mr, &pWin->obj.r, sizeof( RectStt ) );

	// 마우스 포인터를 가린 다음 Frame을 그려야 한다. 
	// MoveRect를 보여준다. 
	draw_mouse_pointer( 0 );
	show_move_rect( 1 );
	draw_mouse_pointer( 1 );

	return( 0 );
}

// 윈도우 크기 변경 모드로 들어간다.
int enter_resize_mode( WinStt *pWin, int nEdgeClip )
{
	MousePointerStt *pMP;
	GuiStt			*pGui;
	WinFrameStt 	*pFrm;

	pMP  = get_system_mpointer();
	pGui = get_gui_stt();
	pFrm = &pWin->pWStyle->frame;

	// 윈도우가 갱신되지 못하도록 락을 건다.
	pGui->nLockFlag 		= 1;
	pGui->nMoveResizeFlag	= GUI_RESIZE_MODE;
	pGui->nResizeEdgeClip   = nEdgeClip;
	pGui->pMoveWin			= pWin;
	pGui->nMPX				= pMP->nX;
	pGui->nMPY				= pMP->nY;
	
	// 윈도우 크기를 복사한다. 
	memcpy( &pGui->resize_org_r, &pWin->obj.r, sizeof( RectStt ) );
	memcpy( &pGui->mr, &pWin->obj.r, sizeof( RectStt ) );

	// 마우스 포인터를 가린 다음 Frame을 그려야 한다. 
	// MoveRect를 보여준다. 
	draw_mouse_pointer( 0 );
	show_move_rect( 1 );
	draw_mouse_pointer( 1 );

	return( 0 );
}

// 윈도우 이동 모드를 벗어난다. 
int leave_resize_mode()
{						
	GuiStt	*pGui;

	pGui = get_gui_stt();

	// 현재 MOVE FRAME가 보여지고 있으면 지운다. 
	if( pGui->nShowMoveFlag != 0 )
		show_move_rect( 0 );

	pGui->nMoveResizeFlag = 0;
	pGui->nLockFlag 	  = 0;

	// 커서를 복원한다.
	recover_resize_mouse_pointer( pGui->pMoveWin );

	// 이동 메시지를 보낸다.
	kpost_message( pGui->pMoveWin, WMESG_WIN_RESIZE, (DWORD)pGui->mr.nX, (DWORD)pGui->mr.nY );

	return( 0 );
}

// 마우스 포인터를 resize용에서 원래의 것으로 변경한다.
int recover_resize_mouse_pointer( WinStt *pWin )
{
	int nCurCSIndex;

	nCurCSIndex = get_mouse_pointer_index();
	if( nCurCSIndex != pWin->nOrgCursorIndex )
		set_mouse_pointer( pWin->nOrgCursorIndex );

	return( 0 );
}

// 마우스 포인터를 resize 용으로 변경한다.
int change_resize_mouse_pointer( WinStt *pWin, int nEdgeClip )
{
	int nNewCSIndex, nCurCSIndex;
	
	// 마우스 포인터를 변경한다.
	switch( nEdgeClip )
	{
		case ECLIP_UP_LEFT :
		case ECLIP_DOWN_RIGHT :
			nNewCSIndex = CSINDEX_RS_UL;
			break;
		case ECLIP_UP_RIGHT :
		case ECLIP_DOWN_LEFT :
			nNewCSIndex = CSINDEX_RS_UR;
			break;
		case ECLIP_UP_CENTER :
		case ECLIP_DOWN_CENTER :
			nNewCSIndex = CSINDEX_RS_V;
			break;
		case ECLIP_LEFT_CENTER :
		case ECLIP_RIGHT_CENTER :
			nNewCSIndex = CSINDEX_RS_H;
			break;
		default:
			return( 0 );
	}

	nCurCSIndex = get_mouse_pointer_index();
	if( nCurCSIndex == nNewCSIndex )
	{
		return( 0 );
	}
	
	//pWin->nOrgCursorIndex = nCurCSIndex;
	set_mouse_pointer( nNewCSIndex );

	//kdbg_printf( "change_resize_cursor: %d\n", nNewCSIndex );

	return( 0 );	
}

// Move Rect를 이동 시킨다. 
int move_rect_change_pos( int nX, int nY )
{
	MousePointerStt		*pMP;
	GuiStt				*pGui;
	int					nH, nV;

	pMP  = get_system_mpointer();
	pGui = get_gui_stt();
	nH = nX - pGui->nMPX;
	nV = nY - pGui->nMPY;

	// 기존 Move Frame을 지운다. 
	show_move_rect( 0 );

	// Move Frame의 위치를 바꾼다. 
	pGui->mr.nX += nH;
	pGui->mr.nY += nV;
	pGui->nMPX  = pMP->nX;
	pGui->nMPY  = pMP->nY;	 

	// Frame이 스크린을 벗어나면 안된다.   (벗어나도 되는것으로 수정함)
	/*
	if( pGui->mr.nX < 0 )
		pGui->mr.nX = 0;
	else if( pGui->mr.nX + pGui->mr.nH >= pGui->wall.obj.r.nH )
		pGui->mr.nX = pGui->wall.obj.r.nH - pGui->mr.nH;

	if( pGui->mr.nY < 0 )
		pGui->mr.nY = 0;
	else if( pGui->mr.nY + pGui->mr.nV >= pGui->wall.obj.r.nV )
		pGui->mr.nY = pGui->wall.obj.r.nV - pGui->mr.nV;
	*/
	// 새로 Move Frame을 그린다. 
	show_move_rect( 1 );	
	
	return( 0 );
}

int resize_rect_change_shape( int nX, int nY )
{
	MousePointerStt 	*pMP;
	RectStt				*pMR;
	GuiStt				*pGui;
	int					nDeltaH, nDeltaV;

	pGui = get_gui_stt();
	if( pGui->nMoveResizeFlag != GUI_RESIZE_MODE )
		return( 0 );

	pMP  = get_system_mpointer();
	pMR  = &pGui->mr;
	nDeltaH   = nX - pGui->nMPX;
	nDeltaV   = nY - pGui->nMPY;

	// 기존 Move Frame을 지운다. 
	show_move_rect( 0 );

	// Move Frame의 위치를 바꾼다. 
	switch( pGui->nResizeEdgeClip )
	{
		case ECLIP_UP_RIGHT :
			pMR->nY += nDeltaV;
			pMR->nV -= nDeltaV;
			pMR->nH += nDeltaH;
			break;
		case ECLIP_DOWN_RIGHT :
			pMR->nV += nDeltaV;
			pMR->nH += nDeltaH;
			break;
		case ECLIP_RIGHT_CENTER :
			pMR->nH += nDeltaH;
			break;
		case ECLIP_UP_LEFT :
			pMR->nX += nDeltaH;
			pMR->nH -= nDeltaH;
			pMR->nY += nDeltaV;
			pMR->nV -= nDeltaV;
			break;
		case ECLIP_DOWN_LEFT :
			pMR->nX += nDeltaH;
			pMR->nH -= nDeltaH;
			pMR->nV += nDeltaV;
			break;
		case ECLIP_LEFT_CENTER :
			pMR->nX += nDeltaH;
			pMR->nH -= nDeltaH;
			break;
		case ECLIP_UP_CENTER :
			pMR->nY += nDeltaV;
			pMR->nV -= nDeltaV;
			break;
		case ECLIP_DOWN_CENTER :
			pMR->nV += nDeltaV;
			break;
	}

	pGui->nMPX	= pMP->nX;
	pGui->nMPY	= pMP->nY;	 

	// 새로 Move Frame을 그린다. 
	show_move_rect( 1 );	
	
	return( 0 );
}


// pWin 윈도우 보다 낮은 레이어의 윈도우들을 다시 그린다.
// 위치가 변경되었을 때 이전 영역을 다시 그릴 때 사용한다.
// pR = 절대 좌표.
int repaint_down_layer( WinStt *pWin, RectStt *pR )
{
	int			nR;
	GuiStt		*pGui;
	WinStt		*pCurWin;

	pGui = get_gui_stt();

	// 마우스 포인터를 지운다.
	draw_mouse_pointer( 0 );

	// 배경화면을 다시 그린다.
	flush_wall( pR );

	// pWin 아래쪽에 위치한 윈도우 가운데 영역 pR과 겹치는 부분이 있는 윈도우만 그린다.
	// Bottom Level 부터 pWin 직전까지 그려 올라온다.
	//for( pCurWin = pWin->pNextLevel; pCurWin != NULL; pCurWin = pCurWin->pNextLevel )
	for( pCurWin = pGui->pEndLevelWin; pCurWin != NULL && pCurWin != pWin; pCurWin = pCurWin->pPreLevel )
	{	
		// minimize되어 있으면 메시지를 날리지 않는다.
		if( pCurWin->gb.dwState & WIN_STATE_MINIMIZED || pCurWin->gb.dwState & WIN_STATE_HIDDEN )
			continue;
		
		// &r이 지역변수의 포인터 이므로 반드시 send로 처리할 것.
		// 겹치는 영역이 있는 윈도우만 다시 그린다.
		{
			RectStt		r, rx;
			nR = get_overlapped_rect( &r, &pCurWin->obj.r, pR );
			if( nR == 0 )
				continue;	// 겹치는 부분이 없으면 통과.
			// 윈도우의 좌표로 변환하여 PAINT 메시지 핸들러를 호출한다.&
			screen_to_win( &rx, &pCurWin->obj.r, &r );

			// ksend_message를 쓰지 말고 직접 PAINT함수를 호출하는 kforward_message를 이용.
 			kforward_message( pCurWin, WMESG_PAINT, rect_xy_to_dword( &rx), rect_hv_to_dword( &rx) );
		}
 	}		

	draw_mouse_pointer( 1 );

	return( 0 );
}

// pWin 윈도우 보다 높은 레이어의 TRANSPARENT 윈도우들을 다시 그린다.
// pR = 절대 좌표.
int repaint_upper_transparent( WinStt *pWin, RectStt *pWR )
{
	int			nR;
	GuiStt		*pGui;
	WinStt		*pCurWin;

	pGui = get_gui_stt();

	draw_mouse_pointer( 0 );

	// Top Level부터 현재 윈도우 바로 위에 존재하는 TRANSPARENT 윈도우만 다시 그린다.
	for( pCurWin = pGui->pStartLevelWin; pCurWin != NULL && pCurWin != pWin; pCurWin = pCurWin->pNextLevel )
	{	
		// minimize되어 있으면 메시지를 날리지 않는다.
		if( pCurWin->gb.dwState & WIN_STATE_MINIMIZED || pCurWin->gb.dwState & WIN_STATE_HIDDEN)
			continue;

		if( (pCurWin->gb.dwAttr & WIN_ATTR_TRANSPARENT ) == 0 )
			continue;
		
		// &r이 지역변수의 포인터 이므로 반드시 send로 처리할 것.
		// 겹치는 영역이 있는 윈도우만 다시 그린다.
		{
			RectStt		r, rx;
			nR = get_overlapped_rect( &r, &pCurWin->obj.r, pWR );
			if( nR == 0 )
				continue;	// 겹치는 부분이 없으면 통과.
			
			// 윈도우의 좌표로 변환하여 PAINT 메시지 핸들러를 호출한다.&
			screen_to_win( &rx, &pCurWin->obj.r, &r );

			// ksend_message를 쓰지 말고 직접 PAINT함수를 호출하는 kforward_message를 이용.
 			kforward_message( pCurWin, WMESG_PAINT, rect_xy_to_dword( &rx), rect_hv_to_dword( &rx) );
		}
 	}		

	draw_mouse_pointer( 1 );

	return( 0 );
}

static int link_gui_obj( GuiObjStt *pOwner, GuiObjStt *pObj )
{
	// Owner, OBJ가 올바른 것인지 확인한다.
	if( pOwner == NULL || pObj == NULL || pObj->wMagic != GUI_OBJ_MAGIC || pOwner->wMagic != GUI_OBJ_MAGIC )
		return( -1 );

	/////////////////////////////////////////////////////////////////
	//if( pOwner->wType == GUI_OTYPE_WINDOW )
	//{
	//	WinStt *pWin;
	//	pWin = (WinStt*)pOwner;
	//	kdbg_printf( "Owner: %s\n", pWin->szTitle );
	//}
	//else
	//	kdbg_printf( "Owner: UNKNOWN\n" );
	//kdbg_printf( "GuiObj: 0x%X (%d, %d, %d, %d)\n", pObj, pObj->r.nX, pObj->r.nY, pObj->r.nH, pObj->r.nV );
	///////////////////////////////////////////////////////////////
		
	if( pOwner->pStart == NULL )
	{	// 링크의 가장 처음에 추가한다.
		pOwner->pStart = pOwner->pEnd = pObj;
		pObj->pPre = pObj->pNext = NULL;
	}
	else
	{	// 링크의 가장 뒤에 연결한다.
		pObj->pPre          = pOwner->pEnd;
		pObj->pNext         = NULL;
		pOwner->pEnd->pNext = pObj;
		pOwner->pEnd        = pObj;
	}

	pObj->pOwner = pOwner;
	
	return( 0 );
}

static int unlink_gui_obj( GuiObjStt *pObj )
{
	GuiObjStt *pOwner;

	// OBJ가 올바른 것인지 확인한다.
	if( pObj == NULL || pObj->wMagic != GUI_OBJ_MAGIC )
		return( -1 );

	// Owner가 없으면 unlink할 필요가 없다.
	if( pObj->pOwner == NULL )
		return( 0 );

	// Parent가 올바른 것인지 확인한다.
	pOwner = pObj->pOwner;
	if( pOwner->wMagic != GUI_OBJ_MAGIC )
		return( -1 );

	// 링크에서 분리한다.
	if( pObj->pPre == NULL )
		pOwner->pStart = pObj->pNext;
	else pObj->pPre->pNext = pObj->pNext;

	if( pObj->pNext == NULL )
		pOwner->pEnd = pObj->pPre;
	else 
		pObj->pNext->pPre = pObj->pPre;
	
	pObj->pPre = pObj->pNext = pObj->pOwner = NULL;

	// 현재 Obj가 Focus를 잡고 있었으면 Focus 포인터를 NULL로 한다.
	if( pOwner->pFocus == pObj )
		pOwner->pFocus = NULL;
	if( pOwner->pCat == pObj )
		pOwner->pCat = NULL;
	
	return( 0 );
}	  

// GUI Object를 초기화 한다.
int init_gui_obj( GuiObjStt *pOwner, GuiObjStt *pObj, UINT16 wType, GuiObjFuncStt *pFuncArray )
{
	memset( pObj, 0, sizeof( GuiObjStt ) );
	pObj->wMagic = GUI_OBJ_MAGIC;
	pObj->wType = wType;
	pObj->pFuncArray = pFuncArray;
	
	// Owner가 없으면 상위 링크에 추가하지 않는다.
	if( pOwner == NULL )
	{
		pObj->pOwner = NULL;
		return( 0 );
	}

	link_gui_obj( pOwner, pObj );	
	
	return( 0 );
}

int close_gui_obj( GuiObjStt *pObj )
{
	int nR;

	nR = unlink_gui_obj( pObj );

	return( nR );
}

DWORD rect_xy_to_dword( RectStt *pR )
{
	DWORD dwR;

	dwR = (DWORD)( ((DWORD)pR->nX << 16) | ((DWORD)pR->nY & (DWORD)0xFFFF) );

	return( dwR );
}

DWORD rect_hv_to_dword( RectStt *pR )
{
	DWORD dwR;

	dwR = (DWORD)( ((DWORD)pR->nH << 16) | ((DWORD)pR->nV & (DWORD)0xFFFF) );

	return( dwR );
}

void dword_to_rect_hv( DWORD dwR, RectStt *pR )
{
	pR->nH = (int)(dwR >> 16);
	pR->nV = (int)(dwR & 0xFFFF);
}

void dword_to_rect_xy( DWORD dwR, RectStt *pR )
{
	pR->nX = (int)(dwR >> 16);
	pR->nY = (int)(dwR & 0xFFFF);
}

