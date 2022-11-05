# Microsoft Developer Studio Generated NMAKE File, Based on GUI.dsp
!IF "$(CFG)" == ""
CFG=GUI - Win32 Debug
!MESSAGE No configuration specified. Defaulting to GUI - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "GUI - Win32 Release" && "$(CFG)" != "GUI - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
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
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "GUI - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\GUI.exe"


CLEAN :
	-@erase "$(INTDIR)\about.obj"
	-@erase "$(INTDIR)\bitmap.obj"
	-@erase "$(INTDIR)\button.obj"
	-@erase "$(INTDIR)\draw.obj"
	-@erase "$(INTDIR)\flatw.obj"
	-@erase "$(INTDIR)\font.obj"
	-@erase "$(INTDIR)\framew.obj"
	-@erase "$(INTDIR)\grxcall.obj"
	-@erase "$(INTDIR)\GUI.obj"
	-@erase "$(INTDIR)\gui.res"
	-@erase "$(INTDIR)\guiexp.obj"
	-@erase "$(INTDIR)\iconwin.obj"
	-@erase "$(INTDIR)\kcmdwin.obj"
	-@erase "$(INTDIR)\mbox.obj"
	-@erase "$(INTDIR)\menu.obj"
	-@erase "$(INTDIR)\mpointer.obj"
	-@erase "$(INTDIR)\simplew.obj"
	-@erase "$(INTDIR)\syscolor.obj"
	-@erase "$(INTDIR)\taskbar.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\wall.obj"
	-@erase "$(INTDIR)\win.obj"
	-@erase "$(INTDIR)\WinRes.obj"
	-@erase "$(OUTDIR)\GUI.exe"
	-@erase "$(OUTDIR)\GUI.map"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /G4 /Zp1 /ML /W3 /Od /Gf /X /I "\oh\test" /I "\oh\test\h" /I "\oh\test\h\common" /u /FAcs /Fa"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
RSC_PROJ=/l 0x412 /fo"$(INTDIR)\gui.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\GUI.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=/nologo /entry:"gui_main" /subsystem:console /incremental:no /pdb:"$(OUTDIR)\GUI.pdb" /map:"$(INTDIR)\GUI.map" /machine:I386 /nodefaultlib /out:"$(OUTDIR)\GUI.exe" 
LINK32_OBJS= \
	"$(INTDIR)\about.obj" \
	"$(INTDIR)\bitmap.obj" \
	"$(INTDIR)\button.obj" \
	"$(INTDIR)\draw.obj" \
	"$(INTDIR)\flatw.obj" \
	"$(INTDIR)\font.obj" \
	"$(INTDIR)\framew.obj" \
	"$(INTDIR)\grxcall.obj" \
	"$(INTDIR)\GUI.obj" \
	"$(INTDIR)\guiexp.obj" \
	"$(INTDIR)\iconwin.obj" \
	"$(INTDIR)\kcmdwin.obj" \
	"$(INTDIR)\mbox.obj" \
	"$(INTDIR)\menu.obj" \
	"$(INTDIR)\mpointer.obj" \
	"$(INTDIR)\simplew.obj" \
	"$(INTDIR)\syscolor.obj" \
	"$(INTDIR)\taskbar.obj" \
	"$(INTDIR)\wall.obj" \
	"$(INTDIR)\win.obj" \
	"$(INTDIR)\WinRes.obj" \
	"$(INTDIR)\gui.res" \
	"..\..\lib\bellona2.lib"

"$(OUTDIR)\GUI.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "GUI - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\GUI.MOD" "$(OUTDIR)\GUI.bsc"


CLEAN :
	-@erase "$(INTDIR)\about.obj"
	-@erase "$(INTDIR)\about.sbr"
	-@erase "$(INTDIR)\bitmap.obj"
	-@erase "$(INTDIR)\bitmap.sbr"
	-@erase "$(INTDIR)\button.obj"
	-@erase "$(INTDIR)\button.sbr"
	-@erase "$(INTDIR)\draw.obj"
	-@erase "$(INTDIR)\draw.sbr"
	-@erase "$(INTDIR)\flatw.obj"
	-@erase "$(INTDIR)\flatw.sbr"
	-@erase "$(INTDIR)\font.obj"
	-@erase "$(INTDIR)\font.sbr"
	-@erase "$(INTDIR)\framew.obj"
	-@erase "$(INTDIR)\framew.sbr"
	-@erase "$(INTDIR)\grxcall.obj"
	-@erase "$(INTDIR)\grxcall.sbr"
	-@erase "$(INTDIR)\GUI.obj"
	-@erase "$(INTDIR)\gui.res"
	-@erase "$(INTDIR)\GUI.sbr"
	-@erase "$(INTDIR)\guiexp.obj"
	-@erase "$(INTDIR)\guiexp.sbr"
	-@erase "$(INTDIR)\iconwin.obj"
	-@erase "$(INTDIR)\iconwin.sbr"
	-@erase "$(INTDIR)\kcmdwin.obj"
	-@erase "$(INTDIR)\kcmdwin.sbr"
	-@erase "$(INTDIR)\mbox.obj"
	-@erase "$(INTDIR)\mbox.sbr"
	-@erase "$(INTDIR)\menu.obj"
	-@erase "$(INTDIR)\menu.sbr"
	-@erase "$(INTDIR)\mpointer.obj"
	-@erase "$(INTDIR)\mpointer.sbr"
	-@erase "$(INTDIR)\simplew.obj"
	-@erase "$(INTDIR)\simplew.sbr"
	-@erase "$(INTDIR)\syscolor.obj"
	-@erase "$(INTDIR)\syscolor.sbr"
	-@erase "$(INTDIR)\taskbar.obj"
	-@erase "$(INTDIR)\taskbar.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\wall.obj"
	-@erase "$(INTDIR)\wall.sbr"
	-@erase "$(INTDIR)\win.obj"
	-@erase "$(INTDIR)\win.sbr"
	-@erase "$(INTDIR)\WinRes.obj"
	-@erase "$(INTDIR)\WinRes.sbr"
	-@erase "$(OUTDIR)\GUI.bsc"
	-@erase "$(OUTDIR)\GUI.map"
	-@erase "$(OUTDIR)\GUI.MOD"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /G4 /Zp1 /MLd /W3 /Od /Gf /X /I "\oh\test" /I "\oh\test\h" /I "\oh\test\h\common" /u /FAcs /Fa"$(INTDIR)\\" /Fr"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\gui.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\GUI.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\about.sbr" \
	"$(INTDIR)\bitmap.sbr" \
	"$(INTDIR)\button.sbr" \
	"$(INTDIR)\draw.sbr" \
	"$(INTDIR)\flatw.sbr" \
	"$(INTDIR)\font.sbr" \
	"$(INTDIR)\framew.sbr" \
	"$(INTDIR)\grxcall.sbr" \
	"$(INTDIR)\GUI.sbr" \
	"$(INTDIR)\guiexp.sbr" \
	"$(INTDIR)\iconwin.sbr" \
	"$(INTDIR)\kcmdwin.sbr" \
	"$(INTDIR)\mbox.sbr" \
	"$(INTDIR)\menu.sbr" \
	"$(INTDIR)\mpointer.sbr" \
	"$(INTDIR)\simplew.sbr" \
	"$(INTDIR)\syscolor.sbr" \
	"$(INTDIR)\taskbar.sbr" \
	"$(INTDIR)\wall.sbr" \
	"$(INTDIR)\win.sbr" \
	"$(INTDIR)\WinRes.sbr"

"$(OUTDIR)\GUI.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=/nologo /entry:"gui_main" /subsystem:console /pdb:none /map:"$(INTDIR)\GUI.map" /machine:I386 /nodefaultlib /out:"$(OUTDIR)\GUI.MOD" /fixed:no 
LINK32_OBJS= \
	"$(INTDIR)\about.obj" \
	"$(INTDIR)\bitmap.obj" \
	"$(INTDIR)\button.obj" \
	"$(INTDIR)\draw.obj" \
	"$(INTDIR)\flatw.obj" \
	"$(INTDIR)\font.obj" \
	"$(INTDIR)\framew.obj" \
	"$(INTDIR)\grxcall.obj" \
	"$(INTDIR)\GUI.obj" \
	"$(INTDIR)\guiexp.obj" \
	"$(INTDIR)\iconwin.obj" \
	"$(INTDIR)\kcmdwin.obj" \
	"$(INTDIR)\mbox.obj" \
	"$(INTDIR)\menu.obj" \
	"$(INTDIR)\mpointer.obj" \
	"$(INTDIR)\simplew.obj" \
	"$(INTDIR)\syscolor.obj" \
	"$(INTDIR)\taskbar.obj" \
	"$(INTDIR)\wall.obj" \
	"$(INTDIR)\win.obj" \
	"$(INTDIR)\WinRes.obj" \
	"$(INTDIR)\gui.res" \
	"..\..\lib\bellona2.lib"

"$(OUTDIR)\GUI.MOD" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\GUI.MOD" "$(OUTDIR)\GUI.bsc"
   copy debug\gui.mod debug\gui.exe
	\oh\test\disk\codemap debug\gui.map debug\gui.mod
	copy debug\gui.mod \oh\test\disk\gui.mod /y
	copy debug\gui.lib \oh\test\lib\gui.lib /y
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
!IF EXISTS("GUI.dep")
!INCLUDE "GUI.dep"
!ELSE 
!MESSAGE Warning: cannot find "GUI.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "GUI - Win32 Release" || "$(CFG)" == "GUI - Win32 Debug"
SOURCE=.\about.c

!IF  "$(CFG)" == "GUI - Win32 Release"


"$(INTDIR)\about.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "GUI - Win32 Debug"


"$(INTDIR)\about.obj"	"$(INTDIR)\about.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\bitmap.c

!IF  "$(CFG)" == "GUI - Win32 Release"


"$(INTDIR)\bitmap.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "GUI - Win32 Debug"


"$(INTDIR)\bitmap.obj"	"$(INTDIR)\bitmap.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\button.c

!IF  "$(CFG)" == "GUI - Win32 Release"


"$(INTDIR)\button.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "GUI - Win32 Debug"


"$(INTDIR)\button.obj"	"$(INTDIR)\button.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\draw.c

!IF  "$(CFG)" == "GUI - Win32 Release"


"$(INTDIR)\draw.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "GUI - Win32 Debug"


"$(INTDIR)\draw.obj"	"$(INTDIR)\draw.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\flatw.c

!IF  "$(CFG)" == "GUI - Win32 Release"


"$(INTDIR)\flatw.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "GUI - Win32 Debug"


"$(INTDIR)\flatw.obj"	"$(INTDIR)\flatw.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\font.c

!IF  "$(CFG)" == "GUI - Win32 Release"


"$(INTDIR)\font.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "GUI - Win32 Debug"


"$(INTDIR)\font.obj"	"$(INTDIR)\font.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\framew.c

!IF  "$(CFG)" == "GUI - Win32 Release"


"$(INTDIR)\framew.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "GUI - Win32 Debug"


"$(INTDIR)\framew.obj"	"$(INTDIR)\framew.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\grxcall.c

!IF  "$(CFG)" == "GUI - Win32 Release"


"$(INTDIR)\grxcall.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "GUI - Win32 Debug"


"$(INTDIR)\grxcall.obj"	"$(INTDIR)\grxcall.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\GUI.c

!IF  "$(CFG)" == "GUI - Win32 Release"


"$(INTDIR)\GUI.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "GUI - Win32 Debug"


"$(INTDIR)\GUI.obj"	"$(INTDIR)\GUI.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\gui.rc

"$(INTDIR)\gui.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\guiexp.c

!IF  "$(CFG)" == "GUI - Win32 Release"


"$(INTDIR)\guiexp.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "GUI - Win32 Debug"


"$(INTDIR)\guiexp.obj"	"$(INTDIR)\guiexp.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\iconwin.c

!IF  "$(CFG)" == "GUI - Win32 Release"


"$(INTDIR)\iconwin.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "GUI - Win32 Debug"


"$(INTDIR)\iconwin.obj"	"$(INTDIR)\iconwin.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\kcmdwin.c

!IF  "$(CFG)" == "GUI - Win32 Release"


"$(INTDIR)\kcmdwin.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "GUI - Win32 Debug"


"$(INTDIR)\kcmdwin.obj"	"$(INTDIR)\kcmdwin.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\mbox.c

!IF  "$(CFG)" == "GUI - Win32 Release"


"$(INTDIR)\mbox.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "GUI - Win32 Debug"


"$(INTDIR)\mbox.obj"	"$(INTDIR)\mbox.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\menu.c

!IF  "$(CFG)" == "GUI - Win32 Release"


"$(INTDIR)\menu.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "GUI - Win32 Debug"


"$(INTDIR)\menu.obj"	"$(INTDIR)\menu.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\mpointer.c

!IF  "$(CFG)" == "GUI - Win32 Release"


"$(INTDIR)\mpointer.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "GUI - Win32 Debug"


"$(INTDIR)\mpointer.obj"	"$(INTDIR)\mpointer.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\simplew.c

!IF  "$(CFG)" == "GUI - Win32 Release"


"$(INTDIR)\simplew.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "GUI - Win32 Debug"


"$(INTDIR)\simplew.obj"	"$(INTDIR)\simplew.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\syscolor.c

!IF  "$(CFG)" == "GUI - Win32 Release"


"$(INTDIR)\syscolor.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "GUI - Win32 Debug"


"$(INTDIR)\syscolor.obj"	"$(INTDIR)\syscolor.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\taskbar.c

!IF  "$(CFG)" == "GUI - Win32 Release"


"$(INTDIR)\taskbar.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "GUI - Win32 Debug"


"$(INTDIR)\taskbar.obj"	"$(INTDIR)\taskbar.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\wall.c

!IF  "$(CFG)" == "GUI - Win32 Release"


"$(INTDIR)\wall.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "GUI - Win32 Debug"


"$(INTDIR)\wall.obj"	"$(INTDIR)\wall.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\win.c

!IF  "$(CFG)" == "GUI - Win32 Release"


"$(INTDIR)\win.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "GUI - Win32 Debug"


"$(INTDIR)\win.obj"	"$(INTDIR)\win.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\WinRes.c

!IF  "$(CFG)" == "GUI - Win32 Release"


"$(INTDIR)\WinRes.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "GUI - Win32 Debug"


"$(INTDIR)\WinRes.obj"	"$(INTDIR)\WinRes.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 


!ENDIF 

