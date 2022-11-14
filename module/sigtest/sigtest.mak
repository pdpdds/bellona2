# Microsoft Developer Studio Generated NMAKE File, Based on sigtest.dsp
!IF "$(CFG)" == ""
CFG=sigtest - Win32 Debug
!MESSAGE No configuration specified. Defaulting to sigtest - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "sigtest - Win32 Release" && "$(CFG)" != "sigtest - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "sigtest.mak" CFG="sigtest - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "sigtest - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "sigtest - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "sigtest - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\sigtest.exe"


CLEAN :
	-@erase "$(INTDIR)\sigtest.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\sigtest.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\sigtest.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\sigtest.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\sigtest.pdb" /machine:I386 /out:"$(OUTDIR)\sigtest.exe" 
LINK32_OBJS= \
	"$(INTDIR)\sigtest.obj" \
	"..\..\lib\stdlib.lib" \
	"..\..\lib\startup.lib"

"$(OUTDIR)\sigtest.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "sigtest - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\sigtest.exe"


CLEAN :
	-@erase "$(INTDIR)\sigtest.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\sigtest.exe"
	-@erase "$(OUTDIR)\sigtest.map"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /G4 /Zp1 /MLd /W3 /GX /Zi /Od /Gf /X /I "..\..\h" /I "..\..\h\common" /u /FAcs /Fa"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\sigtest.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=/nologo /base:"0x80000000" /subsystem:console /pdb:none /map:"$(INTDIR)\sigtest.map" /machine:I386 /nodefaultlib /out:"$(OUTDIR)\sigtest.exe" 
LINK32_OBJS= \
	"$(INTDIR)\sigtest.obj" \
	"..\..\lib\stdlib.lib" \
	"..\..\lib\startup.lib"

"$(OUTDIR)\sigtest.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\sigtest.exe"
   ..\..\disk\codemap debug\sigtest.map debug\sigtest.exe
	copy debug\sigtest.exe ..\..\disk\sigtest.exe /y
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ENDIF 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("sigtest.dep")
!INCLUDE "sigtest.dep"
!ELSE 
!MESSAGE Warning: cannot find "sigtest.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "sigtest - Win32 Release" || "$(CFG)" == "sigtest - Win32 Debug"
SOURCE=.\sigtest.c

"$(INTDIR)\sigtest.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 

