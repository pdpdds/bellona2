# Microsoft Developer Studio Generated NMAKE File, Based on tt.dsp
!IF "$(CFG)" == ""
CFG=tt - Win32 Debug
!MESSAGE No configuration specified. Defaulting to tt - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "tt - Win32 Release" && "$(CFG)" != "tt - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "tt.mak" CFG="tt - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "tt - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "tt - Win32 Debug" (based on "Win32 (x86) Console Application")
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

!IF  "$(CFG)" == "tt - Win32 Release"

OUTDIR=.\BRelease
INTDIR=.\BRelease
# Begin Custom Macros
OutDir=.\BRelease
# End Custom Macros

ALL : "$(OUTDIR)\tt.exe"


CLEAN :
	-@erase "$(INTDIR)\tetris.obj"
	-@erase "$(INTDIR)\tetris_main.obj"
	-@erase "$(INTDIR)\tt_b2os.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\tt.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\tt.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
RSC_PROJ=/l 0x412 /fo"$(INTDIR)\tt_b2os.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\tt.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\tt.pdb" /machine:I386 /out:"$(OUTDIR)\tt.exe" 
LINK32_OBJS= \
	"$(INTDIR)\tetris.obj" \
	"$(INTDIR)\tetris_main.obj" \
	"..\..\lib\guiapp.lib" \
	"..\..\lib\startup.lib" \
	"..\..\lib\stdlib.lib" \
	"$(INTDIR)\tt_b2os.res"

"$(OUTDIR)\tt.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "tt - Win32 Debug"

OUTDIR=.\BDebug
INTDIR=.\BDebug
# Begin Custom Macros
OutDir=.\BDebug
# End Custom Macros

ALL : "$(OUTDIR)\tt.exe"


CLEAN :
	-@erase "$(INTDIR)\tetris.obj"
	-@erase "$(INTDIR)\tetris_main.obj"
	-@erase "$(INTDIR)\tt_b2os.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\tt.exe"
	-@erase "$(OUTDIR)\tt.map"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /G4 /Zp1 /MLd /W3 /GX /Od /Gf /X /I "\oh\test\h" /I "\oh\test\h\common" /u /FAcs /Fa"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
RSC_PROJ=/l 0x412 /fo"$(INTDIR)\tt_b2os.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\tt.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=/nologo /base:"0x80000000" /subsystem:console /pdb:none /map:"$(INTDIR)\tt.map" /machine:I386 /nodefaultlib /out:"$(OUTDIR)\tt.exe" 
LINK32_OBJS= \
	"$(INTDIR)\tetris.obj" \
	"$(INTDIR)\tetris_main.obj" \
	"..\..\lib\guiapp.lib" \
	"..\..\lib\startup.lib" \
	"..\..\lib\stdlib.lib" \
	"$(INTDIR)\tt_b2os.res"

"$(OUTDIR)\tt.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\BDebug
# End Custom Macros

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\tt.exe"
   copy ..\startup\debug\*.cod bdebug\startup /y
	..\..\disk\codemap bdebug\tt.map bdebug\tt.exe
	copy bdebug\*.exe ..\..\disk
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
!IF EXISTS("tt.dep")
!INCLUDE "tt.dep"
!ELSE 
!MESSAGE Warning: cannot find "tt.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "tt - Win32 Release" || "$(CFG)" == "tt - Win32 Debug"
SOURCE=.\tetris.c

"$(INTDIR)\tetris.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\tetris_main.c

"$(INTDIR)\tetris_main.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\tt_b2os.rc

"$(INTDIR)\tt_b2os.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

