// FontEditDoc.cpp : implementation of the CFontEditDoc class
//

#include "stdafx.h"
#include "FontEdit.h"

#include "FontEditDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFontEditDoc

IMPLEMENT_DYNCREATE(CFontEditDoc, CDocument)

BEGIN_MESSAGE_MAP(CFontEditDoc, CDocument)
	//{{AFX_MSG_MAP(CFontEditDoc)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFontEditDoc construction/destruction

CFontEditDoc::CFontEditDoc()
{
}

CFontEditDoc::~CFontEditDoc()
{
}

BOOL CFontEditDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CFontEditDoc serialization

void CFontEditDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
	}
	else
	{
	}
}

/////////////////////////////////////////////////////////////////////////////
// CFontEditDoc diagnostics

#ifdef _DEBUG
void CFontEditDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CFontEditDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CFontEditDoc commands
