#include <bellona2.h>
#include "gui.h"

// wall.c 내부적으로 사용한다.
#define MBOX_ID_EXIT  100

// gui.c의 gui_thread에서 SIG_USER_0를 받으면 WMESG_CLOSE를 wall 윈도우로 날린다.
// 메시지가 여러 번 날아와도 메시지 박스가 이미생성되어 있으면 다시 날리지 않는다.
static DWORD wmh_wall_close( WinStt *pWin, DWORD dwWMesgID, DWORD dwParamA, DWORD dwParamB )
{
	WallPrivateStt	*pPrivate;

	kdbg_printf( "wmh_wall_close\n" );
	
	pPrivate = (WallPrivateStt*)pWin->pPrivate;
	if( pPrivate == NULL )
	{
		kdbg_printf( "wmh_wall_close: pPrivate is NULL!\n" );
		return( WMHRV_ABORT );
	}

	// 메시지 박스가 이미 떠 있으면 그냥 리턴한다.
	if( pPrivate->pExitMBox != NULL )
	{	// 압으로 설정한다.
		set_top_window( pPrivate->pExitMBox );
		return( WMHRV_ABORT );
	}

	// 메시지 박스를 생성한다.	
	pPrivate->pExitMBox = message_box( pWin, MBOX_ID_EXIT, "Really want to quit?", "Question", 200, 150, 
		MBOX_BUTTON_OK | MBOX_BUTTON_CANCEL );

	return( WMHRV_ABORT );
}

static DWORD wmh_wall_destroy( WinStt *pWin, DWORD dwWMesgID, DWORD dwParamA, DWORD dwParamB )
{
	// Private 영역을 해제한다.
	if( pWin->pPrivate != NULL )
	{
		kfree( pWin->pPrivate );
		pWin->pPrivate = NULL;
	}
	
	return( WMHRV_CONTINUE );
}

static DWORD wmh_wall_modal( WinStt *pWin, DWORD dwWMesgID, DWORD dwMBoxID, DWORD dwBtnID )
{
	ThreadStt		*pT;
	WallPrivateStt *pPrivate;

	pPrivate = (WallPrivateStt*)pWin->pPrivate;

	if( dwMBoxID == MBOX_ID_EXIT )
	{	
		if( dwBtnID == MBOX_BUTTON_OK )
		{
			// gui_thread 쪽으로 SIG_USER_1을 날려 GUI 시스템이 종료되도록 한다.
			pT = find_thread_by_alias( GUI_THREAD_NAME );
			if( pT == NULL )
			{	// gui 쓰레드를 찾을 수 없다. 
				kdbg_printf( "gui thread not found!\n" );
			}
			else	
			{	// 시그널을 날린다.
				kdbg_printf( "Quit GUI Signal\n" );
				send_signal_to_thread( pT->dwID, SIG_USER_1 );
			}
		}
		
		// 메시지 박스가 닫히면 MODAL 메시지가 Parent에 전달되므로 
		// 해당 메시지 박스 포인터를 NULL로 만든다.
		pPrivate->pExitMBox = NULL;
	}	

	return( WMHRV_CONTINUE );
}

static WMFuncStt wall_marray[] = {
	{ WMESG_CLOSE		   , wmh_wall_close			},
	{ WMESG_DESTROY		   , wmh_wall_destroy		},
	{ WMESG_MODAL		   , wmh_wall_modal			},
	{ 0, NULL}
};

WinStt *get_wall_win()
{
	GuiStt	*pGui;
	pGui = get_gui_stt();
	
	return( &pGui->wall );
}

// 타일 형태로 배경을 그린다.
static void tile_wallpaper()
{	
	RectStt		*pR;
	ImageStt	*pImg;
	GuiStt		*pGui;
	int			nX, nY, nXLimit, nYLimit;

	pGui = get_gui_stt();
	pR   = &pGui->wall.obj.r;
	pImg = pGui->pWallPaper;

	// 배경 무늬를 그린다. 
	for( nY = 0; nY < pR->nV; nY += pImg->nV )
	{
		nYLimit = pR->nV - nY;
		if( nYLimit > pImg->nV )
			nYLimit = pImg->nV;

		for( nX = 0; nX < pR->nH; nX += pImg->nH )
		{
			nXLimit = pR->nH - nX;
			if( nXLimit > pImg->nH )
				nXLimit = pImg->nH;

			if( pGui->wall.gb.pW == NULL )
				draw_image16( pImg, NULL, nX, nY, 0, 0, nXLimit, nYLimit );
			else
				copy_image16( &pGui->wall.gb, pImg, nX, nY, nXLimit, nYLimit, 0, 0 );
		}
	}  
}

// 바탕화면을 색상으로 채운다.
static void fill_wallpaper( RectStt *pR, MaskStt *pMask )
{
	GraBuffStt	*pGB;
	GuiStt		*pGui;
		
	pGui = get_gui_stt();
	
	// NULL이면 화면 전체를 다시 그린다.
	if( pR == NULL )
		pR = &pGui->wall.obj.r;  // 실제 그려야 하는 영역.

	// 바탕화면에 grabuff가 할당되어있지 않으면 비디오 메모리에 직접 그린다.
	if( pGui->wall.gb.pW == NULL )
		pGB = NULL;
	else
		pGB = &pGui->wall.gb;

	// 버퍼, 전체 윈도우 영역, 실제로 그려야 하는 영역, 라인당 바이트 수, 색상1, 색상2, 마스크
	k_incremental_fill_rect( pGB, &pGui->wall.obj.r, pR, pGui->vmode.LinBytesPerScanLine, 
		                     pGui->wcolor[0], pGui->wcolor[1], pMask);
}

// 배경 무늬로 화면을 지운다. 
static void remake_wall( RectStt *pR )
{	
	GraBuffStt	*pGB;
	GuiStt		*pGui;
	ImageStt	*pImg;
	
	pGui = get_gui_stt();

	// 바탕화면에 grabuff가 할당되어있지 않으면 비디오 메모리에 직접 그린다.
	if( pGui->wall.gb.pW == NULL )
		pGB = NULL;
	else
		pGB = &pGui->wall.gb;
	
	// NULL이면 화면 전체를 다시 그린다.
	if( pR == NULL )
		pR = &pGui->wall.obj.r;  // 실제 그려야 하는 영역.

	if( pGui->dwWallType == WALL_TYPE_TILE )
		tile_wallpaper();
	else if( pGui->dwWallType == WALL_TYPE_FILL_COLOR )
	{	
		// 버퍼, 전체 윈도우 영역, 실제로 그려야 하는 영역, 라인당 바이트 수, 색상1, 색상2, 마스크
		k_incremental_fill_rect( pGB, &pGui->wall.obj.r, pR, pGui->vmode.LinBytesPerScanLine, 
		                     pGui->wcolor[0], pGui->wcolor[1], &pGui->wall.mask );
	}
	else if( pGui->dwWallType == WALL_TYPE_WALL_PAPER )
	{
		__int64 ia, ib, ic;
		DWORD 	x[2];
		
		get_clk( &ia );

		pImg = load_bitmap_file_to_image16( pGui->szWallPaper );

		get_clk( &ib );
		ic = ib - ia;
		memcpy( x, &ic, 8 );
		kdbg_printf( "elapsed freq(load_bitmap_file_to_image16) lo=0x%X, hi=0x%X\n", x[0], x[1] );
		/*  Scatter load
		=========================
			0xA9C53522
			0xAB26337A
			0xA9C33329

			Normal load
		=========================
			0xAF23FACD
			0xB0E26D37
			0xAC51AC9B
		=========================		
		*/
		
		if( pImg != NULL )
		{
			copy_image16( pGB, pImg, 0, 0, pGui->wall.obj.r.nH, pGui->wall.obj.r.nV, 0, 0 );
			// 이미지를 해제한다.
			free_image16( pImg );
		}		
	}
}

// 바탕화면 윈도우를 초기화 한다.  ( kcreate_window()로 생성하지 않는다.)
int init_wall_win( WinThreadStt *pWThread, int nWinH, int nWinV )
{
	RectStt	*pR;
	WinStt	*pWin;
	GuiStt	*pGui;
		
	pGui = get_gui_stt();
	pWin = get_wall_win();

	// 윈도우 크기 설정.
	pR = &pWin->obj.r;
	pR->nX  = 0;
	pR->nY  = 0;
	pR->nH  = nWinH;
	pR->nV  = nWinV;
	
	// 메시지 핸들러 설정.
	pWin->pWMArray = wall_marray;

	// 비트 마스크를 할당하고 모두 1로 설정한다.
	alloc_bit_mask( pWin );

	// 그래픽 버퍼를 할당한다.
	alloc_gra_buff( pWin );
				 
	// Private 영역을 할당한다.
	pWin->pPrivate = kmalloc( sizeof( WallPrivateStt ) );
	if( pWin->pPrivate != NULL )
		memset( pWin->pPrivate, 0, sizeof( WallPrivateStt ) );
	else
		kdbg_printf( "init_wall_win: alloc private failed!\n" );

	// 윈도우 쓰레드에 추가한다. 
	append_win_to_thread( pWThread, pWin );

	{	// 단색으로 배경 화면을 지운다.
		int		nI;
		UINT16	*pW;
		pW   = pWin->gb.pW;
		
		for( nI = 0; nI < pGui->vmode.wX * pGui->vmode.wY; nI++ )
		{
			pW[nI] = WALL_BK_COLOR;
		}
	}

	// 배경 무늬를 만든다.
	//pGui->dwWallType = WALL_TYPE_FILL_COLOR;
	//pGui->wcolor[0]  = RGB16( 245, 142, 155 );
	//pGui->wcolor[1]  = RGB16( 194, 194,  55 );

	// Wall paper를 로드하는 것으로 설정한다.
	strcpy( pGui->szWallPaper, "/c/wall.bmp" );
	pGui->dwWallType = WALL_TYPE_WALL_PAPER;
	
	remake_wall( NULL );
	
	return( 0 );
}

// 바탕화면 윈도우를 닫는다.
int close_gui_wall()
{
	WinStt	*pWin;

	pWin = get_wall_win();

	// Bitmask와 grabuff를 해제한다.
	free_bit_mask( pWin );
	free_gra_buff( pWin );
				 
	// Private 영역을 해제한다.
	if( pWin->pPrivate != NULL )
	{
		kfree( pWin->pPrivate );
		pWin->pPrivate = NULL;
	}

	memset( pWin, 0, sizeof( WinStt ) );

	return( 0 );
}

// 만들어진 배경화면을 스크린에 그린다.
int flush_wall( RectStt *pR )
{
	int		nR;
	GuiStt	*pGui;

	pGui = get_gui_stt();
	if( pR == NULL )
		pR = pGui->wall.gb.pR;

	nR = flush_gra_buff( &pGui->wall, pR );

	// Transparent 윈도우를 모두 새로 그린다.
	repaint_upper_transparent( NULL, pR );
	
	return( nR );
}
