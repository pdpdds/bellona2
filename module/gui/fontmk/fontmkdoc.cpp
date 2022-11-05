// FontEdDoc.cpp : implementation of the CFontEdDoc class
//

#include "stdafx.h"
#include "FontMk.h"

#include "FontMkDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFontEdDoc

IMPLEMENT_DYNCREATE(CFontEdDoc, CDocument)

BEGIN_MESSAGE_MAP(CFontEdDoc, CDocument)
	//{{AFX_MSG_MAP(CFontEdDoc)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFontEdDoc construction/destruction

CFontEdDoc::CFontEdDoc()
{
}

CFontEdDoc::~CFontEdDoc()
{
}

BOOL CFontEdDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CFontEdDoc serialization

void CFontEdDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
	}
	else
	{
	}
}

/////////////////////////////////////////////////////////////////////////////
// CFontEdDoc diagnostics

#ifdef _DEBUG
void CFontEdDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CFontEdDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CFontEdDoc commands
