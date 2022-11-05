// FontEdDoc.h : interface of the CFontEdDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_FONTEDDOC_H__E788384D_3B7C_4119_89BB_D96234C8D775__INCLUDED_)
#define AFX_FONTEDDOC_H__E788384D_3B7C_4119_89BB_D96234C8D775__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CFontEdDoc : public CDocument
{
protected: // create from serialization only
	CFontEdDoc();
	DECLARE_DYNCREATE(CFontEdDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFontEdDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CFontEdDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CFontEdDoc)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FONTEDDOC_H__E788384D_3B7C_4119_89BB_D96234C8D775__INCLUDED_)
