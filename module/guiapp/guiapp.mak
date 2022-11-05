# Microsoft Developer Studio Generated NMAKE File, Based on guiapp.dsp
!IF "$(CFG)" == ""
CFG=guiapp - Win32 Debug
!MESSAGE No configuration specified. Defaulting to guiapp - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "guiapp - Win32 Release" && "$(CFG)" != "guiapp - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "guiapp.mak" CFG="guiapp - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "guiapp - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "guiapp - Win32 Debug" (based on "Win32 (x86) Console Application")
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

!IF  "$(CFG)" == "guiapp - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\guiapp.exe"


CLEAN :
	-@erase "$(INTDIR)\guiapp.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\guiapp.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\guiapp.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\guiapp.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\guiapp.pdb" /machine:I386 /out:"$(OUTDIR)\guiapp.exe" 
LINK32_OBJS= \
	"$(INTDIR)\guiapp.obj" \
	"..\..\lib\stdlib.lib"

"$(OUTDIR)\guiapp.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "guiapp - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\guiapp.mod"


CLEAN :
	-@erase "$(INTDIR)\guiapp.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\guiapp.map"
	-@erase "$(OUTDIR)\guiapp.mod"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /G4 /Zp1 /MLd /W3 /Zi /Od /Gf /X /I "\oh\test" /I "\oh\test\module\gui" /I "\oh\test\h" /I "\oh\test\h\common" /u /FAcs /Fa"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\guiapp.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=/nologo /entry:"guiapp_main" /subsystem:console /pdb:none /map:"$(INTDIR)\guiapp.map" /machine:I386 /nodefaultlib /out:"$(OUTDIR)\guiapp.mod" /FIXED:NO 
LINK32_OBJS= \
	"$(INTDIR)\guiapp.obj" \
	"..\..\lib\stdlib.lib"

"$(OUTDIR)\guiapp.mod" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\guiapp.mod"
   \oh\test\disk\codemap debug\guiapp.map debug\guiapp.mod
	copy debug\guiapp.mod \oh\test\disk
	copy debug\guiapp.lib \oh\test\lib
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
!IF EXISTS("guiapp.dep")
!INCLUDE "guiapp.dep"
!ELSE 
!MESSAGE Warning: cannot find "guiapp.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "guiapp - Win32 Release" || "$(CFG)" == "guiapp - Win32 Debug"
SOURCE=.\guiapp.c

"$(INTDIR)\guiapp.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 

