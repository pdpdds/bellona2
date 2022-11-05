# Microsoft Developer Studio Generated NMAKE File, Based on ush.dsp
!IF "$(CFG)" == ""
CFG=ush - Win32 Debug
!MESSAGE No configuration specified. Defaulting to ush - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "ush - Win32 Release" && "$(CFG)" != "ush - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ush.mak" CFG="ush - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ush - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "ush - Win32 Debug" (based on "Win32 (x86) Console Application")
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

!IF  "$(CFG)" == "ush - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\ush.exe"


CLEAN :
	-@erase "$(INTDIR)\ush.obj"
	-@erase "$(INTDIR)\ush_b2.obj"
	-@erase "$(INTDIR)\ush_cmd.obj"
	-@erase "$(INTDIR)\ush_main.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\ush.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /G4 /Zp1 /ML /W3 /Od /X /I "\oh\test\h" /I "\oh\test\h\common" /u /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\ush.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=stdlib-r.lib /nologo /base:"0x80000000" /subsystem:console /pdb:none /machine:I386 /nodefaultlib /out:"$(OUTDIR)\ush.exe" /libpath:"\oh\os\bellona2\apps\lib" /FIXED:NO 
LINK32_OBJS= \
	"$(INTDIR)\ush.obj" \
	"$(INTDIR)\ush_b2.obj" \
	"$(INTDIR)\ush_cmd.obj" \
	"$(INTDIR)\ush_main.obj" \
	"..\..\lib\stdlib.lib" \
	"..\..\lib\startup.lib"

"$(OUTDIR)\ush.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\ush.exe"
   copy release\*.exe \oh\os\bellona2\disk\apps\release\*.exe
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ELSEIF  "$(CFG)" == "ush - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\ush.exe"


CLEAN :
	-@erase "$(INTDIR)\ush.obj"
	-@erase "$(INTDIR)\ush_b2.obj"
	-@erase "$(INTDIR)\ush_cmd.obj"
	-@erase "$(INTDIR)\ush_main.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\ush.exe"
	-@erase "$(OUTDIR)\ush.map"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /G4 /Zp1 /MLd /W3 /Zi /Od /Gf /X /I "\oh\test\h" /I "\oh\test\h\common" /u /FAcs /Fa"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\ush.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=/nologo /base:"0x80000000" /subsystem:console /pdb:none /map:"$(INTDIR)\ush.map" /machine:I386 /nodefaultlib /out:"$(OUTDIR)\ush.exe" /FIXED:NO 
LINK32_OBJS= \
	"$(INTDIR)\ush.obj" \
	"$(INTDIR)\ush_b2.obj" \
	"$(INTDIR)\ush_cmd.obj" \
	"$(INTDIR)\ush_main.obj" \
	"..\..\lib\stdlib.lib" \
	"..\..\lib\startup.lib"

"$(OUTDIR)\ush.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\ush.exe"
   mkdir debug\startup
	copy ..\startup\debug\*.cod debug\startup /y
	..\..\disk\codemap debug\ush.map debug\ush.exe
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
!IF EXISTS("ush.dep")
!INCLUDE "ush.dep"
!ELSE 
!MESSAGE Warning: cannot find "ush.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "ush - Win32 Release" || "$(CFG)" == "ush - Win32 Debug"
SOURCE=.\ush.c

"$(INTDIR)\ush.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ush_b2.c

"$(INTDIR)\ush_b2.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ush_cmd.c

"$(INTDIR)\ush_cmd.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ush_main.c

"$(INTDIR)\ush_main.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 

