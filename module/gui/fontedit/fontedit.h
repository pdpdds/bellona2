// FontEdit.h : main header file for the FONTEDIT application
//

#if !defined(AFX_FONTEDIT_H__260E58EE_74A8_4885_922D_47A7E63DB56E__INCLUDED_)
#define AFX_FONTEDIT_H__260E58EE_74A8_4885_922D_47A7E63DB56E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CFontEditApp:
// See FontEdit.cpp for the implementation of this class
//

class CFontEditApp : public CWinApp
{
public:
	CFontEditApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFontEditApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CFontEditApp)
	afx_msg void OnAppAbout();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FONTEDIT_H__260E58EE_74A8_4885_922D_47A7E63DB56E__INCLUDED_)
