# Microsoft Developer Studio Project File - Name="GUI" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=GUI - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "GUI.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "GUI.mak" CFG="GUI - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "GUI - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "GUI - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "GUI - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /G4 /Zp1 /W3 /Od /Gf /X /I "\oh\test" /I "\oh\test\h" /I "\oh\test\h\common" /u /FAcs /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x412 /d "NDEBUG"
# ADD RSC /l 0x412 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 /nologo /entry:"gui_main" /subsystem:console /map /machine:I386 /nodefaultlib
# SUBTRACT LINK32 /incremental:yes

!ELSEIF  "$(CFG)" == "GUI - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /G4 /Zp1 /W3 /Od /Gf /X /I "..\.." /I "..\..\h" /I "..\..\h\common" /u /FAcs /Fr /FD /c
# ADD BASE RSC /l 0x412 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /entry:"gui_main" /subsystem:console /pdb:none /map /machine:I386 /nodefaultlib /out:"Debug/GUI.MOD" /fixed:no
# SUBTRACT LINK32 /debug
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy debug\gui.mod debug\gui.exe	..\..\disk\codemap debug\gui.map debug\gui.mod	copy debug\gui.mod ..\..\disk\gui.mod /y	copy debug\gui.lib ..\..\lib\gui.lib /y
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "GUI - Win32 Release"
# Name "GUI - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\about.c
# End Source File
# Begin Source File

SOURCE=.\bitmap.c
# End Source File
# Begin Source File

SOURCE=.\button.c
# End Source File
# Begin Source File

SOURCE=.\draw.c
# End Source File
# Begin Source File

SOURCE=.\flatw.c
# End Source File
# Begin Source File

SOURCE=.\font.c
# End Source File
# Begin Source File

SOURCE=.\framew.c
# End Source File
# Begin Source File

SOURCE=.\grxcall.c
# End Source File
# Begin Source File

SOURCE=.\GUI.c
# End Source File
# Begin Source File

SOURCE=.\gui.rc
# End Source File
# Begin Source File

SOURCE=.\guiexp.c
# End Source File
# Begin Source File

SOURCE=.\iconwin.c
# End Source File
# Begin Source File

SOURCE=.\kcmdwin.c
# End Source File
# Begin Source File

SOURCE=.\mbox.c
# End Source File
# Begin Source File

SOURCE=.\menu.c
# End Source File
# Begin Source File

SOURCE=.\mpointer.c
# End Source File
# Begin Source File

SOURCE=.\simplew.c
# End Source File
# Begin Source File

SOURCE=.\syscolor.c
# End Source File
# Begin Source File

SOURCE=.\taskbar.c
# End Source File
# Begin Source File

SOURCE=.\wall.c
# End Source File
# Begin Source File

SOURCE=.\win.c
# End Source File
# Begin Source File

SOURCE=.\WinRes.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\button.h
# End Source File
# Begin Source File

SOURCE=.\draw.h
# End Source File
# Begin Source File

SOURCE=.\gui.h
# End Source File
# Begin Source File

SOURCE=.\guiobj.h
# End Source File
# Begin Source File

SOURCE=.\kcmdwin.h
# End Source File
# Begin Source File

SOURCE=.\wall.h
# End Source File
# Begin Source File

SOURCE=.\win.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\about.ico
# End Source File
# Begin Source File

SOURCE=.\res\apps.ico
# End Source File
# Begin Source File

SOURCE=.\res\arrow_il.cur
# End Source File
# Begin Source File

SOURCE=.\res\b2k.bmp
# End Source File
# Begin Source File

SOURCE=.\res\c_arrow.cur
# End Source File
# Begin Source File

SOURCE=.\res\c_rs_h.cur
# End Source File
# Begin Source File

SOURCE=.\res\c_rs_ul.cur
# End Source File
# Begin Source File

SOURCE=.\res\c_rs_ur.cur
# End Source File
# Begin Source File

SOURCE=.\res\c_rs_v.cur
# End Source File
# Begin Source File

SOURCE=.\res\clock.ico
# End Source File
# Begin Source File

SOURCE=.\res\cmd.ico
# End Source File
# Begin Source File

SOURCE=.\res\cmd_icon.ico
# End Source File
# Begin Source File

SOURCE=.\res\Exit.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon2.ico
# End Source File
# Begin Source File

SOURCE=.\res\korea.ico
# End Source File
# Begin Source File

SOURCE=.\res\logo8.bmp
# End Source File
# Begin Source File

SOURCE=.\res\maximize.ico
# End Source File
# Begin Source File

SOURCE=.\res\mbox.ico
# End Source File
# Begin Source File

SOURCE=".\res\Millenium pen.cur"
# End Source File
# Begin Source File

SOURCE=.\res\minimize.ico
# End Source File
# Begin Source File

SOURCE=.\res\more.ico
# End Source File
# Begin Source File

SOURCE=.\res\my_com.ico
# End Source File
# Begin Source File

SOURCE=.\res\my_com1.ico
# End Source File
# Begin Source File

SOURCE=.\res\mycom.ico
# End Source File
# Begin Source File

SOURCE=.\res\quit.ico
# End Source File
# Begin Source File

SOURCE=.\res\sysinfo.ico
# End Source File
# Begin Source File

SOURCE=.\res\tetris.ico
# End Source File
# Begin Source File

SOURCE=".\res\wall-b1.bmp"
# End Source File
# Begin Source File

SOURCE=.\res\WALL0.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Wall1.BMP
# End Source File
# Begin Source File

SOURCE=.\res\Wall2.BMP
# End Source File
# End Group
# Begin Source File

SOURCE=.\res\base11.bfn
# End Source File
# Begin Source File

SOURCE=.\base12.bfn
# End Source File
# Begin Source File

SOURCE=.\res\base12.bfn
# End Source File
# Begin Source File

SOURCE=.\base14.bfn
# End Source File
# Begin Source File

SOURCE=.\res\base14.bfn
# End Source File
# Begin Source File

SOURCE=.\res\gulim11.bfn
# End Source File
# Begin Source File

SOURCE=.\res\simple9.bfn
# End Source File
# Begin Source File

SOURCE=.\res\sys12.bfn
# End Source File
# Begin Source File

SOURCE=.\sys12.bfn
# End Source File
# Begin Source File

SOURCE=.\res\sys14.bfn
# End Source File
# Begin Source File

SOURCE=.\sys14.bfn
# End Source File
# Begin Source File

SOURCE=..\..\lib\bellona2.lib
# End Source File
# End Target
# End Project
