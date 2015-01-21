; CLW-Datei enthält Informationen für den MFC-Klassen-Assistenten

[General Info]
Version=1
LastClass=cRaViApp
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "RaVi.h"
LastPage=0

ClassCount=4
Class1=cRaViApp
Class3=CMainFrame
Class4=CAboutDlg

ResourceCount=4
Resource1=IDD_ABOUTBOX
Resource2=IDR_MAINFRAME
Class2=CChildView
Resource3=IDR_MAINFRAME (Englisch (USA))
Resource4=IDD_ABOUTBOX (Englisch (USA))

[CLS:cRaViApp]
Type=0
HeaderFile=ravi-demo.h
ImplementationFile=ravi-demo.cpp
BaseClass=CWinApp
Filter=N
VirtualFilter=AC
LastObject=ID_GIOnlyIndirect

[CLS:CChildView]
Type=0
HeaderFile=ChildView.h
ImplementationFile=ChildView.cpp
Filter=C
LastObject=CChildView
BaseClass=CWnd 
VirtualFilter=WC

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
HeaderFile=RaVi.cpp
ImplementationFile=RaVi.cpp
Filter=D
LastObject=CAboutDlg

[DLG:IDD_ABOUTBOX]
Type=1
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308352
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889
Class=CAboutDlg

[MNU:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command3=ID_FILE_SAVE
Command4=ID_FILE_SAVE_AS
Command5=ID_FILE_MRU_FILE1
Command6=ID_APP_EXIT
Command10=ID_EDIT_PASTE
Command11=ID_VIEW_STATUS_BAR
Command12=ID_APP_ABOUT
CommandCount=12
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command7=ID_EDIT_UNDO
Command8=ID_EDIT_CUT
Command9=ID_EDIT_COPY

[ACL:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command5=ID_EDIT_CUT
Command6=ID_EDIT_COPY
Command7=ID_EDIT_PASTE
Command8=ID_EDIT_UNDO
Command9=ID_EDIT_CUT
Command10=ID_EDIT_COPY
Command11=ID_EDIT_PASTE
Command12=ID_NEXT_PANE
CommandCount=13
Command4=ID_EDIT_UNDO
Command13=ID_PREV_PANE


[MNU:IDR_MAINFRAME (Englisch (USA))]
Type=1
Class=?
Command1=ID_DEMO_MIRRORSPHERE
Command2=ID_DEMO_SPHEREHEAP
Command3=ID_DEMO_3SPHERES
Command4=ID_DEMO_TORUS
Command5=ID_DEMO_CUBE
Command6=ID_DEMO_EGG
Command7=ID_DEMO_SHADOWS1
Command8=ID_DEMO_SHADOWS3
Command9=ID_DEMO_CORNELL
Command10=ID_DEMO_ANIM
Command11=ID_APP_EXIT
Command12=ID_QUALITY_1by1
Command13=ID_QUALITY_SUBSAMPLING
Command14=ID_AA_none
Command15=ID_AA_good
Command16=ID_AA_best
Command17=ID_CaIn_None
Command18=ID_CaIn_Kernel
Command19=ID_CaIn_Other
Command20=ID_Caln_Other2
Command21=ID_Caln_Diff
Command22=ID_Caln_Stack
Command23=ID_Caln_PreBiPatch
Command24=ID_CaSa_Coarse
Command25=ID_CaSa_Med
Command26=ID_CaSa_Hig
Command27=ID_CaQu_Noisy
Command28=ID_CaQu
Command29=ID_CaQu_Smooth
Command30=ID_CAUSTICS
Command31=ID_CaDisp
Command32=ID_GISa_Coarse
Command33=ID_GISa_Med
Command34=ID_GISa_Fine
Command35=ID_GIQu_Noisy
Command36=ID_GIQu_Med
Command37=ID_GIQu_Smooth
Command38=ID_GIOnlyIndirect
Command39=ID_GICalc
Command40=ID_GIDisp
Command41=ID_FILTER
Command42=ID_RES320
Command43=ID_RES480
Command44=ID_RES512
Command45=ID_RES640
Command46=ID_RES800
Command47=ID_SingleThread
Command48=ID_DualThread
Command49=ID_QuadThread
Command50=ID_SAMP_RAW
Command51=ID_SAMP_INT16
Command52=ID_SAMP_4
Command53=ID_SAMP_FULL
Command54=ID_SAMP_STD
Command55=ID_QUALITY_AD5
Command56=ID_QUALITY_AD10
Command57=ID_QUALITY_AD15
Command58=ID_QUALITY_AD20
Command59=ID_QUALITY_AD25
Command60=ID_QUALITY_AD30
Command61=ID_SHADER_DEPTH
Command62=ID_SHADER_NORMALS
Command63=ID_SHADER_REFLECTION
Command64=ID_SHADER_UV
Command65=ID_SHADER_SIMPLE
Command66=ID_SHADER_PHONG
Command67=ID_TEST_ASM
Command68=ID_APP_ABOUT
CommandCount=68

[ACL:IDR_MAINFRAME (Englisch (USA))]
Type=1
Class=?
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

[DLG:IDD_ABOUTBOX (Englisch (USA))]
Type=1
Class=CAboutDlg
ControlCount=8
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889
Control5=IDC_STATIC,static,1342308352
Control6=IDC_STATIC,static,1342308352
Control7=IDC_STATIC,static,1342308352
Control8=IDC_STATIC,static,1342308352

