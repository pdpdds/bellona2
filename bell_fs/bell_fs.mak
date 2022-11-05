# Microsoft Developer Studio Generated NMAKE File, Based on bell_fs.dsp
!IF "$(CFG)" == ""
CFG=bell_fs - Win32 Debug
!MESSAGE No configuration specified. Defaulting to bell_fs - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "bell_fs - Win32 Release" && "$(CFG)" != "bell_fs - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "bell_fs.mak" CFG="bell_fs - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "bell_fs - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "bell_fs - Win32 Debug" (based on "Win32 (x86) Static Library")
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

!IF  "$(CFG)" == "bell_fs - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\bell_fs.lib"


CLEAN :
	-@erase "$(INTDIR)\bcache.obj"
	-@erase "$(INTDIR)\blkdev.obj"
	-@erase "$(INTDIR)\digit.obj"
	-@erase "$(INTDIR)\f16part.obj"
	-@erase "$(INTDIR)\fat32.obj"
	-@erase "$(INTDIR)\fdd35.obj"
	-@erase "$(INTDIR)\fddfat12.obj"
	-@erase "$(INTDIR)\file.obj"
	-@erase "$(INTDIR)\hddpart.obj"
	-@erase "$(INTDIR)\idehdd.obj"
	-@erase "$(INTDIR)\jcommand.obj"
	-@erase "$(INTDIR)\ramdisk.obj"
	-@erase "$(INTDIR)\stdinout.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vfs.obj"
	-@erase "$(OUTDIR)\bell_fs.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /ML /W3 /GX /O2 /I "..\\" /I "..\h" /D "BELLONA2" /D "BELL_FS" /Fp"$(INTDIR)\bell_fs.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\bell_fs.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\bell_fs.lib" 
LIB32_OBJS= \
	"$(INTDIR)\bcache.obj" \
	"$(INTDIR)\blkdev.obj" \
	"$(INTDIR)\digit.obj" \
	"$(INTDIR)\f16part.obj" \
	"$(INTDIR)\fat32.obj" \
	"$(INTDIR)\fdd35.obj" \
	"$(INTDIR)\fddfat12.obj" \
	"$(INTDIR)\file.obj" \
	"$(INTDIR)\hddpart.obj" \
	"$(INTDIR)\idehdd.obj" \
	"$(INTDIR)\jcommand.obj" \
	"$(INTDIR)\ramdisk.obj" \
	"$(INTDIR)\stdinout.obj" \
	"$(INTDIR)\vfs.obj"

"$(OUTDIR)\bell_fs.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "bell_fs - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\bell_fs.lib"


CLEAN :
	-@erase "$(INTDIR)\bcache.obj"
	-@erase "$(INTDIR)\blkdev.obj"
	-@erase "$(INTDIR)\digit.obj"
	-@erase "$(INTDIR)\f16part.obj"
	-@erase "$(INTDIR)\fat32.obj"
	-@erase "$(INTDIR)\fdd35.obj"
	-@erase "$(INTDIR)\fddfat12.obj"
	-@erase "$(INTDIR)\file.obj"
	-@erase "$(INTDIR)\hddpart.obj"
	-@erase "$(INTDIR)\idehdd.obj"
	-@erase "$(INTDIR)\jcommand.obj"
	-@erase "$(INTDIR)\ramdisk.obj"
	-@erase "$(INTDIR)\stdinout.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\vfs.obj"
	-@erase "$(OUTDIR)\bell_fs.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /G4 /Zp1 /MLd /W3 /WX /Zi /Od /Gf /X /I "..\\" /I "..\h" /u /D "BELLONA2" /D "BELL_FS" /FAcs /Fa"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /Yd /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\bell_fs.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\bell_fs.lib" 
LIB32_OBJS= \
	"$(INTDIR)\bcache.obj" \
	"$(INTDIR)\blkdev.obj" \
	"$(INTDIR)\digit.obj" \
	"$(INTDIR)\f16part.obj" \
	"$(INTDIR)\fat32.obj" \
	"$(INTDIR)\fdd35.obj" \
	"$(INTDIR)\fddfat12.obj" \
	"$(INTDIR)\file.obj" \
	"$(INTDIR)\hddpart.obj" \
	"$(INTDIR)\idehdd.obj" \
	"$(INTDIR)\jcommand.obj" \
	"$(INTDIR)\ramdisk.obj" \
	"$(INTDIR)\stdinout.obj" \
	"$(INTDIR)\vfs.obj"

"$(OUTDIR)\bell_fs.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\bell_fs.lib"
   xcopy debug\*.COD ..\debug\bell_fs /y /D
	copy debug\bell_fs.lib ..\lib /y
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
!IF EXISTS("bell_fs.dep")
!INCLUDE "bell_fs.dep"
!ELSE 
!MESSAGE Warning: cannot find "bell_fs.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "bell_fs - Win32 Release" || "$(CFG)" == "bell_fs - Win32 Debug"
SOURCE=.\bcache.c

"$(INTDIR)\bcache.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\blkdev.c

"$(INTDIR)\blkdev.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\digit.c

"$(INTDIR)\digit.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\f16part.c

"$(INTDIR)\f16part.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\fat32.c

"$(INTDIR)\fat32.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\fdd35.c

"$(INTDIR)\fdd35.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\fddfat12.c

"$(INTDIR)\fddfat12.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\file.c

"$(INTDIR)\file.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\hddpart.c

"$(INTDIR)\hddpart.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\idehdd.c

"$(INTDIR)\idehdd.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\jcommand.c

"$(INTDIR)\jcommand.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ramdisk.c

"$(INTDIR)\ramdisk.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\stdinout.c

"$(INTDIR)\stdinout.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\vfs.c

"$(INTDIR)\vfs.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 

