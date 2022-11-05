# Microsoft Developer Studio Generated NMAKE File, Based on wush.dsp
!IF "$(CFG)" == ""
CFG=wush - Win32 Debug
!MESSAGE No configuration specified. Defaulting to wush - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "wush - Win32 Release" && "$(CFG)" != "wush - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "wush.mak" CFG="wush - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "wush - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "wush - Win32 Debug" (based on "Win32 (x86) Console Application")
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

!IF  "$(CFG)" == "wush - Win32 Release"

OUTDIR=.\wRelease
INTDIR=.\wRelease
# Begin Custom Macros
OutDir=.\wRelease
# End Custom Macros

ALL : "$(OUTDIR)\wush.exe"


CLEAN :
	-@erase "$(INTDIR)\env.obj"
	-@erase "$(INTDIR)\ush.obj"
	-@erase "$(INTDIR)\ush_cmd.obj"
	-@erase "$(INTDIR)\ush_main.obj"
	-@erase "$(INTDIR)\ush_w32.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\wush.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /Zp1 /ML /W3 /GX /Od /Gf /I "\oh\test\h\common" /I "\oh\test\h\w32" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\wush.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\wush.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /pdb:none /machine:I386 /out:"$(OUTDIR)\wush.exe" 
LINK32_OBJS= \
	"$(INTDIR)\env.obj" \
	"$(INTDIR)\ush.obj" \
	"$(INTDIR)\ush_main.obj" \
	"$(INTDIR)\ush_w32.obj" \
	"$(INTDIR)\ush_cmd.obj"

"$(OUTDIR)\wush.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "wush - Win32 Debug"

OUTDIR=.\wDebug
INTDIR=.\wDebug
# Begin Custom Macros
OutDir=.\wDebug
# End Custom Macros

ALL : "$(OUTDIR)\wush.exe"


CLEAN :
	-@erase "$(INTDIR)\env.obj"
	-@erase "$(INTDIR)\ush.obj"
	-@erase "$(INTDIR)\ush_cmd.obj"
	-@erase "$(INTDIR)\ush_main.obj"
	-@erase "$(INTDIR)\ush_w32.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\wush.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /G4 /Zp1 /MLd /W3 /GX /Zi /Od /Gf /I "\oh\test\h\common" /I "\oh\test\h\w32" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\wush.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\wush.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /pdb:none /debug /machine:I386 /out:"$(OUTDIR)\wush.exe" 
LINK32_OBJS= \
	"$(INTDIR)\env.obj" \
	"$(INTDIR)\ush.obj" \
	"$(INTDIR)\ush_main.obj" \
	"$(INTDIR)\ush_w32.obj" \
	"$(INTDIR)\ush_cmd.obj"

"$(OUTDIR)\wush.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

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
!IF EXISTS("wush.dep")
!INCLUDE "wush.dep"
!ELSE 
!MESSAGE Warning: cannot find "wush.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "wush - Win32 Release" || "$(CFG)" == "wush - Win32 Debug"
SOURCE=..\stdlib\env.c

"$(INTDIR)\env.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ush.c

"$(INTDIR)\ush.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ush_cmd.c

"$(INTDIR)\ush_cmd.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ush_main.c

"$(INTDIR)\ush_main.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ush_w32.c

"$(INTDIR)\ush_w32.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 

