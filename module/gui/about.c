#include <bellona2.h>
#include "gui.h"

DWORD about_make_screen( WinStt *pWin );

static DWORD wmh_about_create( WinStt *pWin, DWORD dwWMesgID, DWORD dwParamA, DWORD dwParamB )
{
	AboutPrivateStt	*pAboutPrivate;

	//타이틀을 설정한다. 
	set_window_title( pWin, "B2OS GUI..." );

	// Win Private 영역을 할당한다. 
	pAboutPrivate = (AboutPrivateStt*)kmalloc( sizeof( AboutPrivateStt ) );
	pWin->pPrivate = pAboutPrivate;
	if( pAboutPrivate != NULL )
	{
		memset( pAboutPrivate, 0, sizeof( AboutPrivateStt ) );
		// logo8.bmp를 로드한다. 
		pAboutPrivate->pLogo  = load_bitmap_image16( get_winres_stt(), IDB_B2K );
		pAboutPrivate->pKorea = load_icon_image16( get_winres_stt(), IDI_KOREA );
	}	

	// 최초로 윈도우를 그려둔다.
	about_make_screen( pWin );

	// TaskBar에 아이콘을 등록한다.
	tb_add_icon( IDI_ABOUT_ICON, "About B2OS", NULL, pWin );
	
	return( WMHRV_CONTINUE );
}

static DWORD wmh_about_destroy( WinStt *pWin, DWORD dwWMesgID, DWORD dwParamA, DWORD dwParamB )
{
	AboutPrivateStt *pPrivate;

	pPrivate = (AboutPrivateStt*)pWin->pPrivate;
	
	// 이미지를 해제한다.
	free_image16( pPrivate->pLogo  );
	free_image16( pPrivate->pKorea );
	
	// Win Private 영역을 해제한다. 
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

// about window의 화면을 구성한다. (실제 보여지지는 않는다.)
static DWORD about_make_screen( WinStt *pWin )
{
	RectStt			r;
	ImageStt		*pImg;
	int				nX, nY, nI;   
	AboutPrivateStt	*pAboutPrivate;

	// 클라이언트 영역을 지운다. 
	//screen_to_win( &r, pWin->gb.pR, &pWin->ct_r );
	get_client_rect( pWin, &r );
	
	k_fill_rect( &pWin->gb, &r, ABOUT_BACK_COLOR );

	// 로고 이미지를 출력한다. 
	pAboutPrivate = (AboutPrivateStt*)pWin->pPrivate;
	if( pAboutPrivate != NULL && pAboutPrivate->pLogo != NULL )
	{	// 클라이언트 영역 가운데에 구린다. 
		pImg = pAboutPrivate->pLogo;
		nX = ( r.nH - pImg->nH ) / 2;
		nY = r.nY + 10;
		// 8비트 로고를 그린다.
		copy_image16( &pWin->gb, pImg, nX, nY, pImg->nH, pImg->nV, 0, 0 );
		nY += pImg->nV + 4;
	}	
	else
		nY = r.nY + 4;

	// 태극기를 그린다.
	if( pAboutPrivate->pKorea != NULL )
	{
		pImg = pAboutPrivate->pKorea;
		nX = (r.nH - pImg->nH) /2;
		copy_image16( &pWin->gb, pImg, nX, nY, pImg->nH, pImg->nV, 0, 0 );
	}
	nY += pImg->nV;

	// "진행중" 메시지를 출력한다. 
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

// 마우스가 윈도우 바깥으로 빠져 나갔다. 
static DWORD wmh_about_mouse_move_out( WinStt *pWin, DWORD dwWMesgID, DWORD dwParamA, DWORD dwParamB )
{

	return( 0 );
}	

// 마우스가 윈도우 안으로 들어왔다. 
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

