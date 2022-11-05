#if !defined(AFX_CREATENEWDLG_H__4AFF2C11_FC48_4F9E_8FCA_E8ADE147A97D__INCLUDED_)
#define AFX_CREATENEWDLG_H__4AFF2C11_FC48_4F9E_8FCA_E8ADE147A97D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CreateNewDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCreateNewDlg dialog

class CCreateNewDlg : public CDialog
{
// Construction
public:
	CCreateNewDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCreateNewDlg)
	enum { IDD = IDD_CREATE_NEW };
	CString	m_filename;
	int		m_hdots;
	int		m_endcode;
	int		m_startcode;
	int		m_vdots;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCreateNewDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCreateNewDlg)
	afx_msg void OnDdd();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CREATENEWDLG_H__4AFF2C11_FC48_4F9E_8FCA_E8ADE147A97D__INCLUDED_)
