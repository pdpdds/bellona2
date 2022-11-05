// tt.h : main header file for the TT application
//

#if !defined(AFX_TT_H__08D98858_864A_40F3_AF42_BF7DFDDFA491__INCLUDED_)
#define AFX_TT_H__08D98858_864A_40F3_AF42_BF7DFDDFA491__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols


#define WM_TT_INITIALIZED   (WM_USER+98)
#define WM_TT_REPAINT		(WM_USER+99)
#define WM_TT_KEY_INPUT		(WM_USER+100)


/////////////////////////////////////////////////////////////////////////////
// CTtApp:
// See tt.cpp for the implementation of this class
//

class CTtApp : public CWinApp
{
public:
	CTtApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTtApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

public:
	//{{AFX_MSG(CTtApp)
	afx_msg void OnAppAbout();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TT_H__08D98858_864A_40F3_AF42_BF7DFDDFA491__INCLUDED_)
