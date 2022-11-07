# Microsoft Developer Studio Project File - Name="Bellona2" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=Bellona2 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Bellona2.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Bellona2.mak" CFG="Bellona2 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Bellona2 - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "Bellona2 - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Bellona2 - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /G4 /Zp1 /W3 /vmg /vd0 /GX /Od /Gf /I ".\\" /I ".\h" /D "BELLONA" /FAc /FD /c
# SUBTRACT CPP /Gy /Fr /YX
# ADD BASE RSC /l 0x412 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 /nologo /base:"0x400000" /entry:"bellona2_main" /subsystem:console /map /machine:I386 /nodefaultlib /out:"Release/Bellona.bin"
# SUBTRACT LINK32 /incremental:yes /debug

!ELSEIF  "$(CFG)" == "Bellona2 - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /G4 /Zp1 /W3 /vd0 /Od /Gf /X /I ".\\" /I ".\h" /I ".\h\common" /u /D "BELLONA2" /FAcs /FD /Yd /c
# ADD BASE RSC /l 0x412 /d "_DEBUG"
# ADD RSC /l 0x412 /x /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 lib\asm.lib /nologo /base:"0x400000" /entry:"bellona2_main" /subsystem:console /pdb:none /map /machine:I386 /nodefaultlib /out:"debug/bellona2.org" /FIXED
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy debug\*.cod cod /y	copy debug\*.map cod /y 	copy debug\bellona2.lib disk	copy debug\bellona2.org debug\bellona2-d.bin	copy debug\bellona2.org debug\bellona2-r.bin	cod\codemap \oh\os\bellona2\cod\bellona2.map ..\..\bellona2\debug\bellona2-d.bin	stubv86\release\stubv86 debug\bellona2-d.bin v86lib\v86lib.bin	stubv86\release\stubv86 debug\bellona2-r.bin v86lib\v86lib.bin	copy debug\bellona2-d.bin disk	copy debug\bellona2-r.bin disk
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "Bellona2 - Win32 Release"
# Name "Bellona2 - Win32 Debug"
# Begin Source File

SOURCE=.\3c905b.c
# End Source File
# Begin Source File

SOURCE=.\bellona2.c
# End Source File
# Begin Source File

SOURCE=.\cdrom.c
# End Source File
# Begin Source File

SOURCE=.\chardev.c
# End Source File
# Begin Source File

SOURCE=.\Codetbl.c
# End Source File
# Begin Source File

SOURCE=.\cs.c
# End Source File
# Begin Source File

SOURCE=.\cursor.c
# End Source File
# Begin Source File

SOURCE=.\dbgreg.c
# End Source File
# Begin Source File

SOURCE=.\event.c
# End Source File
# Begin Source File

SOURCE=.\export.c
# End Source File
# Begin Source File

SOURCE=.\fdd.c
# End Source File
# Begin Source File

SOURCE=.\ffmt.c
# End Source File
# Begin Source File

SOURCE=.\fork.c
# End Source File
# Begin Source File

SOURCE=.\Gdt.c
# End Source File
# Begin Source File

SOURCE=.\gdt.h
# End Source File
# Begin Source File

SOURCE=.\hdd.c
# End Source File
# Begin Source File

SOURCE=.\init.c
# End Source File
# Begin Source File

SOURCE=.\int.c
# End Source File
# Begin Source File

SOURCE=.\kbd.c
# End Source File
# Begin Source File

SOURCE=.\kbddrv.c
# End Source File
# Begin Source File

SOURCE=.\kdebug.c
# End Source File
# Begin Source File

SOURCE=.\kmesg.c
# End Source File
# Begin Source File

SOURCE=.\kprocess.c
# End Source File
# Begin Source File

SOURCE=.\kshell.c
# End Source File
# Begin Source File

SOURCE=.\kshlcmd.c
# End Source File
# Begin Source File

SOURCE=.\ksignal.c
# End Source File
# Begin Source File

SOURCE=.\ksyscall.c
# End Source File
# Begin Source File

SOURCE=.\Ldr.c
# End Source File
# Begin Source File

SOURCE=.\Lucifer.c
# End Source File
# Begin Source File

SOURCE=.\Malloc.c
# End Source File
# Begin Source File

SOURCE=.\memory.c
# End Source File
# Begin Source File

SOURCE=.\module.c
# End Source File
# Begin Source File

SOURCE=.\mouse.c
# End Source File
# Begin Source File

SOURCE=.\Myasm.c
# End Source File
# Begin Source File

SOURCE=.\mydbg2.c
# End Source File
# Begin Source File

SOURCE=.\nic.c
# End Source File
# Begin Source File

SOURCE=.\paging.c
# End Source File
# Begin Source File

SOURCE=.\pci.c
# End Source File
# Begin Source File

SOURCE=.\Pefile.h
# End Source File
# Begin Source File

SOURCE=.\rsh_serv.c
# End Source File
# Begin Source File

SOURCE=.\schedule.c
# End Source File
# Begin Source File

SOURCE=.\h\sctype.h
# End Source File
# Begin Source File

SOURCE=.\semaph.c
# End Source File
# Begin Source File

SOURCE=.\serial.c
# End Source File
# Begin Source File

SOURCE=.\Stk.c
# End Source File
# Begin Source File

SOURCE=.\Symtbl.c
# End Source File
# Begin Source File

SOURCE=.\tss.c
# End Source File
# Begin Source File

SOURCE=.\tty.c
# End Source File
# Begin Source File

SOURCE=.\util.c
# End Source File
# Begin Source File

SOURCE=.\v86.c
# End Source File
# Begin Source File

SOURCE=.\vconsole.c
# End Source File
# Begin Source File

SOURCE=.\vesa.c
# End Source File
# Begin Source File

SOURCE=.\lib\bell_fs.lib
# End Source File
# End Target
# End Project
