; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CFontEditView
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "FontEdit.h"
LastPage=0

ClassCount=6
Class1=CFontEditApp
Class2=CFontEditDoc
Class3=CFontEditView
Class4=CMainFrame

ResourceCount=4
Resource1=IDR_MAINFRAME
Resource2=IDD_ABOUTBOX
Class5=CAboutDlg
Resource3=IDD_FONTEDIT_FORM
Class6=CCreateNewDlg
Resource4=IDD_CREATE_NEW

[CLS:CFontEditApp]
Type=0
HeaderFile=FontEdit.h
ImplementationFile=FontEdit.cpp
Filter=N

[CLS:CFontEditDoc]
Type=0
HeaderFile=FontEditDoc.h
ImplementationFile=FontEditDoc.cpp
Filter=N

[CLS:CFontEditView]
Type=0
HeaderFile=FontEditView.h
ImplementationFile=FontEditView.cpp
Filter=D
BaseClass=CFormView
VirtualFilter=VWC
LastObject=CFontEditView


[CLS:CMainFrame]
Type=0
HeaderFile=MainFrm.h
ImplementationFile=MainFrm.cpp
Filter=T
BaseClass=CFrameWnd
VirtualFilter=fWC
LastObject=CMainFrame




[CLS:CAboutDlg]
Type=0
HeaderFile=FontEdit.cpp
ImplementationFile=FontEdit.cpp
Filter=D

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889

[MNU:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_FILE_SAVE_AS
Command5=ID_FILE_MRU_FILE1
Command6=ID_APP_EXIT
Command7=ID_EDIT_UNDO
Command8=ID_EDIT_CUT
Command9=ID_EDIT_COPY
Command10=ID_EDIT_PASTE
Command11=ID_VIEW_TOOLBAR
Command12=ID_APP_ABOUT
CommandCount=12

[ACL:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_EDIT_UNDO
Command5=ID_EDIT_CUT
Command6=ID_EDIT_COPY
Command7=ID_EDIT_PASTE
Command8=ID_EDIT_UNDO
Command9=ID_EDIT_CUT
Command10=ID_EDIT_COPY
Command11=ID_EDIT_PASTE
Command12=ID_NEXT_PANE
Command13=ID_PREV_PANE
CommandCount=13

[DLG:IDD_FONTEDIT_FORM]
Type=1
Class=CFontEditView
ControlCount=10
Control1=IDC_PREV_CHAR,button,1342213888
Control2=IDC_NEXT_CHAR,button,1342213888
Control3=IDC_CUR_CHAR,edit,1342242945
Control4=IDC_CHAR_DEC,edit,1342242945
Control5=IDC_CHAR_HEX,edit,1342242945
Control6=IDC_GO_CHAR,button,1342210048
Control7=IDC_GO_DEC,button,1342210048
Control8=IDC_GO_HEX,button,1342210048
Control9=IDC_HSIZE,static,1342177793
Control10=IDC_VSIZE,static,1342177793

[TB:IDR_MAINFRAME]
Type=1
Class=?
Command1=IDM_CREATE_NEW
Command2=IDM_SAVE_FILE
Command3=IDM_OPEN_FONT
Command4=ID_APP_ABOUT
CommandCount=4

[DLG:IDD_CREATE_NEW]
Type=1
Class=CCreateNewDlg
ControlCount=13
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_STATIC,static,1342308352
Control4=IDC_FILENAME,edit,1350631552
Control5=IDC_DDD,button,1342242816
Control6=IDC_STATIC,static,1342308352
Control7=IDC_H_DOTS,edit,1350639744
Control8=IDC_STATIC,static,1342308352
Control9=IDC_V_DOTS,edit,1350639744
Control10=IDC_STATIC,static,1342308352
Control11=IDC_START_CODE,edit,1350639744
Control12=IDC_STATIC,static,1342308352
Control13=IDC_END_CODE,edit,1350639744

[CLS:CCreateNewDlg]
Type=0
HeaderFile=CreateNewDlg.h
ImplementationFile=CreateNewDlg.cpp
BaseClass=CDialog
Filter=D
LastObject=IDOK
VirtualFilter=dWC

