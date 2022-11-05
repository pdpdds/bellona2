// FontEditView.h : interface of the CFontEditView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_FONTEDITVIEW_H__DEF7A8B5_7C2C_4FB9_B999_CD45E0C40979__INCLUDED_)
#define AFX_FONTEDITVIEW_H__DEF7A8B5_7C2C_4FB9_B999_CD45E0C40979__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\font.h"

class CFontEditView : public CFormView
{
protected: // create from serialization only
	CFontEditView();
	DECLARE_DYNCREATE(CFontEditView)

public:
	//{{AFX_DATA(CFontEditView)
	enum { IDD = IDD_FONTEDIT_FORM };
	//}}AFX_DATA
	CFontEditDoc* GetDocument();
	int save_font( char *pS, FontStt *pFont );
	void point_to_dot_Index( CPoint point, int *pX, int *pY );
	void OnSysKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags );
	void process_dot( CPoint point );
	int get_dot( int nDotIndexX, int nDotIndexY, int nCharIndex );
	void toggle_dot( int nDotIndexX, int nDotIndexY );
	FontStt *create_empty_font_file( char *pS, int nFontH, int nFontV, int nStartChar, int nEndChar );
	void create_back_dc();
	void delete_back_dc();
	void redraw_back_dc();
	void copy_back_image( RECT *pR, CDC *pDC );
	FontStt *load_font( char *pS );
	void draw_char( int nXPos, int nYPos, int nIndex, DWORD dwColor );
	void draw_3d_box( int nX, int nY, int nH, int nV, DWORD dwColor1, DWORD dwColor2, CBrush *pBrush );

// Attributes
public:
	BOOL		m_bDC;                          // DC의 생성여부
	CDC			m_back_dc;                      // 배경 DC.
	CBitmap		m_back_bitmap, *m_pOldBitmap;   // 배경 DC에 연결될 비트맵
	RECT		m_rBackDC;                      // Back DC의 크기
	CBrush		m_back_brush, m_black_brush;
	FontStt		*m_pFont;
	char		m_font_path[ 260 ];
	int			m_nLButtonDn;
	int			m_nToggledX, m_nToggledY;
	int			m_nPreViewVMargin;

	int			m_nEditCharIndex;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFontEditView)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnInitialUpdate(); // called first time after construct
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CFontEditView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CFontEditView)
	afx_msg void OnCreateNew();
	afx_msg void OnPaint();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnNextChar();
	afx_msg void OnPrevChar();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnGoChar();
	afx_msg void OnGoDec();
	afx_msg void OnGoHex();
	afx_msg void OnSaveFile();
	afx_msg void OnOpenFont();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in FontEditView.cpp
inline CFontEditDoc* CFontEditView::GetDocument()
   { return (CFontEditDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FONTEDITVIEW_H__DEF7A8B5_7C2C_4FB9_B999_CD45E0C40979__INCLUDED_)
