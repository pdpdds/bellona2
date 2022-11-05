// FontEdView.cpp : implementation of the CFontEdView class
//

#include "stdafx.h"
#include "FontMk.h"

#include "FontMkDoc.h"
#include "FontMkView.h"

#include "..\font.h"

#include <io.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

char BASED_CODE szAllFilter[] = "All Files (*.*)|*.*||";
char szCurDir[260];

/////////////////////////////////////////////////////////////////////////////
// CFontEdView

IMPLEMENT_DYNCREATE(CFontEdView, CView)

BEGIN_MESSAGE_MAP(CFontEdView, CView)
	//{{AFX_MSG_MAP(CFontEdView)
	ON_WM_DESTROY()
	ON_COMMAND(IDT_FONT_SAVE, OnFontSave)
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_COMMAND(IDT_EDIT_FONT_LIST, OnEditFontList)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFontEdView construction/destruction

CFontEdView::CFontEdView()
{
	m_bDC   = FALSE;
	m_hCurFont = NULL;
	m_nFontCreated = 0;
}

CFontEdView::~CFontEdView()
{
}

BOOL CFontEdView::PreCreateWindow(CREATESTRUCT& cs)
{
	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CFontEdView drawing

//static char *pStr0 = "Bellona2 (c)Copyright 2002 OHJJ.";
static char *pStr1 = "abcdefghijklmnopqrstuvwxyz";
static char *pStr2 = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static char *pStr3 = "1234567890!@#$%^&*()";
static char *pStr4 = "`~-_=+\\|,<.>/?;:'\"[{]}";

void CFontEdView::draw_font( CDC *pDC )
{
	BOOL	bR;
	HFONT	hOldFont;
	int		nX, nY, nV, nI, nW, nIndent;
	char	szT[16];//, szT0[33], szT1[33], szT2[33], szT3[33];

	if( m_hCurFont == NULL )
		return;

	hOldFont = (HFONT)SelectObject( pDC->GetSafeHdc(), m_hCurFont );
	
	if( m_nWidth == 0 )
	{	// ��Ʈ�� ũ�Ⱑ 0���� �����Ǿ� ������ ���Ѵ�.
		bR = GetCharWidth32( pDC->GetSafeHdc(), 'W', 'W', &m_nWidth );
		if( bR == FALSE )
		{
			MessageBox( "��Ʈ ũ�⸦ ���� �� ����", "����", 0 );
			goto BACK;
		}
	}

	szT[1] = nY = 0;
	nV = m_nHeight;
	for( nI = 1; nI < 128; nI++ )
	{
		szT[0] = nI;
		nX = (nI % 32) * m_nWidth;
		nY = (nI / 32) * m_nHeight;
	
		// ���� �׸� ������ Width�� ���Ѵ�.
		GetCharWidth32( pDC->GetSafeHdc(), nI, nI, &nW );

		nIndent = (m_nWidth - nW) / 2;
		if( nIndent < 0 )
			nIndent = 0;

		
		TextOut( pDC->GetSafeHdc(), nX, nY, szT, 1 );

	}	

	/*
	nY = 0;
	nV = m_nHeight;
	// 0-127������ ���ڸ� �����.
	for( nI  = 0; nI < 32;  nI++ ) szT0[ nI % 32 ] = (char)nI; szT0[32] = 0;
	for(        ; nI < 64;  nI++ ) szT1[ nI % 32 ] = (char)nI; szT1[32] = 0;
	for(        ; nI < 96;  nI++ ) szT2[ nI % 32 ] = (char)nI; szT2[32] = 0;
	for(        ; nI < 128; nI++ ) szT3[ nI % 32 ] = (char)nI; szT3[32] = 0;
	
	TextOut( pDC->GetSafeHdc(), m_nWidth, nY, &szT0[1], strlen( &szT0[1] ) ); nY += nV;
	TextOut( pDC->GetSafeHdc(),        0, nY,     szT1, strlen( szT1 ) );		nY += nV;
	TextOut( pDC->GetSafeHdc(),        0, nY,     szT2, strlen( szT2 ) );		nY += nV;
	TextOut( pDC->GetSafeHdc(),        0, nY,     szT3, strlen( szT3 ) );		nY += nV;
	 */
BACK:
	SelectObject( pDC->GetSafeHdc(), hOldFont );
}

/////////////////////////////////////////////////////////////////////////////
// CFontEdView diagnostics

#ifdef _DEBUG
void CFontEdView::AssertValid() const
{
	CView::AssertValid();
}

void CFontEdView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CFontEdDoc* CFontEdView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CFontEdDoc)));
	return (CFontEdDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CFontEdView message handlers
void CFontEdView::OnDestroy() 
{	
    // DC�� �����Ѵ�.
    delete_back_dc();
			
	CView::OnDestroy();
}					 

int CFontEdView::save_char_data( int nHandle, FontStt *pBF, int nX, int nY )
{
	int		nV, nH;
	DWORD	dwK, dwColor;

	for( nV = 0; nV < (int)pBF->cV; nV++ )
	{
		dwK = 0;
		for( nH = 0; nH < (int)pBF->cH; nH++ )
		{
			dwColor = m_back_dc.GetPixel( nX + nH, nY + nV );
			dwK = dwK << 1;
			if( dwColor == 0 )	// 0 = ������
				dwK |= 1;
		}	

		// ���� ��Ʈ�� �� ���� ����
		write( nHandle, &dwK, pBF->cLinByte );
	}

	return( 0 );
}

void CFontEdView::create_back_dc()
{
	RECT	r;
	CDC		*pDC;

	if( m_bDC == TRUE )
		return;

	r.left = r.top = 0;
	r.right  = 800;
	r.bottom = 600;

	pDC = GetDC();
    m_back_dc.CreateCompatibleDC( pDC );
    m_back_bitmap.CreateCompatibleBitmap( pDC, r.right, r.bottom );
    m_pOldBitmap = m_back_dc.SelectObject( &m_back_bitmap );
	m_bDC = TRUE;
	memcpy( &m_rBackDC, &r, sizeof( r ) );	// BACK DC�� ũ�⸦ ������ �д�.

	ReleaseDC( pDC );
}

void CFontEdView::delete_back_dc()
{
	if( m_bDC == FALSE )	
		return;

    m_back_dc.SelectObject( m_pOldBitmap );
	m_back_dc.DeleteDC();
	m_back_bitmap.DeleteObject();
	m_bDC = FALSE;
}

void CFontEdView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	
	// DC�� ũ�⸦ �����Ѵ�.
	delete_back_dc();
	create_back_dc();

	m_nFontCreated = 0;

	// ȭ���� �����.
	{
		CDC *pDC;
		RECT r;

		pDC = GetDC();
			GetClientRect( &r );
			pDC->FillRect( &r, WHITE_BRUSH );
		ReleaseDC( pDC );
	}
}

int CFontEdView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// ��� DC�� �����Ѵ�.
	create_back_dc();
	
	return 0;
}

void CFontEdView::OnInitialUpdate() 
{
	CView::OnInitialUpdate();
	
	// ���� ���丮�� �˾Ƴ���.
	GetModuleFileName( NULL, szCurDir, sizeof( szCurDir ) -1 );
	for( int nI = strlen( szCurDir )-1; nI > 0; nI-- )
	{
		if( szCurDir[nI] == '\\' || szCurDir[nI] == ':' )
		{
			szCurDir[nI+1] = 0;
			break;
		}
	}
}

// SPACE, TAB, CR, LF�� �ǳʶڴ�.
static char *skip_white_space( char *pS )
{
	if( pS == NULL )
		return( NULL );

	for( ;; pS++ )
	{
		if( pS[0] == ' ' || pS[0] == 9 || pS[0]	== 10 || pS[0] == 13 )
			continue;
		return( pS );
	} 
	return( pS );
}

// white space���� Ȯ���Ѵ�.
static int is_white_space( char *pS )
{
	if( pS[0] == ' ' || pS[0] == 9 || pS[0]	== 10 || pS[0] == 13 )
		return( 1 );
	return( 0 );
}

// ���忡�� �� �ܾ ���Ѵ�.
static char *get_word( char *pS, char *pWord, int nMaxWord )
{
	int	nI;

	pWord[0] = 0;
	if( pS == NULL )
		return( pS );
	
	// ������ �ǳʶڴ�.
	pS = skip_white_space( pS );
	
	if( pS[0] == 0 )
		return( pS );

	// ��ȣ�� �ѷ�����.
	if( pS[0] == '"' )
	{
		pS++;
		for( nI = 0; ; )
		{
			if( pS[nI] == '"' )
				return( &pS[nI+1] );	   // "�� ������.
			else if( pS[nI] == 0 )
				return( &pS[nI] );		   // 0�� ������.
			
			if( nI < nMaxWord-1 )
			{
				pWord[nI] = pS[nI];
				nI++;
				pWord[nI] = 0;
			}
			else
				nI++;
		}
	}

	// �ܾ �����Ѵ�.
	for( nI = 0; ; )
	{
		if( nI < nMaxWord - 1 )
			pWord[nI] = pS[nI];
		if( pS[nI] == 0 )
			break;
		nI++;
		if( nI < nMaxWord )
			pWord[nI] = 0;

		if( is_white_space( &pS[nI] ) != 0 )
			break;
	} 

	return( &pS[nI] );
}

// ������ ���Ͽ� ��ϵ� ��Ʈ�̸�, ����, ���ϸ����� ��Ʈ�� �����Ѵ�.
int CFontEdView::make_font_file( char *pS )	// pS = "Font_name   height   new_font_file_name"
{
	FontStt		hd;
	HFONT		hFont;
	int			nHeight;
	int			nR, nK, nI, nX, nY, nHandle;
	char		szFontName[64], szHeight[32], szPath[260], szT[260];

	pS = get_word( pS, szFontName, sizeof( szFontName ) );
	pS = get_word( pS, szHeight,   sizeof( szHeight   ) );
	pS = get_word( pS, szPath,     sizeof( szPath     ) );
						
	nHeight = atoi( szHeight );
	if( szFontName[0] == 0 || nHeight <= 0 )
		return( -1 );

	// ��Ʈ�� �����Ѵ�. 
	m_nWidth = 0;
	m_nHeight = nHeight;
	//hFont = CreateFont( nHeight,0, 0,0, FW_THIN, 0,0,0, 
	hFont = CreateFont( nHeight,0, 0,0, FW_THIN, 0,0,0, 
				ANSI_CHARSET, OUT_DEFAULT_PRECIS, 
				CLIP_DEFAULT_PRECIS, 
				PROOF_QUALITY, 
				FF_MODERN | FIXED_PITCH, 
				//FF_MODERN | VARIABLE_PITCH, 
				szFontName ); 
	if( hFont == NULL )
		return( -1 );		// ��Ʈ�� ������ �� ����.

	// ȭ���� �����Ѵ�.
	m_hCurFont = hFont;
	{
		CDC *pDC;
		RECT r;

		pDC = GetDC();
			GetClientRect( &r );
			pDC->FillRect( &r, WHITE_BRUSH );
			draw_font( pDC );
		ReleaseDC( pDC );
	
	}
	
	// �н��� �����Ǿ� ���� ������ ȭ�鿡�� ���̰� �׳� ���ư���.
	if( szPath[0] == 0 )
		return( -1 );	

	// ���� �н��� �����Ѵ�.
	if( szPath[1] != ':' )
	{
		strcpy( szT, szPath );
		sprintf( szPath, "%s%s", szCurDir, szT );
	}

	// ����� �����Ѵ�. 
	memset( &hd, 0, sizeof( hd ) );
	hd.cBitPerPixel = 1;
	hd.dwVersion    = 1;
	hd.dwMagic      = BELL_FONT_MAGIC;
	hd.wStartChar   = 0;
	hd.wEndChar     = 127;
	hd.cH = (char)m_nWidth;
	hd.cV = (char)m_nHeight;
	nK = ( (int)hd.cBitPerPixel * (int)hd.cH );
	if( 0 < nK && nK <= 8 )	    hd.cLinByte = 1; 	// 1 ����Ʈ
	else if( nK <= 16 ) hd.cLinByte = 2;			// 2 ����Ʈ
	else if( nK <= 32 ) hd.cLinByte = 4; 			// 4 ����Ʈ
	else
	{
		MessageBox( "hd.cLinByte�� �߸��Ǿ���", "����", 0 );
		return(-1);
	}

	// ������ �����Ͽ� ����� ����Ѵ�. 
	remove( szT );
	nHandle = open( szPath, _O_BINARY | _O_RDWR | _O_CREAT, _S_IREAD | _S_IWRITE );
	if( nHandle < 0 )
	{
		MessageBox( "������ ������ �� ����.", "����", 0 );
		return(-1);
	}
	write( nHandle, &hd, sizeof( hd ) );

	// DC�� ������ �� ���۷� �����Ѵ�.
	nR = 0;
	m_back_dc.FillRect( &m_rBackDC, WHITE_BRUSH );
	// back dc�� ȭ�鿡 �׷ȴ� ���� �״�θ� �׸���.
	draw_font( &m_back_dc );
	m_nFontCreated = 1;

	// �ȼ� ���� ���ͼ� ���������� �����Ѵ�. 
	nR = 0;
	for( nI = (int)hd.wStartChar; nI <= (int)hd.wEndChar; nI++ )
	{	// �ȼ��� X
		nX = ( nI % 32 ); 
		nX = nX * (int)hd.cH;

		// �ʼ��� Y
		nY = ( nI / 32 );
		nY = nY * (int)hd.cV;

		// �� ���ڵ��� �ȼ��� �о� BYTE, WORD, DWORD�� �°� ������ �� �����Ѵ�. 
		nR = save_char_data( nHandle, &hd, nX, nY );
		if( nR < 0 )
		{
			MessageBox( "GetPixel() - error", "ERROR", MB_OK );
			break;
		}
	}	

	// ������ �ݰ� �����Ѵ�. 
	close( nHandle );
	if( nR < 0 )
	{	// ������ �߻������� ������ ����������.	
		remove( szT );
		return( -1 );
	}

	// ��Ʈ�� �����Ѵ�.
	DeleteObject( hFont );
	m_hCurFont = NULL;

	return( 0 );
}
static char *kill_crlf( char *pS )
{
	int nI;

	for( nI = strlen( pS ) -1; nI >= 0; nI-- )
	{
		if( pS[nI] == 10 || pS[nI] == 13 )
			pS[nI] = 0;
	}				   

	return( pS );
}

void CFontEdView::OnFontSave() 
{
	// ������ ������ �д´�.
	int		nR, nTotal, nMessageBox;
	char	szT[260], *pS;
	FILE	*pF;

	sprintf( szT, "%s%s", szCurDir, "fonted.txt" );
	pF = fopen( szT, "r" );
	if( pF == NULL )
	{	// ������ ������ �� �� ����.
		MessageBox( szT, "Open Error", 0 );
		return;
	}

	nTotal = 0;
	nMessageBox = 1;
	for( ;; )
	{	// �� �پ� �о ó���Ѵ�.
		pS = fgets( szT, sizeof( szT )-1, pF );
		if( pS == NULL )
			break;

		// CRLF�� �����Ѵ�.
		kill_crlf( pS );

		if( szT[0] == ';' )
			continue;
		if( strcmpi( szT, "stop" ) == 0 )
		{	// �� ���� ������ �����ߴٴ� �޽��� �ڽ��� ������� �ʰ� �׳� ���ư�.
			nMessageBox = 0;
			break;
		}	 

		// ������ ���� ��Ʈ ������ �����Ѵ�.  (font_name, height, font_path)
		nR = make_font_file( szT );
		if( nR == 0 )
			nTotal++;
	}

	fclose( pF );

	// �� ���� ������ �����ߴٴ� �޽��� �ڽ��� ����.
	if( nMessageBox != 0 )
	{
		sprintf( szT, "%d���� ��Ʈ�� �����Ͽ����ϴ�.", nTotal );
		MessageBox( szT, "����", 0 );
	}
}

// fonted.txt ������ Notepad�� ����.
void CFontEdView::OnEditFontList() 
{
	int		nR;
	char	szT[260];

	sprintf( szT, "NOTEPAD %s%s", szCurDir, "fonted.txt" );
	nR = WinExec( szT, SW_NORMAL );
	if( nR < 31 )
	{
		MessageBox( szT, "Error", 0 );
		return;
	}
}

void CFontEdView::OnPaint() 
{
	if( m_nFontCreated == 0 )
		return;

	CPaintDC dc(this); // device context for painting

	RECT	r;
	GetClientRect( &r );

	dc.BitBlt( 0, 0, r.right, r.bottom, &m_back_dc, 0, 0, SRCCOPY );

}

// �̰��� ���ָ� ������ ����. (??)
void CFontEdView::OnDraw(CDC* pDC) 
{
	
}
