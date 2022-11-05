// FontEd.h : main header file for the FONTED application
//

#if !defined(AFX_FONTED_H__BF8CC71C_CA8C_47EB_BFDC_BC4100071DD7__INCLUDED_)
#define AFX_FONTED_H__BF8CC71C_CA8C_47EB_BFDC_BC4100071DD7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CFontEdApp:
// See FontEd.cpp for the implementation of this class
//

class CFontEdApp : public CWinApp
{
public:
	CFontEdApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFontEdApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CFontEdApp)
	afx_msg void OnAppAbout();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FONTED_H__BF8CC71C_CA8C_47EB_BFDC_BC4100071DD7__INCLUDED_)
