// FontEditDoc.h : interface of the CFontEditDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_FONTEDITDOC_H__61D499CB_C7D2_4123_8B08_26AE8F256951__INCLUDED_)
#define AFX_FONTEDITDOC_H__61D499CB_C7D2_4123_8B08_26AE8F256951__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CFontEditDoc : public CDocument
{
protected: // create from serialization only
	CFontEditDoc();
	DECLARE_DYNCREATE(CFontEditDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFontEditDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CFontEditDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CFontEditDoc)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FONTEDITDOC_H__61D499CB_C7D2_4123_8B08_26AE8F256951__INCLUDED_)
