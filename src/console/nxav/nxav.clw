; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CRequestProcessingDlg
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "nxav.h"
LastPage=0

ClassCount=7
Class1=CAlarmViewApp
Class3=CMainFrame
Class4=CAboutDlg

ResourceCount=4
Resource1=IDR_MAINFRAME
Resource2=IDD_ABOUTBOX
Class2=CChildView
Class5=CAlarmList
Class6=CInfoLine
Class7=CRequestProcessingDlg
Resource3=IDD_REQUEST_WAIT
Resource4=IDM_CONTEXT

[CLS:CAlarmViewApp]
Type=0
HeaderFile=nxav.h
ImplementationFile=nxav.cpp
Filter=N

[CLS:CChildView]
Type=0
HeaderFile=ChildView.h
ImplementationFile=ChildView.cpp
Filter=N

[CLS:CMainFrame]
Type=0
HeaderFile=MainFrm.h
ImplementationFile=MainFrm.cpp
Filter=W
BaseClass=CFrameWnd
VirtualFilter=fWC




[CLS:CAboutDlg]
Type=0
HeaderFile=nxav.cpp
ImplementationFile=nxav.cpp
Filter=D

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889

[ACL:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_EDIT_COPY
Command2=ID_EDIT_PASTE
Command3=ID_EDIT_UNDO
Command4=ID_EDIT_CUT
Command5=ID_NEXT_PANE
Command6=ID_PREV_PANE
Command7=ID_EDIT_COPY
Command8=ID_EDIT_PASTE
Command9=ID_EDIT_CUT
Command10=ID_EDIT_UNDO
CommandCount=10

[CLS:CAlarmList]
Type=0
HeaderFile=AlarmList.h
ImplementationFile=AlarmList.cpp
BaseClass=CListCtrl
Filter=W
VirtualFilter=FWC

[CLS:CInfoLine]
Type=0
HeaderFile=InfoLine.h
ImplementationFile=InfoLine.cpp
BaseClass=CWnd
Filter=W
VirtualFilter=WC
LastObject=CInfoLine

[DLG:IDD_REQUEST_WAIT]
Type=1
Class=CRequestProcessingDlg
ControlCount=2
Control1=IDC_STATIC,static,1342177283
Control2=IDC_INFO_TEXT,static,1342308352

[CLS:CRequestProcessingDlg]
Type=0
HeaderFile=RequestProcessingDlg.h
ImplementationFile=RequestProcessingDlg.cpp
BaseClass=CDialog
Filter=D
LastObject=CRequestProcessingDlg

[MNU:IDM_CONTEXT]
Type=1
Class=?
Command1=ID_VIEW_REFRESH
CommandCount=1

