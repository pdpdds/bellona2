# Microsoft Developer Studio Project File - Name="bell_fs" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=bell_fs - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "bell_fs.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "bell_fs.mak" CFG="bell_fs - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "bell_fs - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "bell_fs - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "bell_fs - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "..\\" /I "..\h" /D "BELLONA2" /D "BELL_FS" /YX /FD /c
# ADD BASE RSC /l 0x412
# ADD RSC /l 0x412
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "bell_fs - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G4 /Zp1 /W3 /WX /Zi /Od /Gf /X /I "..\\" /I "..\h" /u /D "BELLONA2" /D "BELL_FS" /FAcs /FD /Yd /c
# ADD BASE RSC /l 0x412
# ADD RSC /l 0x412
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=xcopy debug\*.COD ..\debug\bell_fs /y /D	copy debug\bell_fs.lib ..\lib /y
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "bell_fs - Win32 Release"
# Name "bell_fs - Win32 Debug"
# Begin Source File

SOURCE=.\bcache.c
# End Source File
# Begin Source File

SOURCE=.\blkdev.c
# End Source File
# Begin Source File

SOURCE=.\digit.c
# End Source File
# Begin Source File

SOURCE=.\f16part.c
# End Source File
# Begin Source File

SOURCE=.\fat32.c
# End Source File
# Begin Source File

SOURCE=.\fdd35.c
# End Source File
# Begin Source File

SOURCE=.\fddfat12.c
# End Source File
# Begin Source File

SOURCE=.\file.c
# End Source File
# Begin Source File

SOURCE=.\hddpart.c
# End Source File
# Begin Source File

SOURCE=.\idehdd.c
# End Source File
# Begin Source File

SOURCE=.\jcommand.c
# End Source File
# Begin Source File

SOURCE=.\ramdisk.c
# End Source File
# Begin Source File

SOURCE=.\stdinout.c
# End Source File
# Begin Source File

SOURCE=.\vfs.c
# End Source File
# End Target
# End Project
