# Microsoft Developer Studio Generated NMAKE File, Based on stdlib.dsp
!IF "$(CFG)" == ""
CFG=stdlib - Win32 Debug
!MESSAGE No configuration specified. Defaulting to stdlib - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "stdlib - Win32 Release" && "$(CFG)" != "stdlib - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "stdlib.mak" CFG="stdlib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "stdlib - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "stdlib - Win32 Debug" (based on "Win32 (x86) Console Application")
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

!IF  "$(CFG)" == "stdlib - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\stdlib.mod"


CLEAN :
	-@erase "$(INTDIR)\conio.obj"
	-@erase "$(INTDIR)\env.obj"
	-@erase "$(INTDIR)\ffmt.obj"
	-@erase "$(INTDIR)\fileapi.obj"
	-@erase "$(INTDIR)\ipc.obj"
	-@erase "$(INTDIR)\lib.obj"
	-@erase "$(INTDIR)\mem.obj"
	-@erase "$(INTDIR)\modcall.obj"
	-@erase "$(INTDIR)\proc.obj"
	-@erase "$(INTDIR)\signal.obj"
	-@erase "$(INTDIR)\stdio.obj"
	-@erase "$(INTDIR)\stdlib.obj"
	-@erase "$(INTDIR)\stdlib_main.obj"
	-@erase "$(INTDIR)\syscall.obj"
	-@erase "$(INTDIR)\uarea.obj"
	-@erase "$(INTDIR)\util.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\stdlib.mod"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /G4 /Zp1 /ML /W3 /Od /Gf /X /I "\oh\test" /I "\oh\test\h" /I "\oh\test\h\common" /u /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\stdlib.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=/nologo /entry:"stdlib_main" /subsystem:console /pdb:none /machine:I386 /nodefaultlib /out:"$(OUTDIR)\stdlib.mod" /FIXED:NO 
LINK32_OBJS= \
	"$(INTDIR)\conio.obj" \
	"$(INTDIR)\env.obj" \
	"$(INTDIR)\ffmt.obj" \
	"$(INTDIR)\fileapi.obj" \
	"$(INTDIR)\ipc.obj" \
	"$(INTDIR)\lib.obj" \
	"$(INTDIR)\mem.obj" \
	"$(INTDIR)\modcall.obj" \
	"$(INTDIR)\proc.obj" \
	"$(INTDIR)\signal.obj" \
	"$(INTDIR)\stdio.obj" \
	"$(INTDIR)\stdlib.obj" \
	"$(INTDIR)\stdlib_main.obj" \
	"$(INTDIR)\syscall.obj" \
	"$(INTDIR)\uarea.obj" \
	"$(INTDIR)\util.obj" \
	"..\..\lib\bellona2.lib"

"$(OUTDIR)\stdlib.mod" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "stdlib - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\stdlib.mod"


CLEAN :
	-@erase "$(INTDIR)\conio.obj"
	-@erase "$(INTDIR)\env.obj"
	-@erase "$(INTDIR)\ffmt.obj"
	-@erase "$(INTDIR)\fileapi.obj"
	-@erase "$(INTDIR)\ipc.obj"
	-@erase "$(INTDIR)\lib.obj"
	-@erase "$(INTDIR)\mem.obj"
	-@erase "$(INTDIR)\modcall.obj"
	-@erase "$(INTDIR)\proc.obj"
	-@erase "$(INTDIR)\signal.obj"
	-@erase "$(INTDIR)\stdio.obj"
	-@erase "$(INTDIR)\stdlib.obj"
	-@erase "$(INTDIR)\stdlib_main.obj"
	-@erase "$(INTDIR)\syscall.obj"
	-@erase "$(INTDIR)\uarea.obj"
	-@erase "$(INTDIR)\util.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\stdlib.map"
	-@erase "$(OUTDIR)\stdlib.mod"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /G4 /Zp1 /MLd /W3 /Zi /Od /Gf /X /I "\oh\test" /I "\oh\test\h" /I "\oh\test\h\common" /u /FAcs /Fa"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\stdlib.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=/nologo /entry:"stdlib_main" /subsystem:console /pdb:none /map:"$(INTDIR)\stdlib.map" /machine:I386 /nodefaultlib /out:"$(OUTDIR)\stdlib.mod" /FIXED:NO 
LINK32_OBJS= \
	"$(INTDIR)\conio.obj" \
	"$(INTDIR)\env.obj" \
	"$(INTDIR)\ffmt.obj" \
	"$(INTDIR)\fileapi.obj" \
	"$(INTDIR)\ipc.obj" \
	"$(INTDIR)\lib.obj" \
	"$(INTDIR)\mem.obj" \
	"$(INTDIR)\modcall.obj" \
	"$(INTDIR)\proc.obj" \
	"$(INTDIR)\signal.obj" \
	"$(INTDIR)\stdio.obj" \
	"$(INTDIR)\stdlib.obj" \
	"$(INTDIR)\stdlib_main.obj" \
	"$(INTDIR)\syscall.obj" \
	"$(INTDIR)\uarea.obj" \
	"$(INTDIR)\util.obj" \
	"..\..\lib\bellona2.lib"

"$(OUTDIR)\stdlib.mod" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\stdlib.mod"
   \oh\test\disk\codemap debug\stdlib.map debug\stdlib.mod
	copy debug\stdlib.mod \oh\test\disk
	copy debug\stdlib.lib \oh\test\lib
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
!IF EXISTS("stdlib.dep")
!INCLUDE "stdlib.dep"
!ELSE 
!MESSAGE Warning: cannot find "stdlib.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "stdlib - Win32 Release" || "$(CFG)" == "stdlib - Win32 Debug"
SOURCE=.\conio.c

"$(INTDIR)\conio.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\env.c

"$(INTDIR)\env.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=..\..\ffmt.c

"$(INTDIR)\ffmt.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\fileapi.c

"$(INTDIR)\fileapi.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ipc.c

"$(INTDIR)\ipc.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\lib.c

"$(INTDIR)\lib.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mem.c

"$(INTDIR)\mem.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\modcall.c

"$(INTDIR)\modcall.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\proc.c

"$(INTDIR)\proc.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\signal.c

"$(INTDIR)\signal.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\stdio.c

"$(INTDIR)\stdio.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\stdlib.c

"$(INTDIR)\stdlib.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\stdlib_main.c

"$(INTDIR)\stdlib_main.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\syscall.c

"$(INTDIR)\syscall.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\uarea.c

"$(INTDIR)\uarea.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=..\..\util.c

"$(INTDIR)\util.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 

