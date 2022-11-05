// FontEditView.cpp : implementation of the CFontEditView class
//

#include "stdafx.h"
#include "FontEdit.h"

#include "FontEditDoc.h"
#include "FontEditView.h"
#include "CreateNewDlg.h"

#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define VIEW_TOP_MARGIN		10
#define VIEW_H_MARGIN		70
#define PREVIEW_H_MARGIN	70
/////////////////////////////////////////////////////////////////////////////
// CFontEditView

IMPLEMENT_DYNCREATE(CFontEditView, CFormView)

BEGIN_MESSAGE_MAP(CFontEditView, CFormView)
	//{{AFX_MSG_MAP(CFontEditView)
	ON_COMMAND(IDM_CREATE_NEW, OnCreateNew)
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
	ON_BN_CLICKED(IDC_NEXT_CHAR, OnNextChar)
	ON_BN_CLICKED(IDC_PREV_CHAR, OnPrevChar)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_BN_CLICKED(IDC_GO_CHAR, OnGoChar)
	ON_BN_CLICKED(IDC_GO_DEC, OnGoDec)
	ON_BN_CLICKED(IDC_GO_HEX, OnGoHex)
	ON_COMMAND(IDM_SAVE_FILE, OnSaveFile)
	ON_COMMAND(IDM_OPEN_FONT, OnOpenFont)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFontEditView construction/destruction

CFontEditView::CFontEditView()
	: CFormView(CFontEditView::IDD)
{
	m_font_path[0]	  = 0;
	m_nEditCharIndex  = 0;
	m_nLButtonDn	  = 0;
	m_nPreViewVMargin = 0;
	m_pFont			  = NULL;

	//{{AFX_DATA_INIT(CFontEditView)
	//}}AFX_DATA_INIT
}

CFontEditView::~CFontEditView()
{
}

void CFontEditView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFontEditView)
	//}}AFX_DATA_MAP
}

BOOL CFontEditView::PreCreateWindow(CREATESTRUCT& cs)
{
	return CFormView::PreCreateWindow(cs);
}

void CFontEditView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
	GetParentFrame()->RecalcLayout();
	ResizeParentToFit();

}

/////////////////////////////////////////////////////////////////////////////
// CFontEditView diagnostics

#ifdef _DEBUG
void CFontEditView::AssertValid() const
{
	CFormView::AssertValid();
}

void CFontEditView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}

CFontEditDoc* CFontEditView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CFontEditDoc)));
	return (CFontEditDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CFontEditView message handlers
// 빈 폰트 파일을 생성하고 메모리를 할당한다.
FontStt *CFontEditView::create_empty_font_file( char *pS, int nFontH, int nFontV, int nStartChar, int nEndChar )
{
	FontStt		hd, *pFont;
	int			nK, nMemSize, nTotalChar;

	memset( &hd, 0, sizeof( hd ) );
	hd.cBitPerPixel = 1;
	hd.dwVersion    = 1;
	hd.dwMagic      = BELL_FONT_MAGIC;
	hd.wStartChar   = nStartChar;
	hd.wEndChar     = nEndChar;
	hd.cH = (char)nFontH;
	hd.cV = (char)nFontV;
	nK = ( (int)hd.cBitPerPixel * (int)hd.cH );

	if( 0 < nK && nK <= 8 )	    hd.cLinByte = 1; 	// 1 바이트
	else if( nK <= 16 ) hd.cLinByte = 2;			// 2 바이트
	else if( nK <= 32 ) hd.cLinByte = 4; 			// 4 바이트
	else
	{
		return( NULL );
	}

	nTotalChar = nEndChar - nStartChar + 1;
	nMemSize = sizeof( hd ) + ( ( hd.cLinByte * nFontV ) * nTotalChar );
	pFont = (FontStt*)malloc( nMemSize );
	if( pFont == NULL )
		return( NULL );

	memset( pFont, 0, nMemSize );
	memcpy( pFont, &hd, sizeof( hd ) );
	pFont->b.pB = (BYTE*)( (DWORD)pFont + sizeof( FontStt ) );

	// 현재 편집하고 있는 글자 인덱스
	m_nEditCharIndex = 0;

	// 파일명이 설정되어 있으면 저장한다.
	if( pS != NULL && pS[0] != 0 )
		save_font( pS, pFont );

	return( pFont );
}

void CFontEditView::create_back_dc()
{
	RECT	r;
	CDC		*pDC;

	if( m_bDC == TRUE )
		return;

	GetClientRect( &r );
	r.right  -= VIEW_H_MARGIN;
	if( r.right <= 0 || r.bottom <= 0 )
		return;
	pDC = GetDC();
    m_back_dc.CreateCompatibleDC( pDC );
    m_back_bitmap.CreateCompatibleBitmap( pDC, r.right, r.bottom );
    m_pOldBitmap = m_back_dc.SelectObject( &m_back_bitmap );
	memcpy( &m_rBackDC, &r, sizeof( r ) );	// BACK DC의 크기를 보관해 둔다.
	m_bDC = TRUE;

	// 다시 그린다.
	redraw_back_dc();

	ReleaseDC( pDC );
}

void CFontEditView::delete_back_dc()
{
	if( m_bDC == FALSE )	
		return;

    m_back_dc.SelectObject( m_pOldBitmap );
	m_back_dc.DeleteDC();
	m_back_bitmap.DeleteObject();
	m_bDC = FALSE;
}

void CFontEditView::copy_back_image( RECT *pR, CDC *pDC )
{
	RECT	r;
	BOOL	bRelease;

	if( m_bDC == FALSE )
		return;

	// RECT를 구한다.
	if( pR == NULL )
	{
		pR = &r;
		GetClientRect( pR );
		pR->left   += VIEW_H_MARGIN;
	}

	// Current DC를 할당한다.
	if( pDC == NULL )
	{
		bRelease = TRUE;
		pDC = GetDC();
	}
	else
		bRelease = FALSE;
	
	// 이미지를 복사한다.
	pDC->BitBlt( pR->left, pR->top, pR->right - pR->left, pR->bottom - pR->top, 
	    &m_back_dc, 0, 0, SRCCOPY );

	// DC를 해제한다.
	if( bRelease )
		ReleaseDC( pDC );
}

void CFontEditView::draw_char( int nXPos, int nYPos, int nCharIndex, DWORD dwColor )
{
	int nX, nY;

	if( m_pFont == NULL || nCharIndex < 0 || nCharIndex > m_pFont->wEndChar - m_pFont->wStartChar )
		return;

	for( nY = 0; nY < m_pFont->cV; nY++ )
	{
		for( nX = 0; nX < m_pFont->cH; nX++ )
		{
			if( get_dot( nX, nY, nCharIndex ) != 0 )
				m_back_dc.SetPixel( nXPos + nX, nYPos + nY, dwColor );
		}
	}
}

void CFontEditView::draw_3d_box( int nX, int nY, int nH, int nV, DWORD dwColor1, DWORD dwColor2, CBrush *pBrush )
{
	RECT	r;

	m_back_dc.Draw3dRect( nX, nY, nH, nV, dwColor1, dwColor2  );

	r.left   = nX + 1;
	r.top    = nY + 1;
	r.right  = r.left + nH - 2;
	r.bottom = r.top  + nV - 2;
	m_back_dc.FillRect( &r, pBrush );
}

// back dc를 다시 그린다.
void CFontEditView::redraw_back_dc()
{
	char	szT[32];
	int		nHDotSize, nVDotSize;
	int		nX, nY, nH, nV, nYIndex, nXIndex;
							
	if( m_bDC == FALSE )
		return;

	// 배경색으로 지운다.
	m_back_dc.FillRect( &m_rBackDC, &m_back_brush );

	if( m_pFont == NULL )
	{
		sprintf( szT, "." );
		SetDlgItemText( IDC_CUR_CHAR, szT );
		SetDlgItemText( IDC_CHAR_DEC, szT );
		SetDlgItemText( IDC_CHAR_HEX, szT );
		UpdateData( FALSE );
		return;
	}
	
	// 픽셀을 그릴 영역.
	nX = 0;
	nY = VIEW_TOP_MARGIN;
	nH = m_rBackDC.right  - PREVIEW_H_MARGIN;
	nV = m_rBackDC.bottom - m_nPreViewVMargin - VIEW_TOP_MARGIN;

	//테두리를 두른다.
	//m_back_dc.Draw3dRect( nX, nY, nH, nV, 0x00888888, 0x00444444 );

	nHDotSize = nH / m_pFont->cH;
	nVDotSize = nV / m_pFont->cV;
	for( nYIndex = 0; nYIndex < m_pFont->cV; nYIndex++ )
	{
		for( nXIndex = 0; nXIndex < m_pFont->cH; nXIndex++ )
		{
			if( get_dot( nXIndex, nYIndex, m_nEditCharIndex ) == 0 )
				draw_3d_box( nX + (nXIndex*nHDotSize), nY + (nYIndex*nVDotSize), nHDotSize, nVDotSize,
						0x00666666, 0x00666666, WHITE_BRUSH );
			else
				draw_3d_box( nX + (nXIndex*nHDotSize), nY + (nYIndex*nVDotSize), nHDotSize, nVDotSize,
						0x00666666, 0x00666666, &m_black_brush );
		}
	}
	
	// preview 영역을 그린다.
	nX = m_rBackDC.right - PREVIEW_H_MARGIN;
	nH = PREVIEW_H_MARGIN;
	nY = VIEW_TOP_MARGIN;
	nV = m_pFont->cV + 2;
	nX = nX + ( (nH - (m_pFont->cH+2)) / 2);
	nH = m_pFont->cH+2;
	draw_3d_box( nX, nY, nH, nV, 
					0x00EEEEEE, 0x00EEEEEE, WHITE_BRUSH );
	draw_char( nX+1, nY+1, m_nEditCharIndex, 0x00000000 );

	// 모든 문자를 출력한다.
	{
		int nI, nTotal, nCharPreLine;

		nTotal = m_pFont->wEndChar - m_pFont->wStartChar;
		nX = 10;
		nY = (m_rBackDC.bottom - m_nPreViewVMargin) - m_pFont->cV;
		nCharPreLine = ( m_rBackDC.right - (nX*2) ) / m_pFont->cH;
		for( nI = 0; nI <= nTotal; nI++ )
		{
			if( ( nI % nCharPreLine ) == 0 )
			{
				nY += m_pFont->cV;
				nX = 10;
			}
			if( nI == m_nEditCharIndex )
				draw_char( nX, nY, nI, 0x000000FF );
			else
				draw_char( nX, nY, nI, 0x00000000 );
			nX += m_pFont->cH;
		}
	}



	// 편집중인 문자의 코드들을 설정한다.
	sprintf( szT, "%c", m_nEditCharIndex + m_pFont->wStartChar );
	SetDlgItemText( IDC_CUR_CHAR, szT );
	sprintf( szT, "%d", m_nEditCharIndex + m_pFont->wStartChar );
	SetDlgItemText( IDC_CHAR_DEC, szT );
	sprintf( szT, "0x%X", m_nEditCharIndex + m_pFont->wStartChar );
	SetDlgItemText( IDC_CHAR_HEX, szT );
	UpdateData( FALSE );
}

void CFontEditView::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	copy_back_image( NULL, (CDC*)&dc );
}

int CFontEditView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CFormView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// 브러시를 생성한다.
	m_back_brush.CreateSolidBrush( GetSysColor(	COLOR_BTNFACE ) );
	m_black_brush.CreateSolidBrush( 0x000000 );
	
	create_back_dc();
	
	return 0;
}

void CFontEditView::OnDestroy() 
{	
	delete_back_dc();
	
	// 브러시를 제거한다.
	m_back_brush.DeleteObject();
	m_black_brush.DeleteObject();

	if( m_pFont != NULL )
	{
		free( m_pFont );
		m_pFont = NULL;
	}
	
	CFormView::OnDestroy();
}

void CFontEditView::OnSize(UINT nType, int cx, int cy) 
{
	delete_back_dc();	

	CFormView::OnSize(nType, cx, cy);
	
	create_back_dc();	
}

int CFontEditView::get_dot( int nX, int nYIndex, int nCharIndex )
{
	int nIndent;

	// 현재 폰트의 문자의 x, y 도트를 토글한다.
	if( m_pFont == NULL )
		return( 0 );

	if( m_pFont->cLinByte == 1 )
	{
		BYTE *pB, byTe;
		pB = &m_pFont->b.pB [ nCharIndex*m_pFont->cV ];
		byTe = 0x80;
		nIndent = 8 - m_pFont->cH;
		if( nIndent > 0 )
			byTe = (BYTE)( byTe >> nIndent );
		if( nX > 0 )
			byTe = (BYTE)( byTe >> nX );
		if( pB[nYIndex] & byTe )
			return( 1 );
		else
			return( 0 );
	}
	else if( m_pFont->cLinByte == 2 )
	{
		unsigned short *pW, wX;
		pW = &m_pFont->b.pW [ nCharIndex*m_pFont->cV ];
		wX = 0x8000;
		nIndent = 16 - m_pFont->cH;
		if( nIndent > 0 )
			wX = (unsigned short)( wX >> nIndent );
		if( nX > 0 )
			wX = (unsigned short)( wX >> nX );
		if( pW[nYIndex] & wX )
			return( 1 );
		else
			return( 0 );
	}
	else if( m_pFont->cLinByte == 4 )
	{
		DWORD *pD, dwX;
		pD = &m_pFont->b.pD [ nCharIndex*m_pFont->cV ];
		dwX = 0x80000000;
		nIndent = 32 - m_pFont->cH;
		if( nIndent > 0 )
			dwX = (DWORD)( dwX >> nIndent );
		if( nX > 0 )
			dwX = (DWORD)( dwX >> nX );
		if( pD[nYIndex] & dwX )
			return( 1 );
		else
			return( 0 );
	}
	return( 0 );
}

void CFontEditView::toggle_dot( int nX, int nYIndex )
{
	int nIndent;

	// 현재 폰트의 문자의 x, y 도트를 토글한다.
	if( m_pFont == NULL )
		return;

	m_nToggledX = nX;
	m_nToggledY = nYIndex;

	if( m_pFont->cLinByte == 1 )
	{
		BYTE *pB, byTe;
		pB = &m_pFont->b.pB [ m_nEditCharIndex*m_pFont->cV ];
		byTe = 0x80;
		nIndent = 8 - m_pFont->cH;
		if( nIndent > 0 )
			byTe = (BYTE)( byTe >> nIndent );
		if( nX > 0 )
			byTe = (BYTE)( byTe >> nX );
		if( pB[nYIndex] & byTe )
			pB[nYIndex] &= (BYTE)~byTe;
		else
			pB[nYIndex] |= byTe;
	}
	else if( m_pFont->cLinByte == 2 )
	{
		unsigned short *pW, wX;
		pW = &m_pFont->b.pW [ m_nEditCharIndex*m_pFont->cV ];
		wX = 0x8000;
		nIndent = 16 - m_pFont->cH;
		if( nIndent > 0 )
			wX = (unsigned short)( wX >> nIndent );
		if( nX > 0 )
			wX = (unsigned short)( wX >> nX );
		if( pW[nYIndex] & wX )
			pW[nYIndex] &= (unsigned short)~wX;
		else
			pW[nYIndex] |= wX;
	}
	else if( m_pFont->cLinByte == 4 )
	{
		DWORD *pD, dwX;
		pD = &m_pFont->b.pD [ m_nEditCharIndex*m_pFont->cV ];
		dwX = 0x80000000;
		nIndent = 32 - m_pFont->cH;
		if( nIndent > 0 )
			dwX = (DWORD)( dwX >> nIndent );
		if( nX > 0 )
			dwX = (DWORD)( dwX >> nX );
		if( pD[nYIndex] & dwX )
			pD[nYIndex] &= (DWORD)~dwX;
		else
			pD[nYIndex] |= dwX;
	}
}

void CFontEditView::OnNextChar() 
{
	if( m_pFont == NULL )
		return;

	// 인덱스를 조정한다.
	m_nEditCharIndex++;
	if( m_nEditCharIndex > m_pFont->wEndChar - m_pFont->wStartChar )
		m_nEditCharIndex = m_pFont->wEndChar - m_pFont->wStartChar;
	
	// 화면을 다시 그린다.
	redraw_back_dc();
	InvalidateRect( NULL, FALSE );
}

void CFontEditView::OnPrevChar() 
{
	 // 인덱스를 조정한다.
	if( m_nEditCharIndex > 0 )
		m_nEditCharIndex--;
	
	// 화면을 다시 그린다.
	redraw_back_dc();
	InvalidateRect( NULL, FALSE );
}

void CFontEditView::point_to_dot_Index( CPoint point, int *pX, int *pY )
{
	int nX, nY, nH, nV;

	if( m_pFont == NULL )
		return;

	// Dot 영역인가?
	nX = VIEW_H_MARGIN;
	nY = VIEW_TOP_MARGIN;
	nH = ( m_rBackDC.right - PREVIEW_H_MARGIN );
	nV = m_rBackDC.bottom - VIEW_TOP_MARGIN - m_nPreViewVMargin;

	if( nX <= point.x && point.x < nX + nH )
	{
		if( nY <= point.y && point.y < nY + nV )
		{
			pX[0] = ( point.x - nX ) / ( nH / m_pFont->cH );
			pY[0] = ( point.y - nY ) / ( nV / m_pFont->cV );
			
			if( pX[0] < m_pFont->cH && pY[0] < m_pFont->cV )
				return;
		}
	}

	pX[0] = -1;
	pY[0] = -1;
}

void CFontEditView::process_dot( CPoint point )
{
	int nDotIndeX, nDotIndexY;

	point_to_dot_Index( point, &nDotIndeX, &nDotIndexY );

	if( nDotIndeX >= 0 && nDotIndexY >= 0 )
	{
		// 도트를 토클하고 화면을 다시 그린다.
		toggle_dot( nDotIndeX, nDotIndexY );
		redraw_back_dc();
		InvalidateRect( NULL, FALSE );
	}
}

void CFontEditView::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	//CFormView::OnMouseMove(nFlags, point);

	int nX, nY;

	point_to_dot_Index( point, &nX, &nY );

	if( m_nLButtonDn != 0 && (m_nToggledX != nX || m_nToggledY != nY) )
		process_dot( point );
}

void CFontEditView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	//CFormView::OnLButtonUp(nFlags, point);
	m_nLButtonDn = 0;
}

void CFontEditView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	m_nLButtonDn = 1;
	process_dot( point );

	//CFormView::OnLButtonDown(nFlags, point);
}

void CFontEditView::OnGoChar() 
{
	char	szT[32];

	if( m_pFont == NULL )
		return;

	UpdateData( TRUE );
	GetDlgItemText( IDC_CUR_CHAR, szT, 2 );

	if( szT[0] < m_pFont->wStartChar || m_pFont->wEndChar < szT[0]  )
		return;

	// 인덱스를 변경한다.
	m_nEditCharIndex = 	szT[0] - m_pFont->wStartChar;

	// 화면을 갱신한다.
	redraw_back_dc();
	InvalidateRect( NULL, FALSE );
}

void CFontEditView::OnGoDec() 
{
	char	szT[32];
	int		nDec;

	if( m_pFont == NULL )
		return;

	UpdateData( TRUE );
	GetDlgItemText( IDC_CHAR_DEC, szT, 10 );

	nDec = atoi( szT );
	if( nDec < m_pFont->wStartChar || m_pFont->wEndChar < nDec  )
		return;

	// 인덱스를 변경한다.
	m_nEditCharIndex = 	nDec - m_pFont->wStartChar;

	// 화면을 갱신한다.
	redraw_back_dc();
	InvalidateRect( NULL, FALSE );
}

static DWORD dwHexValue( char *pS )
{
	DWORD dwR;
	int   nI;

	dwR = 0;
	for( nI = 0; pS[nI] != 0; nI++ )
	{
		if( '0' <= pS[nI] && pS[nI] <= '9' )
		{
			dwR *= 16;
			dwR += (DWORD)( pS[nI] - '0' );
		}
		else if( 'A' <= pS[nI] && pS[nI] <= 'F' )
		{
			dwR *= 16;
			dwR += (DWORD)( pS[nI] - 'A' + 10 );
		}
		else if( 'a' <= pS[nI] && pS[nI] <= 'f' )
		{
			dwR *= 16;
			dwR += (DWORD)( pS[nI] - 'a' + 10 );
		}
	}

	return( dwR );
}

void CFontEditView::OnGoHex() 
{
	char	szT[32];
	int		nDec;

	if( m_pFont == NULL )
		return;

	UpdateData( TRUE );
	GetDlgItemText( IDC_CHAR_HEX, szT, 10 );

	nDec = dwHexValue( szT );
	if( nDec < m_pFont->wStartChar || m_pFont->wEndChar < nDec  )
		return;

	// 인덱스를 변경한다.
	m_nEditCharIndex = 	nDec - m_pFont->wStartChar;

	// 화면을 갱신한다.
	redraw_back_dc();
	InvalidateRect( NULL, FALSE );
}

// 폰트를 파일로 저장한다.
int CFontEditView::save_font( char *pS, FontStt *pFont )
{
	int nHandle, nBodySize;

	if( pS == NULL || pS[0] == 0 )
	{
		MessageBox( "Invalid filename!", "Error", 0 );
		return( -1 );
	}

	// 파일을 지우고 새로 생성해 버린다.
	remove( pS );
	nHandle = open( pS, _O_BINARY | _O_RDWR | _O_CREAT, _S_IREAD | _S_IWRITE );
	if( nHandle < 0 )
	{	// 파일을 생성할 수 없다.
		MessageBox( "File creation failed!", "Error", 0 );
		return( -1 );
	}

	// 헤더를 기록한다.
	write( nHandle, pFont, sizeof( FontStt ) );
	
	// body를 기록한다.
	nBodySize = (pFont->cLinByte * pFont->cV ) * ( pFont->wEndChar - pFont->wStartChar + 1); 
	write( nHandle, pFont->b.pB, nBodySize );
	close( nHandle );

	return( 0 );
}

extern char BASED_CODE szBFNFilter[];
void CFontEditView::OnSaveFile() 
{
	if( m_pFont == NULL )
		return;
	
	if( m_font_path[0] == 0 )
	{	// 파일 다이얼로그를 띄운다.
		int		nR;
		CFileDialog fdlg( FALSE, "bfn", NULL, OFN_OVERWRITEPROMPT, szBFNFilter, this );
		nR = fdlg.DoModal();
		if( IDOK != nR )
			return;
		strcpy( m_font_path, fdlg.GetPathName() );
	}

	// 파일이 지정되어 있으면 저장한다.
	if( m_font_path[0] != 0 )
		save_font( m_font_path, m_pFont );
}

// 새로운 폰트를 생성한다.
void CFontEditView::OnCreateNew() 
{
	CCreateNewDlg	dlg;
	char			szT[32];

	// 기존 폰트를 폐기할 것인지 확인한다.
	if( m_pFont != NULL )
	{
		if( MessageBox( "기존의 폰트를 폐기하시겠습니까?", "질문", MB_YESNO ) != IDYES )
			return;
	}			   

	if( dlg.DoModal() != IDOK )
		return;
	
	// 폰트 명
	strcpy( m_font_path, (char*)(LPCSTR)dlg.m_filename );

	// 기존 폰트가 있다면 먼저 제거한다.
	if( m_pFont != NULL )
	{
		free( m_pFont );
		m_pFont = NULL;
	}

	// 빈 폰트를 생성한다.  (파일명이 지정되어 있지 않으면 파일은 생성하지 않는다.)
	m_pFont = create_empty_font_file( m_font_path,
		dlg.m_hdots, dlg.m_vdots, dlg.m_startcode, dlg.m_endcode );

	if( m_pFont != NULL )
	{
		m_pFont->wStartChar = dlg.m_startcode;
		m_pFont->wEndChar   = dlg.m_endcode;
		m_pFont->cH			= dlg.m_hdots;
		m_pFont->cV			= dlg.m_vdots;
	
		sprintf( szT, "H=%02d", m_pFont->cH );
		SetDlgItemText( IDC_HSIZE, szT );
		sprintf( szT, "V=%02d", m_pFont->cV );
		SetDlgItemText( IDC_VSIZE, szT );
		m_nPreViewVMargin = m_pFont->cV * 6;
	}
	else
	{
		sprintf( szT, "." );
		SetDlgItemText( IDC_HSIZE, szT );
		SetDlgItemText( IDC_VSIZE, szT );
	}


	// 화면을 갱신한다.
	redraw_back_dc();
	InvalidateRect( NULL, FALSE );
}

FontStt *CFontEditView::load_font( char *pS )
{
	FontStt *pFont;
	int		nHandle, nSize;

	// 폰트를 연다.
	nHandle = open( pS, _O_BINARY | _O_RDONLY );
	if( nHandle < 0 )
	{	// 폰트를 열 수 없다.
		MessageBox( "Open Font Failed!", "error", 0 );
		return( NULL );
	}

	nSize = lseek( nHandle, 0, SEEK_END );
	if( nSize < sizeof( FontStt ) )
	{
ERR_FILE:
		// 파일을 닫고 돌아간다.
		close( nHandle );
		MessageBox( "Invalid File or Memory error!", "Error", 0 );
		return( NULL );
	}
	pFont = (FontStt*)malloc( nSize );
	if( pFont == NULL )
		goto ERR_FILE;		// 메모리를 할당할 수 없다.

	// 파일을 전부 읽어들인다.
	lseek( nHandle, 0, SEEK_SET );
	read( nHandle, pFont, nSize );

	// MAGIC을 확인해 본다.
	if( pFont->dwMagic != BELL_FONT_MAGIC )
		goto ERR_FILE;
		
	// 파일을 닫는다.
	close( nHandle );

	// 버퍼를 설정한다.
	pFont->b.pB = (BYTE*)( (DWORD)pFont + sizeof( FontStt ) );

	return( pFont );
}

// 기존의 폰트를 오픈한다.
void CFontEditView::OnOpenFont() 
{
	int		nR;
	char	szT[32];	

	// 기존 폰트를 폐기할 것인지 확인한다.
	if( m_pFont != NULL )
	{
		if( MessageBox( "기존의 폰트를 폐기하시겠습니까?", "질문", MB_YESNO ) != IDYES )
			return;
	}		
	
	// 읽어들일 폰트를 선택한다.
	CFileDialog fdlg( TRUE, "bfn", NULL, OFN_OVERWRITEPROMPT, szBFNFilter, this );
	nR = fdlg.DoModal();
	if( IDOK != nR )
		return;
	
	strcpy( m_font_path, fdlg.GetPathName() );

	// 폰트를 해제한다.
	free( m_pFont );
	m_pFont = NULL;
	
	// 폰트를 로드한다.
	m_pFont = load_font( m_font_path );
	if( m_pFont != NULL )
	{
		sprintf( szT, "H=%02d", m_pFont->cH );
		SetDlgItemText( IDC_HSIZE, szT );
		sprintf( szT, "V=%02d", m_pFont->cV );
		SetDlgItemText( IDC_VSIZE, szT );

		// Preview V Margin을 계산한다.
		m_nPreViewVMargin = m_pFont->cV * 6;
	}
	else
	{
		sprintf( szT, "." );
		SetDlgItemText( IDC_HSIZE, szT );
		SetDlgItemText( IDC_VSIZE, szT );
	}

	// 화면을 갱신한다.
	redraw_back_dc();
	InvalidateRect( NULL, FALSE );
}
