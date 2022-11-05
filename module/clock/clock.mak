# Microsoft Developer Studio Generated NMAKE File, Based on clock.dsp
!IF "$(CFG)" == ""
CFG=clock - Win32 Debug
!MESSAGE No configuration specified. Defaulting to clock - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "clock - Win32 Release" && "$(CFG)" != "clock - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "clock.mak" CFG="clock - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "clock - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "clock - Win32 Debug" (based on "Win32 (x86) Console Application")
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

!IF  "$(CFG)" == "clock - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\clock.exe"


CLEAN :
	-@erase "$(INTDIR)\clkres.res"
	-@erase "$(INTDIR)\clock_main.obj"
	-@erase "$(INTDIR)\tri_tbl.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\clock.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\clock.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
RSC_PROJ=/l 0x412 /fo"$(INTDIR)\clkres.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\clock.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\clock.pdb" /machine:I386 /out:"$(OUTDIR)\clock.exe" 
LINK32_OBJS= \
	"$(INTDIR)\clock_main.obj" \
	"$(INTDIR)\tri_tbl.obj" \
	"$(INTDIR)\clkres.res" \
	"..\..\lib\stdlib.lib" \
	"..\..\lib\guiapp.lib" \
	"..\..\lib\startup.lib"

"$(OUTDIR)\clock.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "clock - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\clock.exe"


CLEAN :
	-@erase "$(INTDIR)\clkres.res"
	-@erase "$(INTDIR)\clock_main.obj"
	-@erase "$(INTDIR)\tri_tbl.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\clock.exe"
	-@erase "$(OUTDIR)\clock.map"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /G4 /Zp1 /MLd /W3 /GX /Od /Gf /X /I "..\..\h" /I "..\..\h\common" /u /FAcs /Fa"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
RSC_PROJ=/l 0x412 /fo"$(INTDIR)\clkres.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\clock.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=/nologo /base:"0x80000000" /subsystem:console /pdb:none /map:"$(INTDIR)\clock.map" /machine:I386 /nodefaultlib /out:"$(OUTDIR)\clock.exe" /FIXED:NO 
LINK32_OBJS= \
	"$(INTDIR)\clock_main.obj" \
	"$(INTDIR)\tri_tbl.obj" \
	"$(INTDIR)\clkres.res" \
	"..\..\lib\stdlib.lib" \
	"..\..\lib\guiapp.lib" \
	"..\..\lib\startup.lib"

"$(OUTDIR)\clock.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\clock.exe"
   copy ..\startup\debug\*.cod debug\startup /y
	..\..\disk\codemap debug\clock.map debug\clock.exe
	copy debug\*.exe ..\..\disk
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
!IF EXISTS("clock.dep")
!INCLUDE "clock.dep"
!ELSE 
!MESSAGE Warning: cannot find "clock.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "clock - Win32 Release" || "$(CFG)" == "clock - Win32 Debug"
SOURCE=.\clkres.rc

"$(INTDIR)\clkres.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\clock_main.c

"$(INTDIR)\clock_main.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\tri_tbl.c

"$(INTDIR)\tri_tbl.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 

