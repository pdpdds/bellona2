# Microsoft Developer Studio Generated NMAKE File, Based on FontEdit.dsp
!IF "$(CFG)" == ""
CFG=FontEdit - Win32 Debug
!MESSAGE No configuration specified. Defaulting to FontEdit - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "FontEdit - Win32 Release" && "$(CFG)" != "FontEdit - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "FontEdit.mak" CFG="FontEdit - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "FontEdit - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "FontEdit - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "FontEdit - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\FontEdit.exe"


CLEAN :
	-@erase "$(INTDIR)\CreateNewDlg.obj"
	-@erase "$(INTDIR)\FontEdit.obj"
	-@erase "$(INTDIR)\FontEdit.pch"
	-@erase "$(INTDIR)\FontEdit.res"
	-@erase "$(INTDIR)\FontEditDoc.obj"
	-@erase "$(INTDIR)\FontEditView.obj"
	-@erase "$(INTDIR)\MainFrm.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\FontEdit.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /Zp1 /MT /W3 /GX /Od /Gf /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Fp"$(INTDIR)\FontEdit.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x412 /fo"$(INTDIR)\FontEdit.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\FontEdit.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=/nologo /subsystem:windows /pdb:none /machine:I386 /out:"$(OUTDIR)\FontEdit.exe" 
LINK32_OBJS= \
	"$(INTDIR)\CreateNewDlg.obj" \
	"$(INTDIR)\FontEdit.obj" \
	"$(INTDIR)\FontEditDoc.obj" \
	"$(INTDIR)\FontEditView.obj" \
	"$(INTDIR)\MainFrm.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\FontEdit.res"

"$(OUTDIR)\FontEdit.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "FontEdit - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\FontEdit.exe"


CLEAN :
	-@erase "$(INTDIR)\CreateNewDlg.obj"
	-@erase "$(INTDIR)\FontEdit.obj"
	-@erase "$(INTDIR)\FontEdit.pch"
	-@erase "$(INTDIR)\FontEdit.res"
	-@erase "$(INTDIR)\FontEditDoc.obj"
	-@erase "$(INTDIR)\FontEditView.obj"
	-@erase "$(INTDIR)\MainFrm.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\FontEdit.exe"
	-@erase "$(OUTDIR)\FontEdit.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /Zp1 /MTd /W3 /Gm /GX /Zi /Od /Gf /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Fp"$(INTDIR)\FontEdit.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x412 /fo"$(INTDIR)\FontEdit.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\FontEdit.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=/nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\FontEdit.pdb" /debug /machine:I386 /out:"$(OUTDIR)\FontEdit.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\CreateNewDlg.obj" \
	"$(INTDIR)\FontEdit.obj" \
	"$(INTDIR)\FontEditDoc.obj" \
	"$(INTDIR)\FontEditView.obj" \
	"$(INTDIR)\MainFrm.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\FontEdit.res"

"$(OUTDIR)\FontEdit.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("FontEdit.dep")
!INCLUDE "FontEdit.dep"
!ELSE 
!MESSAGE Warning: cannot find "FontEdit.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "FontEdit - Win32 Release" || "$(CFG)" == "FontEdit - Win32 Debug"
SOURCE=.\CreateNewDlg.cpp

"$(INTDIR)\CreateNewDlg.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\FontEdit.pch"


SOURCE=.\FontEdit.cpp

"$(INTDIR)\FontEdit.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\FontEdit.pch"


SOURCE=.\FontEdit.rc

"$(INTDIR)\FontEdit.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\FontEditDoc.cpp

"$(INTDIR)\FontEditDoc.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\FontEdit.pch"


SOURCE=.\FontEditView.cpp

"$(INTDIR)\FontEditView.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\FontEdit.pch"


SOURCE=.\MainFrm.cpp

"$(INTDIR)\MainFrm.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\FontEdit.pch"


SOURCE=.\StdAfx.cpp

!IF  "$(CFG)" == "FontEdit - Win32 Release"

CPP_SWITCHES=/nologo /Zp1 /MT /W3 /GX /Od /Gf /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Fp"$(INTDIR)\FontEdit.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\FontEdit.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "FontEdit - Win32 Debug"

CPP_SWITCHES=/nologo /Zp1 /MTd /W3 /Gm /GX /Zi /Od /Gf /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Fp"$(INTDIR)\FontEdit.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\FontEdit.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 


!ENDIF 

