; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CFontEdView
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "FontEd.h"
LastPage=0

ClassCount=5
Class1=CFontEdApp
Class2=CFontEdDoc
Class3=CFontEdView
Class4=CMainFrame

ResourceCount=2
Resource1=IDR_MAINFRAME
Class5=CAboutDlg
Resource2=IDD_ABOUTBOX

[CLS:CFontEdApp]
Type=0
HeaderFile=FontEd.h
ImplementationFile=FontEd.cpp
Filter=N

[CLS:CFontEdDoc]
Type=0
HeaderFile=FontEdDoc.h
ImplementationFile=FontEdDoc.cpp
Filter=N

[CLS:CFontEdView]
Type=0
HeaderFile=FontEdView.h
ImplementationFile=FontEdView.cpp
Filter=C
BaseClass=CView
VirtualFilter=VWC
LastObject=CFontEdView


[CLS:CMainFrame]
Type=0
HeaderFile=MainFrm.h
ImplementationFile=MainFrm.cpp
Filter=T
LastObject=CMainFrame




[CLS:CAboutDlg]
Type=0
HeaderFile=FontEd.cpp
ImplementationFile=FontEd.cpp
Filter=D

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=4
Control1=IDC_STATIC,static,1342308480
Control2=IDC_STATIC,static,1342308352
Control3=IDOK,button,1342373889
Control4=IDC_STATIC,static,1342177283

[MNU:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_APP_EXIT
Command2=ID_VIEW_TOOLBAR
Command3=ID_VIEW_STATUS_BAR
Command4=ID_APP_ABOUT
CommandCount=4

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

[TB:IDR_MAINFRAME]
Type=1
Class=?
Command1=IDT_EDIT_FONT_LIST
Command2=IDT_FONT_SAVE
CommandCount=2

