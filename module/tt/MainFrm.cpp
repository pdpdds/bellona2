// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "tt.h"
#include "tetris.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern "C" TTCfgStt tt_cfg;
extern "C" int flush_slot( DWORD dwHandle );
extern "C" int fill_rect32( DWORD dwHandle, RectStt *pR, DWORD dwColor );
extern "C" int gx_line( DWORD dwHandle, int nX1, int nY1, int nX2, int nY2, unsigned short wColor );
extern "C" int gx_drawtext_xy( DWORD dwHandle, int nX, int nY, DWORD dwFontID, char *pStr, unsigned short wColor, DWORD dwEffect );


/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	ON_MESSAGE( WM_TT_INITIALIZED, OnInitialized )
	ON_MESSAGE( WM_TT_REPAINT,     OnRepaint     )
	ON_MESSAGE( WM_TT_KEY_INPUT,   OnKeyInput    )
	ON_WM_CLOSE()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	// create a view to occupy the client area of the frame
	if (!m_wndView.Create(NULL, NULL, AFX_WS_DEFAULT_VIEW,
		CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST, NULL))
	{
		TRACE0("Failed to create view window\n");
		return -1;
	}
	
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	cs.lpszClass = AfxRegisterWndClass(0);

	cs.cx = TT_WIN_H;
	cs.cy = TT_WIN_V;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

CMainFrame *G_pMainFrame = NULL;

typedef struct {
	CBrush			*pBrush;
	DWORD			dwColor;
	int				nMake;
} BrushTblStt;

static CBrush	bk_brush;
static CBrush	info_brush;
static CBrush	blue_brush;
static CBrush	stack_brush;

static BrushTblStt br_tbl[] = {
	{ &bk_brush,		COLOR_SLOT	,	1	},
	{ &info_brush,		COLOR_INFO	,	1	},
	{ &blue_brush,		COLOR_BLOCK ,	1	},
	{ &stack_brush,		COLOR_STACK ,	1	},
	{ WHITE_BRUSH,		COLOR_WHITE	,	0	},
	{ NULL,				0			,	-1	}
};

static CPen brown_pen;

static void create_brushes()
{
	int nI;

	for( nI = 0; br_tbl[nI].nMake != -1; nI++ )
	{
		if( br_tbl[nI].nMake == 0 )
			continue;
		br_tbl[nI].pBrush->CreateSolidBrush( br_tbl[nI].dwColor );
	}

	brown_pen.CreatePen( PS_SOLID, 1, RGB_BROWN );
}

static void delete_brushes()
{
	int nI;

	for( nI = 0; br_tbl[nI].nMake != -1; nI++ )
	{
		if( br_tbl[nI].nMake == 0 )
			continue;
		br_tbl[nI].pBrush->DeleteObject();
		br_tbl[nI].pBrush = NULL;
	}

	brown_pen.DeleteObject();
}

static CBrush *find_brush( DWORD dwColor )
{
	int nI;

	for( nI = 0; br_tbl[nI].nMake != -1; nI++ )
	{
		if( br_tbl[nI].dwColor == dwColor )
			return( br_tbl[nI].pBrush );
	}
	return( NULL );
}

int fill_rect32( DWORD dwHandle, RectStt *pR, DWORD dwColor )
{
	RectStt	r;
	CDC		*pDC;
	CBrush	*pBrush;

	pDC = (CDC*)dwHandle;

	// 브러시를 찾는다.
	pBrush = find_brush( dwColor );

	memcpy( &r, pR, sizeof( RectStt ) );
	r.nH += r.nX;
	r.nV += r.nY;

	pDC->FillRect( (RECT*)&r, pBrush );

	return( 0 );
}

int gx_line( DWORD dwHandle, int nX1, int nY1, int nX2, int nY2, unsigned short wColor )
{
	CDC		*pDC;
	CPen	*pOldPen;

	pDC = (CDC*)dwHandle;

	pDC->MoveTo( nX1, nY1 );

	pOldPen = pDC->SelectObject( &brown_pen );
	pDC->LineTo( nX2, nY2 );
	pDC->SelectObject( pOldPen );

	return( 0 );
}

int gx_drawtext_xy( DWORD dwHandle, int nX, int nY, DWORD dwFontID, char *pStr, unsigned short wColor, DWORD dwEffect )
{
	CDC		*pDC;

	pDC = (CDC*)dwHandle;

	pDC->TextOut( nX, nY, pStr, strlen( pStr ) );

	return( 0 );
}

// 그려둔 슬롯 전체를 스크린에 복사한다.
int flush_slot( DWORD dwHandle )
{
	CDC *pDC;

	if( G_pMainFrame == NULL )
		return( 0 );

	pDC = G_pMainFrame->m_wndView.GetDC();

	pDC->BitBlt( 0, 0, tt_cfg.wClientH, tt_cfg.wClientV, &G_pMainFrame->m_back_dc, 0, 0, SRCCOPY );
	
	G_pMainFrame->m_wndView.ReleaseDC( pDC );

	return( 0 );
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers
void CMainFrame::OnSetFocus(CWnd* pOldWnd)
{
	// forward focus to the view window
	m_wndView.SetFocus();
}

BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// let the view have first crack at the command
	if (m_wndView.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	// otherwise, do default handling
	return CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}


void CMainFrame::create_back_dc( int nH, int nV )
{
	CDC		*pDC;

	if( m_bDC == TRUE )
		return;

	pDC	= m_wndView.GetDC();
    m_back_dc.CreateCompatibleDC( pDC );
    m_back_bitmap.CreateCompatibleBitmap( pDC, nH, nV );
    m_pOldBitmap = m_back_dc.SelectObject( &m_back_bitmap );
	m_bDC = TRUE;
	
	m_wndView.ReleaseDC( pDC );
}

void CMainFrame::delete_back_dc()
{
	if( m_bDC == FALSE )	
		return;

    m_back_dc.SelectObject( m_pOldBitmap );
	m_back_dc.DeleteDC();
	m_back_bitmap.DeleteObject();
	m_bDC = FALSE;
}

void CMainFrame::OnClose() 
{
	KillTimer( 5555 );

	delete_back_dc();

	// 브러시를 제거한다.
	delete_brushes();

	CFrameWnd::OnClose();
}

long CMainFrame::OnInitialized( UINT wParam, long lParam )
{
	RECT	r;

	G_pMainFrame = this;

	SetTimer( 5555, TT_TIMER_INTERVAL, NULL );
	srand( (DWORD)GetTickCount() );
	
	m_wndView.GetClientRect( &r );
	init_tt_cfg( &tt_cfg, (RectStt*)&r );

	// 동일 크기의 back dc를 생성한다.
	create_back_dc( r.right, r.bottom );

	// 브러시를 생성한다.
	create_brushes();

	// GAME OVER 상태로 실행한다.
	set_state( TT_STATE_GAME_OVER );
	
	// 슬롯을 재구성한다.
	remake_slot( (DWORD)&m_back_dc );

	// 구성된 슬롯을 화면에 출력한다.
	flush_slot( 0 );

	return( 0 );
}

long CMainFrame::OnRepaint( UINT wParam, long lparam )
{
	flush_slot( 0 );
	return( 0 );
}

long CMainFrame::OnKeyInput( UINT wChar, long lParam )
{
	int nState;

	switch( wChar )
	{
	case 0x25:	// LEFT
		block_left( (DWORD)&m_back_dc );
		break;
	case 0x26:	// UP
		block_rotate( (DWORD)&m_back_dc );
		break;
	case 0x27:	// RIGHT
		block_right( (DWORD)&m_back_dc );
		break;
	case 0x28:	// DOWN
		block_down( (DWORD)&m_back_dc, TT_LINE_V );
		break;
	case 0x0D :
		nState = get_state();
		if( nState == TT_STATE_PAUSED )
			set_state( TT_STATE_RUNNING );
		else if( nState == TT_STATE_GAME_OVER )
			block_start( (DWORD)&m_back_dc );
		break;
	case 'p' :
	case 'P' :
		block_pause( (DWORD)&m_back_dc );
		break;
	}

	return( 0 );
}

void CMainFrame::OnTimer(UINT nIDEvent) 
{
	CFrameWnd::OnTimer(nIDEvent);

	if( tt_cfg.nState != TT_STATE_RUNNING )
		return;		  
	
	// 일정 시간 간격으로 불러 준다.
	tt_cfg.nTimerCount++;
	if( TT_MAX_LEVEL - tt_cfg.nLevel < tt_cfg.nTimerCount )
	{
		slot_feed_timer( (DWORD)&m_back_dc );
		tt_cfg.nTimerCount = 0;
	}
}

