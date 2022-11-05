// FontEdView.h : interface of the CFontEdView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_FONTEDVIEW_H__E282D1E9_20E5_453E_B114_65CC49E0EBEF__INCLUDED_)
#define AFX_FONTEDVIEW_H__E282D1E9_20E5_453E_B114_65CC49E0EBEF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "..\Font.h"

class CFontEdView : public CView
{
protected: // create from serialization only
	CFontEdView();
	DECLARE_DYNCREATE(CFontEdView)

// Attributes
public:
	int			m_nFontCreated;
	BOOL		m_bDC;                          // DC의 생성여부
	CDC			m_back_dc;                      // 배경 DC.
	CBitmap		m_back_bitmap, *m_pOldBitmap;   // 배경 DC에 연결될 비트맵
	RECT		m_rBackDC;                      // Back DC의 크기
	int			m_nWidth;
	int			m_nHeight;
	HFONT		m_hCurFont;
	
	void draw_font( CDC *pDC );
	void delete_back_dc();
	void create_back_dc();

	int make_font_file( char *pS );
	int save_char_data( int nHandle, FontStt* pBF, int nX, int nY );
	CFontEdDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFontEdView)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CFontEdView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CFontEdView)
	afx_msg void OnDestroy();
	afx_msg void OnFontSave();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnEditFontList();
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in FontEdView.cpp
inline CFontEdDoc* CFontEdView::GetDocument()
   { return (CFontEdDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FONTEDVIEW_H__E282D1E9_20E5_453E_B114_65CC49E0EBEF__INCLUDED_)
