# Microsoft Developer Studio Generated NMAKE File, Based on wintt.dsp
!IF "$(CFG)" == ""
CFG=wintt - Win32 Debug
!MESSAGE No configuration specified. Defaulting to wintt - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "wintt - Win32 Release" && "$(CFG)" != "wintt - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "wintt.mak" CFG="wintt - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "wintt - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "wintt - Win32 Debug" (based on "Win32 (x86) Application")
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

!IF  "$(CFG)" == "wintt - Win32 Release"

OUTDIR=.\WRelease
INTDIR=.\WRelease
# Begin Custom Macros
OutDir=.\WRelease
# End Custom Macros

ALL : "$(OUTDIR)\wintt.exe" "$(OUTDIR)\wintt.pch"


CLEAN :
	-@erase "$(INTDIR)\ChildView.obj"
	-@erase "$(INTDIR)\MainFrm.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\tetris.obj"
	-@erase "$(INTDIR)\tt.obj"
	-@erase "$(INTDIR)\tt.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\wintt.pch"
	-@erase "$(OUTDIR)\wintt.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /Zp1 /MT /W3 /GX /Od /Gf /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Fp"$(INTDIR)\wintt.pch" /YX"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x412 /fo"$(INTDIR)\tt.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\wintt.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=/nologo /subsystem:windows /pdb:none /machine:I386 /out:"$(OUTDIR)\wintt.exe" 
LINK32_OBJS= \
	"$(INTDIR)\ChildView.obj" \
	"$(INTDIR)\MainFrm.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\tetris.obj" \
	"$(INTDIR)\tt.obj" \
	"$(INTDIR)\tt.res"

"$(OUTDIR)\wintt.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "wintt - Win32 Debug"

OUTDIR=.\WDebug
INTDIR=.\WDebug
# Begin Custom Macros
OutDir=.\WDebug
# End Custom Macros

ALL : "$(OUTDIR)\wintt.exe" "$(OUTDIR)\wintt.pch"


CLEAN :
	-@erase "$(INTDIR)\ChildView.obj"
	-@erase "$(INTDIR)\MainFrm.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\tetris.obj"
	-@erase "$(INTDIR)\tt.obj"
	-@erase "$(INTDIR)\tt.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\wintt.pch"
	-@erase "$(OUTDIR)\wintt.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /Zp1 /MTd /W3 /GX /Zi /Od /Gf /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Fp"$(INTDIR)\wintt.pch" /YX"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x412 /fo"$(INTDIR)\tt.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\wintt.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=/nologo /subsystem:windows /pdb:none /debug /machine:I386 /out:"$(OUTDIR)\wintt.exe" 
LINK32_OBJS= \
	"$(INTDIR)\ChildView.obj" \
	"$(INTDIR)\MainFrm.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\tetris.obj" \
	"$(INTDIR)\tt.obj" \
	"$(INTDIR)\tt.res"

"$(OUTDIR)\wintt.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("wintt.dep")
!INCLUDE "wintt.dep"
!ELSE 
!MESSAGE Warning: cannot find "wintt.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "wintt - Win32 Release" || "$(CFG)" == "wintt - Win32 Debug"
SOURCE=.\ChildView.cpp

"$(INTDIR)\ChildView.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\MainFrm.cpp

"$(INTDIR)\MainFrm.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\StdAfx.cpp

!IF  "$(CFG)" == "wintt - Win32 Release"

CPP_SWITCHES=/nologo /Zp1 /MT /W3 /GX /Od /Gf /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Fp"$(INTDIR)\wintt.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\wintt.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "wintt - Win32 Debug"

CPP_SWITCHES=/nologo /Zp1 /MTd /W3 /GX /Zi /Od /Gf /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Fp"$(INTDIR)\wintt.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\wintt.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\tetris.c

"$(INTDIR)\tetris.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\tt.cpp

"$(INTDIR)\tt.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\tt.rc

"$(INTDIR)\tt.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

