#include <bellona2.h>
#include "gui.h"

// wall.c ���������� ����Ѵ�.
#define MBOX_ID_EXIT  100

// gui.c�� gui_thread���� SIG_USER_0�� ������ WMESG_CLOSE�� wall ������� ������.
// �޽����� ���� �� ���ƿ͵� �޽��� �ڽ��� �̹̻����Ǿ� ������ �ٽ� ������ �ʴ´�.
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

	// �޽��� �ڽ��� �̹� �� ������ �׳� �����Ѵ�.
	if( pPrivate->pExitMBox != NULL )
	{	// ������ �����Ѵ�.
		set_top_window( pPrivate->pExitMBox );
		return( WMHRV_ABORT );
	}

	// �޽��� �ڽ��� �����Ѵ�.	
	pPrivate->pExitMBox = message_box( pWin, MBOX_ID_EXIT, "Really want to quit?", "Question", 200, 150, 
		MBOX_BUTTON_OK | MBOX_BUTTON_CANCEL );

	return( WMHRV_ABORT );
}

static DWORD wmh_wall_destroy( WinStt *pWin, DWORD dwWMesgID, DWORD dwParamA, DWORD dwParamB )
{
	// Private ������ �����Ѵ�.
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
			// gui_thread ������ SIG_USER_1�� ���� GUI �ý����� ����ǵ��� �Ѵ�.
			pT = find_thread_by_alias( GUI_THREAD_NAME );
			if( pT == NULL )
			{	// gui �����带 ã�� �� ����. 
				kdbg_printf( "gui thread not found!\n" );
			}
			else	
			{	// �ñ׳��� ������.
				kdbg_printf( "Quit GUI Signal\n" );
				send_signal_to_thread( pT->dwID, SIG_USER_1 );
			}
		}
		
		// �޽��� �ڽ��� ������ MODAL �޽����� Parent�� ���޵ǹǷ� 
		// �ش� �޽��� �ڽ� �����͸� NULL�� �����.
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

// Ÿ�� ���·� ����� �׸���.
static void tile_wallpaper()
{	
	RectStt		*pR;
	ImageStt	*pImg;
	GuiStt		*pGui;
	int			nX, nY, nXLimit, nYLimit;

	pGui = get_gui_stt();
	pR   = &pGui->wall.obj.r;
	pImg = pGui->pWallPaper;

	// ��� ���̸� �׸���. 
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

// ����ȭ���� �������� ä���.
static void fill_wallpaper( RectStt *pR, MaskStt *pMask )
{
	GraBuffStt	*pGB;
	GuiStt		*pGui;
		
	pGui = get_gui_stt();
	
	// NULL�̸� ȭ�� ��ü�� �ٽ� �׸���.
	if( pR == NULL )
		pR = &pGui->wall.obj.r;  // ���� �׷��� �ϴ� ����.

	// ����ȭ�鿡 grabuff�� �Ҵ�Ǿ����� ������ ���� �޸𸮿� ���� �׸���.
	if( pGui->wall.gb.pW == NULL )
		pGB = NULL;
	else
		pGB = &pGui->wall.gb;

	// ����, ��ü ������ ����, ������ �׷��� �ϴ� ����, ���δ� ����Ʈ ��, ����1, ����2, ����ũ
	k_incremental_fill_rect( pGB, &pGui->wall.obj.r, pR, pGui->vmode.LinBytesPerScanLine, 
		                     pGui->wcolor[0], pGui->wcolor[1], pMask);
}

// ��� ���̷� ȭ���� �����. 
static void remake_wall( RectStt *pR )
{	
	GraBuffStt	*pGB;
	GuiStt		*pGui;
	ImageStt	*pImg;
	
	pGui = get_gui_stt();

	// ����ȭ�鿡 grabuff�� �Ҵ�Ǿ����� ������ ���� �޸𸮿� ���� �׸���.
	if( pGui->wall.gb.pW == NULL )
		pGB = NULL;
	else
		pGB = &pGui->wall.gb;
	
	// NULL�̸� ȭ�� ��ü�� �ٽ� �׸���.
	if( pR == NULL )
		pR = &pGui->wall.obj.r;  // ���� �׷��� �ϴ� ����.

	if( pGui->dwWallType == WALL_TYPE_TILE )
		tile_wallpaper();
	else if( pGui->dwWallType == WALL_TYPE_FILL_COLOR )
	{	
		// ����, ��ü ������ ����, ������ �׷��� �ϴ� ����, ���δ� ����Ʈ ��, ����1, ����2, ����ũ
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
			// �̹����� �����Ѵ�.
			free_image16( pImg );
		}		
	}
}

// ����ȭ�� �����츦 �ʱ�ȭ �Ѵ�.  ( kcreate_window()�� �������� �ʴ´�.)
int init_wall_win( WinThreadStt *pWThread, int nWinH, int nWinV )
{
	RectStt	*pR;
	WinStt	*pWin;
	GuiStt	*pGui;
		
	pGui = get_gui_stt();
	pWin = get_wall_win();

	// ������ ũ�� ����.
	pR = &pWin->obj.r;
	pR->nX  = 0;
	pR->nY  = 0;
	pR->nH  = nWinH;
	pR->nV  = nWinV;
	
	// �޽��� �ڵ鷯 ����.
	pWin->pWMArray = wall_marray;

	// ��Ʈ ����ũ�� �Ҵ��ϰ� ��� 1�� �����Ѵ�.
	alloc_bit_mask( pWin );

	// �׷��� ���۸� �Ҵ��Ѵ�.
	alloc_gra_buff( pWin );
				 
	// Private ������ �Ҵ��Ѵ�.
	pWin->pPrivate = kmalloc( sizeof( WallPrivateStt ) );
	if( pWin->pPrivate != NULL )
		memset( pWin->pPrivate, 0, sizeof( WallPrivateStt ) );
	else
		kdbg_printf( "init_wall_win: alloc private failed!\n" );

	// ������ �����忡 �߰��Ѵ�. 
	append_win_to_thread( pWThread, pWin );

	{	// �ܻ����� ��� ȭ���� �����.
		int		nI;
		UINT16	*pW;
		pW   = pWin->gb.pW;
		
		for( nI = 0; nI < pGui->vmode.wX * pGui->vmode.wY; nI++ )
		{
			pW[nI] = WALL_BK_COLOR;
		}
	}

	// ��� ���̸� �����.
	//pGui->dwWallType = WALL_TYPE_FILL_COLOR;
	//pGui->wcolor[0]  = RGB16( 245, 142, 155 );
	//pGui->wcolor[1]  = RGB16( 194, 194,  55 );

	// Wall paper�� �ε��ϴ� ������ �����Ѵ�.
	strcpy( pGui->szWallPaper, "/c/wall.bmp" );
	pGui->dwWallType = WALL_TYPE_WALL_PAPER;
	
	remake_wall( NULL );
	
	return( 0 );
}

// ����ȭ�� �����츦 �ݴ´�.
int close_gui_wall()
{
	WinStt	*pWin;

	pWin = get_wall_win();

	// Bitmask�� grabuff�� �����Ѵ�.
	free_bit_mask( pWin );
	free_gra_buff( pWin );
				 
	// Private ������ �����Ѵ�.
	if( pWin->pPrivate != NULL )
	{
		kfree( pWin->pPrivate );
		pWin->pPrivate = NULL;
	}

	memset( pWin, 0, sizeof( WinStt ) );

	return( 0 );
}

// ������� ���ȭ���� ��ũ���� �׸���.
int flush_wall( RectStt *pR )
{
	int		nR;
	GuiStt	*pGui;

	pGui = get_gui_stt();
	if( pR == NULL )
		pR = pGui->wall.gb.pR;

	nR = flush_gra_buff( &pGui->wall, pR );

	// Transparent �����츦 ��� ���� �׸���.
	repaint_upper_transparent( NULL, pR );
	
	return( nR );
}
