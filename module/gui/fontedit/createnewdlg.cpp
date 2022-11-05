// CreateNewDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FontEdit.h"
#include "CreateNewDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCreateNewDlg dialog


CCreateNewDlg::CCreateNewDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCreateNewDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCreateNewDlg)
	m_filename = _T("");
	m_hdots = 0;
	m_endcode = 0;
	m_startcode = 0;
	m_vdots = 0;
	//}}AFX_DATA_INIT
}


void CCreateNewDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCreateNewDlg)
	DDX_Text(pDX, IDC_FILENAME, m_filename);
	DDX_Text(pDX, IDC_H_DOTS, m_hdots);
	DDV_MinMaxInt(pDX, m_hdots, 4, 32);
	DDX_Text(pDX, IDC_END_CODE, m_endcode);
	DDV_MinMaxInt(pDX, m_endcode, 1, 255);
	DDX_Text(pDX, IDC_START_CODE, m_startcode);
	DDV_MinMaxInt(pDX, m_startcode, 0, 254);
	DDX_Text(pDX, IDC_V_DOTS, m_vdots);
	DDV_MinMaxInt(pDX, m_vdots, 4, 32);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCreateNewDlg, CDialog)
	//{{AFX_MSG_MAP(CCreateNewDlg)
	ON_BN_CLICKED(IDC_DDD, OnDdd)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCreateNewDlg message handlers
char BASED_CODE szBFNFilter[] = "BFN Files (*.bfn)|*.bfn|All Files (*.*)|*.*||";
void CCreateNewDlg::OnDdd() 
{
	int		nR;

	CFileDialog fdlg( FALSE, "bfn", NULL, OFN_OVERWRITEPROMPT, szBFNFilter, this );

	nR = fdlg.DoModal();
	if( IDOK != nR )
		return;

	m_filename = fdlg.GetPathName();
	UpdateData( FALSE );	
}

BOOL CCreateNewDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_startcode = 0;
	m_endcode   = 127;
	m_hdots = 6;
	m_vdots = 9;
	UpdateData( FALSE );	

	return TRUE;  
}

void CCreateNewDlg::OnOK() 
{
	UpdateData( TRUE );

	CDialog::OnOK();
}
