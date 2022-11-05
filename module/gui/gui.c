//  GUI Extension Module for Bellona2 Kernel (2002-05-12 )
//  ��ũ�� �� __chkesp �ɺ��� ���ٰ� ������ ����� "/GZ" ������ �ɼ��� �����ؾ� �Ѵ�.

#include <bellona2.h>
#include "gui.h"

extern WMFuncStt iconwin_marray[];
extern WMFuncStt taskbar_marray[];
extern WMFuncStt about_marray[];
extern WMFuncStt KCmdWin_marray[];


// ��ũ�� ������ ����
static GuiStt	    gui;			// ��ũ��, �ػ�, Video Mode�� ���� ����.
static WinResStt	winres;			// ����� ���ҽ�(������, ��Ʈ��, Ŀ��, ��Ʈ...)
static ModuleStt	*pModule;		// ��� �ڽ��� �ڵ�.
static WinStt		*pAboutWin;		// Bellona2 Gui�� About window
static WinStt		*pTaskBar;		// Bellona2 Gui�� Task Bar.
static WinStt		*pKCmdWin;		// Kernel Shell Window.
static WinThreadStt	*pKWinThread;	// Ŀ�� ��������� �޽����� ó���ϴ� ������.
static ThreadStt	*pGuiThread;	// GUI System ��ü�� �����ϴ� ������.

extern void set_grx_syscall();
extern void reset_grx_syscall();

int leave_gui();

GuiStt *get_gui_stt() { return( &gui ); }
WinResStt *get_winres_stt() { return( &winres ); }
WinThreadStt *get_kwin_thread() { return( pKWinThread ); }

/////////////////////////////////////////////////////////
// ALT-F4�� ������ �� �ñ׳��� �޾Ƽ� ó���ϴ� ������  //
/////////////////////////////////////////////////////////

// �ñ׳� ���� ����� �������� ���� ALT-F4�� �������� �ʴ´�.
int gui_thread( void *pPram )
{
//	DWORD dwSig;
	
	for( ;; )
	{	
		// �������� ��� �ñ׳��� Ŭ���� �Ѵ�.
		//clear_thread_signal_bits( NULL, 0xFFFFFFFF );

		// �ñ׳��� �����⸦ ��ٸ���.
		kpause();		

		// ���� �ñ׳��� ó���Ѵ�.
		//dwSig = get_thread_signal_bits( NULL );

		// �ñ׳� ��Ʈ�� Ŭ�����Ѵ�.
		//clear_thread_signal_bits( NULL, dwSig );
		//if( dwSig & SIG_USER_0 )
		//{	
		//	// Wall Window�� CLOSE �޽����� ������.
		//	kpost_message( get_wall_win(), WMESG_CLOSE, 0, 0 );
		//}		
		//else if( dwSig & SIG_USER_1 )
		//{
		//	// GUI system ����.
		//	leave_gui();	
		//}
	}	
}

// ����� �ε��Ǵ� �������� ȣ��ȴ�.  �ʱ�ȭ�� ����Ѵ�.
/////////////////////////////// GUI MAIN ////////////////////////////
/**/ int gui_main( ModuleStt *pCurModule, int argc, char* argv[] ) //
/////////////////////////////////////////////////////////////////////
{
	WinResStt		*pWR;
	WinResEntStt	*pEnt;
	int				nI, nR;

	pModule = pCurModule;
	kdbg_printf( "B2OS GUI Module (c) 2003 OHJJ\n" );

	// �ڷᱸ���� 0���� Ŭ�����Ѵ�. 
	memset( &gui, 0, sizeof( gui ) );
	
	// ���ҽ��� �ν��Ѵ�. 
	memset( &winres, 0, sizeof( winres ) );
	nR = win_resource( pCurModule->dwLoadAddr, &winres );
	if( nR < 0 )
		kdbg_printf( "No resource.\n" );
	else
	{	// ���ҽ��� ����Ѵ�. 
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
	// gui thread�� �����Ѵ�. (ALT-F4�� ������ �� �� �����忡�� ó���ȴ�.)
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

// x, y�� Rect ���� ���ԵǾ� �ִ°�?
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

// ���� �޸� ��ü�� wColor�� ä���.
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

		// �� �̹����� �Ҵ��Ͽ� bitmap�� �̹����� ��ȯ�Ѵ�.  (kfree()�� �����ϸ� �ȴ�.)
		pCE->pVoidImg = bitmap_to_image16( pCE->pBit, MPOINTER_H, MPOINTER_V );
		// Ŀ�� �ε����� �����Ѵ�.
		pCE->nIndex = nI;
	}
	
	// Ŀ���� �׷��� ���� ��� �̹����� ������ �� ���� �Ҵ��Ѵ�. (kfree()�� �����ϸ� �ȴ�.) 
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

// GUI ���� �����Ѵ�. 
int enter_gui( VESAModeStt *pVM )
{
	RectStt		r;
	int			nI, nR;
	BYTE		*pBuff;
	DWORD		dwSize;
		
	kdbg_printf( "enter_gui\n" );

	// �׷��� �ý��� �� ���̺��� �����Ѵ�.
	set_grx_syscall();
	
	// ��ũ�� ����ü�� 0���� �ʱ�ȭ �ϰ� video mode ����ü�� �����Ѵ�.
	memset( &gui, 0, sizeof( gui ) );
	memcpy( &gui.vmode, pVM, sizeof( VESAModeStt ) );

	// ���콺 ������
	init_mpointer();
	gui.mpointer.nH = MPOINTER_H;
	gui.mpointer.nV = MPOINTER_V;

	// gui timer�� �ʱ�ȭ �Ѵ�.
	init_gui_timer( gui_timer_mesg_post_func );

	// load_cursor���� ���� �޸� �Ҵ��� �Ͼ���� �ʴ´�.
	nI = load_cursor_set( &winres, &gui.mpointer.cursor_set, sys_cursor_id );
	// ���ʷ� ����Ʈ Ŀ���� �����Ѵ�.
	gui.mpointer.pCurrentCursor = &gui.mpointer.cursor_set.ent[CSINDEX_ARROW];

	// ��� ���̸� �ε��Ѵ�.  (kfree()�� �����ϸ� �ȴ�.)
	gui.pWallPaper = load_bitmap_image16( &winres, IDB_WALL_BLOCK );
	if( gui.pWallPaper == NULL )
	{
		kdbg_printf( "load wallpaper(IDB_WALL_BLOCK) failed!\n" );
		nR = -4;
		goto FREE_VOID_IMG;
	}  

	// system �������� �ε��Ѵ�.
	preload_sys_icon();

	// ȭ�� ��带 Text���� Graphic ���� ��ȯ�Ѵ�. 
	pBuff = set_vesa_mode( pVM, &dwSize );
	if( pBuff == NULL )
	{
		nR = -5;
		goto FREE_WALL;
	}

	set_system_gui_info( 1, &gui_exp );
	
	// VMWare���� �� ���� 0���� �Ѿ���� ���� �ִ�.  !!!
	if( gui.vmode.LinBytesPerScanLine == 0 )
	    gui.vmode.LinBytesPerScanLine = gui.vmode.wX * 2;  // ���� ����� �ش�.
	    
	// ���� �޸� �ּҿ� ũ�� ������ �����Ѵ�. 
	gui.dwMemSize = dwSize;
	gui.dwMemAddr = (DWORD)pBuff;

	// ���� �޸𸮸� �⺻ �������� �����.
	fill_gui_screen( (DWORD)pBuff, gui.vmode.wX, gui.vmode.wY, gui.vmode.LinBytesPerScanLine, WALL_BK_COLOR );

	// Ŀ�� ������ �����带 �����Ѵ�.
	pKWinThread = kcreate_win_thread( NULL );
	if( pKWinThread == NULL )
		goto FREE_WALL;

	// Predefined window style�� �����Ѵ�.  (������ ���� �۾��� �ʿ���� ������ ���ص� ��.)
	init_predef_winstyle();

	// ����ȭ�� �����츦 �ʱ�ȭ�Ѵ�.
	init_wall_win( pKWinThread , gui.vmode.wX, gui.vmode.wY );

	// ���콺�� ���ʷ� �� �� �׷��ش�. 	
	draw_mouse_pointer( 1 );
	
	// ���콺 CALL_BACK�� �ɴ´�.
	set_mouse_callback( gui_mouse_call_back );

	// ��Ʈ ���ҽ��� �ε��Ѵ�. (Ư���� ������ �ʿ䰡 ����.)
	load_system_fonts();

	// ����ȭ�鿡 �����츦 �����Ѵ�.
	create_icon_windows( pKWinThread, NULL );

	// Task Bar�� �����Ѵ�.
	r.nH = gui.wall.obj.r.nH;
	r.nV = TASKBAR_V;
	r.nX = 0;
	r.nY = gui.wall.obj.r.nV - r.nV;
	pTaskBar = kcreate_window( pKWinThread, NULL, WSTYLE_FLAT, &r, taskbar_marray, 0, 0, 0, 0, 0 );
	if( pTaskBar == NULL )
		kdbg_printf( "kcreate_window( TASKBAR ) : failed!\n" );

	// KCmd Window�� �����Ѵ�.
	// RECT�� ���ʿ��� ���ȴ�.
	calc_KCmdWin_size( &r, WSTYLE_SIMPLE );
	pKCmdWin = kcreate_window( pKWinThread, NULL, WSTYLE_SIMPLE, &r, 
		                       KCmdWin_marray, 0, 0, IDI_CMD_ICON, 0, 0 );
	if( pKCmdWin == NULL )
	{
		kdbg_printf( "kcreate_window( KCMD ) : failed!\n" );
		nR = -7;
		goto CLOSE_WIN_THREAD;
	}

	// About Window�� �����Ѵ�. 
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
	
	// �ٴ�ȭ�� �����츦 �׷��ش�.
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

// GUI ���κ��� ��Ż�Ѵ�.
int leave_gui()
{
	kdbg_printf( "leave_gui\n" );

	// gui timer�� �����Ѵ�.
	close_all_gui_timer();

	// �׷��� �ý��� �� ���̺��� �����Ѵ�.
	reset_grx_syscall();

	// Kernel Shell Window�� �ݴ´�.
	call_win_message_handler( pKCmdWin, WMESG_DESTROY, 0, 0 );
	pKCmdWin = NULL;

	// ����ȭ���� ������ ��������� �ݴ´�.
	close_icon_windows();

	// Task Bar�� �ݴ´�.
	call_win_message_handler( pTaskBar, WMESG_DESTROY, 0, 0 );
	pTaskBar = NULL;

	// About Window�� �ݴ´�.
	call_win_message_handler( pAboutWin, WMESG_DESTROY, 0, 0 );
	pAboutWin = NULL;
	
	// kernel winq�� �����Ѵ�.
	kclose_win_thread( pKWinThread );
	pKWinThread = NULL;
		
	// �ý��� ��Ʈ�� �����Ѵ�.
	unload_system_fonts();	

	// �ý��� �����ܵ��� �����Ѵ�.
	release_preload_sys_icon();
	
	// ���콺 �ݹ��� �����Ѵ�.
	set_mouse_callback( NULL );
	
	// ���ȭ�� Ÿ��
	if( gui.pWallPaper != NULL )
	{
		kfree( gui.pWallPaper );
		gui.pWallPaper = NULL;
	}
	
	// Ŀ�� �̹���
	free_cursor_set( &gui.mpointer.cursor_set );

	// ����ȭ�� �����츦 �����Ѵ�.
	close_gui_wall();
	
	// ȭ���� text 50 line���� �����Ѵ�.
	lines_xx( 50 );

	// gui ��� �÷��׸� �����Ѵ�.
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
			return( -1 );		// �׸� �� ����.
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
			// ���� �׸���.
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

// MOVE RECT�� ���̰ų� �����. 
int show_move_rect( int nShowFlag )
{
	RectStt		*pR;
	GuiStt		*pGui;
	int			nFrameSize;

	pGui = get_gui_stt();
	if( pGui->nShowMoveFlag == nShowFlag )
		return( 0 );

	pGui->nShowMoveFlag = nShowFlag;

	// FlipFlop�� �����Ѵ�.
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

// ������ �̵� ��带 �����. 
int leave_move_mode()
{					  	
	GuiStt	*pGui;

	pGui = get_gui_stt();

	// ���� MOVE FRAME�� �������� ������ �����. 
	if( pGui->nShowMoveFlag != 0 )
		show_move_rect( 0 );

	pGui->nMoveResizeFlag = 0;
	pGui->nLockFlag       = 0;

	// �̵� �޽����� ������.
	kpost_message( pGui->pMoveWin, WMESG_WIN_MOVE, (DWORD)pGui->mr.nX, (DWORD)pGui->mr.nY );

	return( 0 );
}

// ������ �̵� ���� ����.
int enter_move_mode( WinStt *pWin )
{
	MousePointerStt	*pMP;
	GuiStt			*pGui;
	WinFrameStt		*pFrm;

	pMP  = get_system_mpointer();
	pGui = get_gui_stt();
	pFrm = &pWin->pWStyle->frame;

	// �����찡 ���ŵ��� ���ϵ��� ���� �Ǵ�.
	pGui->nLockFlag 		= 1;
	pGui->nMoveResizeFlag 	= GUI_MOVE_MODE;
	pGui->pMoveWin  		= pWin;
	pGui->nMPX      		= pMP->nX;
	pGui->nMPY      		= pMP->nY;
	
	// ������ ũ�⸦ �����Ѵ�. 
	memcpy( &pGui->mr, &pWin->obj.r, sizeof( RectStt ) );

	// ���콺 �����͸� ���� ���� Frame�� �׷��� �Ѵ�. 
	// MoveRect�� �����ش�. 
	draw_mouse_pointer( 0 );
	show_move_rect( 1 );
	draw_mouse_pointer( 1 );

	return( 0 );
}

// ������ ũ�� ���� ���� ����.
int enter_resize_mode( WinStt *pWin, int nEdgeClip )
{
	MousePointerStt *pMP;
	GuiStt			*pGui;
	WinFrameStt 	*pFrm;

	pMP  = get_system_mpointer();
	pGui = get_gui_stt();
	pFrm = &pWin->pWStyle->frame;

	// �����찡 ���ŵ��� ���ϵ��� ���� �Ǵ�.
	pGui->nLockFlag 		= 1;
	pGui->nMoveResizeFlag	= GUI_RESIZE_MODE;
	pGui->nResizeEdgeClip   = nEdgeClip;
	pGui->pMoveWin			= pWin;
	pGui->nMPX				= pMP->nX;
	pGui->nMPY				= pMP->nY;
	
	// ������ ũ�⸦ �����Ѵ�. 
	memcpy( &pGui->resize_org_r, &pWin->obj.r, sizeof( RectStt ) );
	memcpy( &pGui->mr, &pWin->obj.r, sizeof( RectStt ) );

	// ���콺 �����͸� ���� ���� Frame�� �׷��� �Ѵ�. 
	// MoveRect�� �����ش�. 
	draw_mouse_pointer( 0 );
	show_move_rect( 1 );
	draw_mouse_pointer( 1 );

	return( 0 );
}

// ������ �̵� ��带 �����. 
int leave_resize_mode()
{						
	GuiStt	*pGui;

	pGui = get_gui_stt();

	// ���� MOVE FRAME�� �������� ������ �����. 
	if( pGui->nShowMoveFlag != 0 )
		show_move_rect( 0 );

	pGui->nMoveResizeFlag = 0;
	pGui->nLockFlag 	  = 0;

	// Ŀ���� �����Ѵ�.
	recover_resize_mouse_pointer( pGui->pMoveWin );

	// �̵� �޽����� ������.
	kpost_message( pGui->pMoveWin, WMESG_WIN_RESIZE, (DWORD)pGui->mr.nX, (DWORD)pGui->mr.nY );

	return( 0 );
}

// ���콺 �����͸� resize�뿡�� ������ ������ �����Ѵ�.
int recover_resize_mouse_pointer( WinStt *pWin )
{
	int nCurCSIndex;

	nCurCSIndex = get_mouse_pointer_index();
	if( nCurCSIndex != pWin->nOrgCursorIndex )
		set_mouse_pointer( pWin->nOrgCursorIndex );

	return( 0 );
}

// ���콺 �����͸� resize ������ �����Ѵ�.
int change_resize_mouse_pointer( WinStt *pWin, int nEdgeClip )
{
	int nNewCSIndex, nCurCSIndex;
	
	// ���콺 �����͸� �����Ѵ�.
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

// Move Rect�� �̵� ��Ų��. 
int move_rect_change_pos( int nX, int nY )
{
	MousePointerStt		*pMP;
	GuiStt				*pGui;
	int					nH, nV;

	pMP  = get_system_mpointer();
	pGui = get_gui_stt();
	nH = nX - pGui->nMPX;
	nV = nY - pGui->nMPY;

	// ���� Move Frame�� �����. 
	show_move_rect( 0 );

	// Move Frame�� ��ġ�� �ٲ۴�. 
	pGui->mr.nX += nH;
	pGui->mr.nY += nV;
	pGui->nMPX  = pMP->nX;
	pGui->nMPY  = pMP->nY;	 

	// Frame�� ��ũ���� ����� �ȵȴ�.   (����� �Ǵ°����� ������)
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
	// ���� Move Frame�� �׸���. 
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

	// ���� Move Frame�� �����. 
	show_move_rect( 0 );

	// Move Frame�� ��ġ�� �ٲ۴�. 
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

	// ���� Move Frame�� �׸���. 
	show_move_rect( 1 );	
	
	return( 0 );
}


// pWin ������ ���� ���� ���̾��� ��������� �ٽ� �׸���.
// ��ġ�� ����Ǿ��� �� ���� ������ �ٽ� �׸� �� ����Ѵ�.
// pR = ���� ��ǥ.
int repaint_down_layer( WinStt *pWin, RectStt *pR )
{
	int			nR;
	GuiStt		*pGui;
	WinStt		*pCurWin;

	pGui = get_gui_stt();

	// ���콺 �����͸� �����.
	draw_mouse_pointer( 0 );

	// ���ȭ���� �ٽ� �׸���.
	flush_wall( pR );

	// pWin �Ʒ��ʿ� ��ġ�� ������ ��� ���� pR�� ��ġ�� �κ��� �ִ� �����츸 �׸���.
	// Bottom Level ���� pWin �������� �׷� �ö�´�.
	//for( pCurWin = pWin->pNextLevel; pCurWin != NULL; pCurWin = pCurWin->pNextLevel )
	for( pCurWin = pGui->pEndLevelWin; pCurWin != NULL && pCurWin != pWin; pCurWin = pCurWin->pPreLevel )
	{	
		// minimize�Ǿ� ������ �޽����� ������ �ʴ´�.
		if( pCurWin->gb.dwState & WIN_STATE_MINIMIZED || pCurWin->gb.dwState & WIN_STATE_HIDDEN )
			continue;
		
		// &r�� ���������� ������ �̹Ƿ� �ݵ�� send�� ó���� ��.
		// ��ġ�� ������ �ִ� �����츸 �ٽ� �׸���.
		{
			RectStt		r, rx;
			nR = get_overlapped_rect( &r, &pCurWin->obj.r, pR );
			if( nR == 0 )
				continue;	// ��ġ�� �κ��� ������ ���.
			// �������� ��ǥ�� ��ȯ�Ͽ� PAINT �޽��� �ڵ鷯�� ȣ���Ѵ�.&
			screen_to_win( &rx, &pCurWin->obj.r, &r );

			// ksend_message�� ���� ���� ���� PAINT�Լ��� ȣ���ϴ� kforward_message�� �̿�.
 			kforward_message( pCurWin, WMESG_PAINT, rect_xy_to_dword( &rx), rect_hv_to_dword( &rx) );
		}
 	}		

	draw_mouse_pointer( 1 );

	return( 0 );
}

// pWin ������ ���� ���� ���̾��� TRANSPARENT ��������� �ٽ� �׸���.
// pR = ���� ��ǥ.
int repaint_upper_transparent( WinStt *pWin, RectStt *pWR )
{
	int			nR;
	GuiStt		*pGui;
	WinStt		*pCurWin;

	pGui = get_gui_stt();

	draw_mouse_pointer( 0 );

	// Top Level���� ���� ������ �ٷ� ���� �����ϴ� TRANSPARENT �����츸 �ٽ� �׸���.
	for( pCurWin = pGui->pStartLevelWin; pCurWin != NULL && pCurWin != pWin; pCurWin = pCurWin->pNextLevel )
	{	
		// minimize�Ǿ� ������ �޽����� ������ �ʴ´�.
		if( pCurWin->gb.dwState & WIN_STATE_MINIMIZED || pCurWin->gb.dwState & WIN_STATE_HIDDEN)
			continue;

		if( (pCurWin->gb.dwAttr & WIN_ATTR_TRANSPARENT ) == 0 )
			continue;
		
		// &r�� ���������� ������ �̹Ƿ� �ݵ�� send�� ó���� ��.
		// ��ġ�� ������ �ִ� �����츸 �ٽ� �׸���.
		{
			RectStt		r, rx;
			nR = get_overlapped_rect( &r, &pCurWin->obj.r, pWR );
			if( nR == 0 )
				continue;	// ��ġ�� �κ��� ������ ���.
			
			// �������� ��ǥ�� ��ȯ�Ͽ� PAINT �޽��� �ڵ鷯�� ȣ���Ѵ�.&
			screen_to_win( &rx, &pCurWin->obj.r, &r );

			// ksend_message�� ���� ���� ���� PAINT�Լ��� ȣ���ϴ� kforward_message�� �̿�.
 			kforward_message( pCurWin, WMESG_PAINT, rect_xy_to_dword( &rx), rect_hv_to_dword( &rx) );
		}
 	}		

	draw_mouse_pointer( 1 );

	return( 0 );
}

static int link_gui_obj( GuiObjStt *pOwner, GuiObjStt *pObj )
{
	// Owner, OBJ�� �ùٸ� ������ Ȯ���Ѵ�.
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
	{	// ��ũ�� ���� ó���� �߰��Ѵ�.
		pOwner->pStart = pOwner->pEnd = pObj;
		pObj->pPre = pObj->pNext = NULL;
	}
	else
	{	// ��ũ�� ���� �ڿ� �����Ѵ�.
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

	// OBJ�� �ùٸ� ������ Ȯ���Ѵ�.
	if( pObj == NULL || pObj->wMagic != GUI_OBJ_MAGIC )
		return( -1 );

	// Owner�� ������ unlink�� �ʿ䰡 ����.
	if( pObj->pOwner == NULL )
		return( 0 );

	// Parent�� �ùٸ� ������ Ȯ���Ѵ�.
	pOwner = pObj->pOwner;
	if( pOwner->wMagic != GUI_OBJ_MAGIC )
		return( -1 );

	// ��ũ���� �и��Ѵ�.
	if( pObj->pPre == NULL )
		pOwner->pStart = pObj->pNext;
	else pObj->pPre->pNext = pObj->pNext;

	if( pObj->pNext == NULL )
		pOwner->pEnd = pObj->pPre;
	else 
		pObj->pNext->pPre = pObj->pPre;
	
	pObj->pPre = pObj->pNext = pObj->pOwner = NULL;

	// ���� Obj�� Focus�� ��� �־����� Focus �����͸� NULL�� �Ѵ�.
	if( pOwner->pFocus == pObj )
		pOwner->pFocus = NULL;
	if( pOwner->pCat == pObj )
		pOwner->pCat = NULL;
	
	return( 0 );
}	  

// GUI Object�� �ʱ�ȭ �Ѵ�.
int init_gui_obj( GuiObjStt *pOwner, GuiObjStt *pObj, UINT16 wType, GuiObjFuncStt *pFuncArray )
{
	memset( pObj, 0, sizeof( GuiObjStt ) );
	pObj->wMagic = GUI_OBJ_MAGIC;
	pObj->wType = wType;
	pObj->pFuncArray = pFuncArray;
	
	// Owner�� ������ ���� ��ũ�� �߰����� �ʴ´�.
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

