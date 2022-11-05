#include <bellona2.h>
#include "gui.h"

DWORD about_make_screen( WinStt *pWin );

static DWORD wmh_about_create( WinStt *pWin, DWORD dwWMesgID, DWORD dwParamA, DWORD dwParamB )
{
	AboutPrivateStt	*pAboutPrivate;

	//Ÿ��Ʋ�� �����Ѵ�. 
	set_window_title( pWin, "B2OS GUI..." );

	// Win Private ������ �Ҵ��Ѵ�. 
	pAboutPrivate = (AboutPrivateStt*)kmalloc( sizeof( AboutPrivateStt ) );
	pWin->pPrivate = pAboutPrivate;
	if( pAboutPrivate != NULL )
	{
		memset( pAboutPrivate, 0, sizeof( AboutPrivateStt ) );
		// logo8.bmp�� �ε��Ѵ�. 
		pAboutPrivate->pLogo  = load_bitmap_image16( get_winres_stt(), IDB_B2K );
		pAboutPrivate->pKorea = load_icon_image16( get_winres_stt(), IDI_KOREA );
	}	

	// ���ʷ� �����츦 �׷��д�.
	about_make_screen( pWin );

	// TaskBar�� �������� ����Ѵ�.
	tb_add_icon( IDI_ABOUT_ICON, "About B2OS", NULL, pWin );
	
	return( WMHRV_CONTINUE );
}

static DWORD wmh_about_destroy( WinStt *pWin, DWORD dwWMesgID, DWORD dwParamA, DWORD dwParamB )
{
	AboutPrivateStt *pPrivate;

	pPrivate = (AboutPrivateStt*)pWin->pPrivate;
	
	// �̹����� �����Ѵ�.
	free_image16( pPrivate->pLogo  );
	free_image16( pPrivate->pKorea );
	
	// Win Private ������ �����Ѵ�. 
	kfree( pWin->pPrivate );
	pWin->pPrivate = NULL;
	
	return( WMHRV_ABORT );
}

static char *about_string[] = {
	"B2OS GUI system is under construction.",
	"[ALT-F4] to quit GUI system.",	
	"bellona2@chollian.net",
	NULL
};

// about window�� ȭ���� �����Ѵ�. (���� ���������� �ʴ´�.)
static DWORD about_make_screen( WinStt *pWin )
{
	RectStt			r;
	ImageStt		*pImg;
	int				nX, nY, nI;   
	AboutPrivateStt	*pAboutPrivate;

	// Ŭ���̾�Ʈ ������ �����. 
	//screen_to_win( &r, pWin->gb.pR, &pWin->ct_r );
	get_client_rect( pWin, &r );
	
	k_fill_rect( &pWin->gb, &r, ABOUT_BACK_COLOR );

	// �ΰ� �̹����� ����Ѵ�. 
	pAboutPrivate = (AboutPrivateStt*)pWin->pPrivate;
	if( pAboutPrivate != NULL && pAboutPrivate->pLogo != NULL )
	{	// Ŭ���̾�Ʈ ���� ����� ������. 
		pImg = pAboutPrivate->pLogo;
		nX = ( r.nH - pImg->nH ) / 2;
		nY = r.nY + 10;
		// 8��Ʈ �ΰ� �׸���.
		copy_image16( &pWin->gb, pImg, nX, nY, pImg->nH, pImg->nV, 0, 0 );
		nY += pImg->nV + 4;
	}	
	else
		nY = r.nY + 4;

	// �±ر⸦ �׸���.
	if( pAboutPrivate->pKorea != NULL )
	{
		pImg = pAboutPrivate->pKorea;
		nX = (r.nH - pImg->nH) /2;
		copy_image16( &pWin->gb, pImg, nX, nY, pImg->nH, pImg->nV, 0, 0 );
	}
	nY += pImg->nV;

	// "������" �޽����� ����Ѵ�. 
	nX = r.nX + 10;
	for( nI = 0; about_string[nI] != NULL; nI++ )
	{
		drawtext_xy( 
			&pWin->gb,													
			nX,
			nY,
			get_system_font( IDR_BF_BASE11 ),						
			about_string[nI],
			RGB16( 0, 0, 0 ),
			0 );													
		nY += 15;
	}

	return( 0 );
}

static DWORD wmh_about_lbtn_dn( WinStt *pWin, DWORD dwWMesgID, DWORD dwParamA, DWORD dwParamB )
{
	return( WMHRV_CONTINUE );
}

static DWORD wmh_about_lbtn_up( WinStt *pWin, DWORD dwWMesgID, DWORD dwParamA, DWORD dwParamB )
{

	return( WMHRV_CONTINUE );
}

static DWORD wmh_about_rbtn_dn( WinStt *pWin, DWORD dwWMesgID, DWORD dwParamA, DWORD dwParamB )
{

	return( WMHRV_CONTINUE );
}

static DWORD wmh_about_rbtn_up( WinStt *pWin, DWORD dwWMesgID, DWORD dwParamA, DWORD dwParamB )
{

	return( WMHRV_CONTINUE );
}

// ���콺�� ������ �ٱ����� ���� ������. 
static DWORD wmh_about_mouse_move_out( WinStt *pWin, DWORD dwWMesgID, DWORD dwParamA, DWORD dwParamB )
{

	return( 0 );
}	

// ���콺�� ������ ������ ���Դ�. 
static DWORD wmh_about_mouse_move_in( WinStt *pWin, DWORD dwWMesgID, DWORD dwParamA, DWORD dwParamB )
{

	return( 0 );
}	
static DWORD wmh_about_mouse_move( WinStt *pWin, DWORD dwWMesgID, DWORD dwParamA, DWORD dwParamB )
{
	return( 0 );
}				

static DWORD wmh_about_minimize( WinStt *pWin, DWORD dwWMesgID, DWORD dwParamA, DWORD dwParamB )
{
	return( 0 );
}

static DWORD wmh_about_maximize( WinStt *pWin, DWORD dwWMesgID, DWORD dwParamA, DWORD dwParamB )
{
	return( 0 );
}

static DWORD wmh_about_win_move( WinStt *pWin, DWORD dwWMesgID, DWORD dwParamA, DWORD dwParamB )
{
	return( 0 );
}

WMFuncStt about_marray[] = {
	{ WMESG_LBTN_DN		   , wmh_about_lbtn_dn			},
	{ WMESG_LBTN_UP		   , wmh_about_lbtn_up			},
	{ WMESG_RBTN_DN		   , wmh_about_rbtn_dn			},
	{ WMESG_RBTN_UP		   , wmh_about_rbtn_up			},
	{ WMESG_MOUSE_MOVE_OUT , wmh_about_mouse_move_out	},									 
	{ WMESG_MOUSE_MOVE_IN  , wmh_about_mouse_move_in	},
	{ WMESG_MINIMIZE       , wmh_about_minimize			},
	{ WMESG_MAXIMIZE       , wmh_about_maximize			},
	{ WMESG_MOUSE_MOVE     , wmh_about_mouse_move		},
	{ WMESG_WIN_MOVE       , wmh_about_win_move			},
	{ WMESG_CREATE		   , wmh_about_create			},
	{ WMESG_DESTROY		   , wmh_about_destroy			},
	{ 0, NULL }
};

