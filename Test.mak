# Microsoft Developer Studio Generated NMAKE File, Based on Test.dsp
!IF "$(CFG)" == ""
CFG=Test - Win32 Debug
!MESSAGE No configuration specified. Defaulting to Test - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "Test - Win32 Release" && "$(CFG)" != "Test - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Test.mak" CFG="Test - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Test - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "Test - Win32 Debug" (based on "Win32 (x86) Console Application")
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

!IF  "$(CFG)" == "Test - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\Bellona.bin"


CLEAN :
	-@erase "$(INTDIR)\3c905b.obj"
	-@erase "$(INTDIR)\bellona2.obj"
	-@erase "$(INTDIR)\cdrom.obj"
	-@erase "$(INTDIR)\chardev.obj"
	-@erase "$(INTDIR)\Codetbl.obj"
	-@erase "$(INTDIR)\cs.obj"
	-@erase "$(INTDIR)\cursor.obj"
	-@erase "$(INTDIR)\dbgreg.obj"
	-@erase "$(INTDIR)\event.obj"
	-@erase "$(INTDIR)\export.obj"
	-@erase "$(INTDIR)\fdd.obj"
	-@erase "$(INTDIR)\ffmt.obj"
	-@erase "$(INTDIR)\fork.obj"
	-@erase "$(INTDIR)\Gdt.obj"
	-@erase "$(INTDIR)\hdd.obj"
	-@erase "$(INTDIR)\init.obj"
	-@erase "$(INTDIR)\int.obj"
	-@erase "$(INTDIR)\kbd.obj"
	-@erase "$(INTDIR)\kbddrv.obj"
	-@erase "$(INTDIR)\kdebug.obj"
	-@erase "$(INTDIR)\kgrxcall.obj"
	-@erase "$(INTDIR)\kmesg.obj"
	-@erase "$(INTDIR)\kprocess.obj"
	-@erase "$(INTDIR)\kshell.obj"
	-@erase "$(INTDIR)\kshlcmd.obj"
	-@erase "$(INTDIR)\ksignal.obj"
	-@erase "$(INTDIR)\ksyscall.obj"
	-@erase "$(INTDIR)\Ldr.obj"
	-@erase "$(INTDIR)\Lucifer.obj"
	-@erase "$(INTDIR)\Malloc.obj"
	-@erase "$(INTDIR)\memory.obj"
	-@erase "$(INTDIR)\module.obj"
	-@erase "$(INTDIR)\mouse.obj"
	-@erase "$(INTDIR)\Myasm.obj"
	-@erase "$(INTDIR)\mydbg2.obj"
	-@erase "$(INTDIR)\nic.obj"
	-@erase "$(INTDIR)\paging.obj"
	-@erase "$(INTDIR)\pci.obj"
	-@erase "$(INTDIR)\rsh_serv.obj"
	-@erase "$(INTDIR)\schedule.obj"
	-@erase "$(INTDIR)\semaph.obj"
	-@erase "$(INTDIR)\serial.obj"
	-@erase "$(INTDIR)\Stk.obj"
	-@erase "$(INTDIR)\Symtbl.obj"
	-@erase "$(INTDIR)\tss.obj"
	-@erase "$(INTDIR)\tty.obj"
	-@erase "$(INTDIR)\util.obj"
	-@erase "$(INTDIR)\v86.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vconsole.obj"
	-@erase "$(INTDIR)\vesa.obj"
	-@erase "$(OUTDIR)\Bellona.bin"
	-@erase "$(OUTDIR)\Bellona.map"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /G4 /Zp1 /ML /W3 /vmg /vd0 /GX /Od /Gf /I ".\\" /I ".\h" /I ".\h\common" /D "BELLONA" /FAc /Fa"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Test.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=/nologo /base:"0x400000" /entry:"bellona2_main" /subsystem:console /incremental:no /pdb:"$(OUTDIR)\Bellona.pdb" /map:"$(INTDIR)\Bellona.map" /machine:I386 /nodefaultlib /out:"$(OUTDIR)\Bellona.bin" 
LINK32_OBJS= \
	"$(INTDIR)\3c905b.obj" \
	"$(INTDIR)\bellona2.obj" \
	"$(INTDIR)\cdrom.obj" \
	"$(INTDIR)\chardev.obj" \
	"$(INTDIR)\Codetbl.obj" \
	"$(INTDIR)\cs.obj" \
	"$(INTDIR)\cursor.obj" \
	"$(INTDIR)\dbgreg.obj" \
	"$(INTDIR)\event.obj" \
	"$(INTDIR)\export.obj" \
	"$(INTDIR)\fdd.obj" \
	"$(INTDIR)\ffmt.obj" \
	"$(INTDIR)\fork.obj" \
	"$(INTDIR)\Gdt.obj" \
	"$(INTDIR)\hdd.obj" \
	"$(INTDIR)\init.obj" \
	"$(INTDIR)\int.obj" \
	"$(INTDIR)\kbd.obj" \
	"$(INTDIR)\kbddrv.obj" \
	"$(INTDIR)\kdebug.obj" \
	"$(INTDIR)\kgrxcall.obj" \
	"$(INTDIR)\kmesg.obj" \
	"$(INTDIR)\kprocess.obj" \
	"$(INTDIR)\kshell.obj" \
	"$(INTDIR)\kshlcmd.obj" \
	"$(INTDIR)\ksignal.obj" \
	"$(INTDIR)\ksyscall.obj" \
	"$(INTDIR)\Ldr.obj" \
	"$(INTDIR)\Lucifer.obj" \
	"$(INTDIR)\Malloc.obj" \
	"$(INTDIR)\memory.obj" \
	"$(INTDIR)\module.obj" \
	"$(INTDIR)\mouse.obj" \
	"$(INTDIR)\Myasm.obj" \
	"$(INTDIR)\mydbg2.obj" \
	"$(INTDIR)\nic.obj" \
	"$(INTDIR)\paging.obj" \
	"$(INTDIR)\pci.obj" \
	"$(INTDIR)\rsh_serv.obj" \
	"$(INTDIR)\schedule.obj" \
	"$(INTDIR)\semaph.obj" \
	"$(INTDIR)\serial.obj" \
	"$(INTDIR)\Stk.obj" \
	"$(INTDIR)\Symtbl.obj" \
	"$(INTDIR)\tss.obj" \
	"$(INTDIR)\tty.obj" \
	"$(INTDIR)\util.obj" \
	"$(INTDIR)\v86.obj" \
	"$(INTDIR)\vconsole.obj" \
	"$(INTDIR)\vesa.obj" \
	".\lib\bell_fs.lib" \
	".\lib\asm.lib"

"$(OUTDIR)\Bellona.bin" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Test - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\bellona2.mod"


CLEAN :
	-@erase "$(INTDIR)\3c905b.obj"
	-@erase "$(INTDIR)\bellona2.obj"
	-@erase "$(INTDIR)\cdrom.obj"
	-@erase "$(INTDIR)\chardev.obj"
	-@erase "$(INTDIR)\Codetbl.obj"
	-@erase "$(INTDIR)\cs.obj"
	-@erase "$(INTDIR)\cursor.obj"
	-@erase "$(INTDIR)\dbgreg.obj"
	-@erase "$(INTDIR)\event.obj"
	-@erase "$(INTDIR)\export.obj"
	-@erase "$(INTDIR)\fdd.obj"
	-@erase "$(INTDIR)\ffmt.obj"
	-@erase "$(INTDIR)\fork.obj"
	-@erase "$(INTDIR)\Gdt.obj"
	-@erase "$(INTDIR)\hdd.obj"
	-@erase "$(INTDIR)\init.obj"
	-@erase "$(INTDIR)\int.obj"
	-@erase "$(INTDIR)\kbd.obj"
	-@erase "$(INTDIR)\kbddrv.obj"
	-@erase "$(INTDIR)\kdebug.obj"
	-@erase "$(INTDIR)\kgrxcall.obj"
	-@erase "$(INTDIR)\kmesg.obj"
	-@erase "$(INTDIR)\kprocess.obj"
	-@erase "$(INTDIR)\kshell.obj"
	-@erase "$(INTDIR)\kshlcmd.obj"
	-@erase "$(INTDIR)\ksignal.obj"
	-@erase "$(INTDIR)\ksyscall.obj"
	-@erase "$(INTDIR)\Ldr.obj"
	-@erase "$(INTDIR)\Lucifer.obj"
	-@erase "$(INTDIR)\Malloc.obj"
	-@erase "$(INTDIR)\memory.obj"
	-@erase "$(INTDIR)\module.obj"
	-@erase "$(INTDIR)\mouse.obj"
	-@erase "$(INTDIR)\Myasm.obj"
	-@erase "$(INTDIR)\mydbg2.obj"
	-@erase "$(INTDIR)\nic.obj"
	-@erase "$(INTDIR)\paging.obj"
	-@erase "$(INTDIR)\pci.obj"
	-@erase "$(INTDIR)\rsh_serv.obj"
	-@erase "$(INTDIR)\schedule.obj"
	-@erase "$(INTDIR)\semaph.obj"
	-@erase "$(INTDIR)\serial.obj"
	-@erase "$(INTDIR)\Stk.obj"
	-@erase "$(INTDIR)\Symtbl.obj"
	-@erase "$(INTDIR)\tss.obj"
	-@erase "$(INTDIR)\tty.obj"
	-@erase "$(INTDIR)\util.obj"
	-@erase "$(INTDIR)\v86.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vconsole.obj"
	-@erase "$(INTDIR)\vesa.obj"
	-@erase "$(OUTDIR)\bellona2.map"
	-@erase "$(OUTDIR)\bellona2.mod"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /G4 /Zp1 /MLd /W3 /vd0 /Od /Gf /X /I ".\\" /I ".\h" /I ".\h\common" /u /D "BELLONA2" /FAcs /Fa"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /Gs9999999 /FD /Yd /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Test.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=/nologo /base:"0x400000" /entry:"bellona2_main" /subsystem:console /pdb:none /map:"$(INTDIR)\bellona2.map" /machine:I386 /nodefaultlib /out:"$(OUTDIR)\bellona2.mod" /FIXED 
LINK32_OBJS= \
	"$(INTDIR)\3c905b.obj" \
	"$(INTDIR)\bellona2.obj" \
	"$(INTDIR)\cdrom.obj" \
	"$(INTDIR)\chardev.obj" \
	"$(INTDIR)\Codetbl.obj" \
	"$(INTDIR)\cs.obj" \
	"$(INTDIR)\cursor.obj" \
	"$(INTDIR)\dbgreg.obj" \
	"$(INTDIR)\event.obj" \
	"$(INTDIR)\export.obj" \
	"$(INTDIR)\fdd.obj" \
	"$(INTDIR)\ffmt.obj" \
	"$(INTDIR)\fork.obj" \
	"$(INTDIR)\Gdt.obj" \
	"$(INTDIR)\hdd.obj" \
	"$(INTDIR)\init.obj" \
	"$(INTDIR)\int.obj" \
	"$(INTDIR)\kbd.obj" \
	"$(INTDIR)\kbddrv.obj" \
	"$(INTDIR)\kdebug.obj" \
	"$(INTDIR)\kgrxcall.obj" \
	"$(INTDIR)\kmesg.obj" \
	"$(INTDIR)\kprocess.obj" \
	"$(INTDIR)\kshell.obj" \
	"$(INTDIR)\kshlcmd.obj" \
	"$(INTDIR)\ksignal.obj" \
	"$(INTDIR)\ksyscall.obj" \
	"$(INTDIR)\Ldr.obj" \
	"$(INTDIR)\Lucifer.obj" \
	"$(INTDIR)\Malloc.obj" \
	"$(INTDIR)\memory.obj" \
	"$(INTDIR)\module.obj" \
	"$(INTDIR)\mouse.obj" \
	"$(INTDIR)\Myasm.obj" \
	"$(INTDIR)\mydbg2.obj" \
	"$(INTDIR)\nic.obj" \
	"$(INTDIR)\paging.obj" \
	"$(INTDIR)\pci.obj" \
	"$(INTDIR)\rsh_serv.obj" \
	"$(INTDIR)\schedule.obj" \
	"$(INTDIR)\semaph.obj" \
	"$(INTDIR)\serial.obj" \
	"$(INTDIR)\Stk.obj" \
	"$(INTDIR)\Symtbl.obj" \
	"$(INTDIR)\tss.obj" \
	"$(INTDIR)\tty.obj" \
	"$(INTDIR)\util.obj" \
	"$(INTDIR)\v86.obj" \
	"$(INTDIR)\vconsole.obj" \
	"$(INTDIR)\vesa.obj" \
	".\lib\bell_fs.lib" \
	".\lib\asm.lib"

"$(OUTDIR)\bellona2.mod" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\bellona2.mod"
   copy debug\bellona2.lib lib\bellona2.lib /y
	stubv86\release\stubv86 debug\bellona2.mod v86lib\v86lib.bin
	if exist debug\bellona2.dbg del debug\bellona2.dbg /F /Q
	disk\codemap debug\bellona2.map debug\bellona2.dbg
	copy debug\bellona2.mod disk\bellona2.mod/y
	copy debug\bellona2.dbg disk\bellona2.dbg/y
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
!IF EXISTS("Test.dep")
!INCLUDE "Test.dep"
!ELSE 
!MESSAGE Warning: cannot find "Test.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "Test - Win32 Release" || "$(CFG)" == "Test - Win32 Debug"
SOURCE=.\3c905b.c

"$(INTDIR)\3c905b.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\bellona2.c

"$(INTDIR)\bellona2.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\cdrom.c

"$(INTDIR)\cdrom.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\chardev.c

"$(INTDIR)\chardev.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Codetbl.c

"$(INTDIR)\Codetbl.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\cs.c

"$(INTDIR)\cs.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\cursor.c

"$(INTDIR)\cursor.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\dbgreg.c

"$(INTDIR)\dbgreg.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\event.c

"$(INTDIR)\event.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\export.c

"$(INTDIR)\export.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\fdd.c

"$(INTDIR)\fdd.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ffmt.c

"$(INTDIR)\ffmt.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\fork.c

"$(INTDIR)\fork.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Gdt.c

"$(INTDIR)\Gdt.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\hdd.c

"$(INTDIR)\hdd.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\init.c

"$(INTDIR)\init.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\int.c

"$(INTDIR)\int.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\kbd.c

"$(INTDIR)\kbd.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\kbddrv.c

"$(INTDIR)\kbddrv.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\kdebug.c

"$(INTDIR)\kdebug.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\kgrxcall.c

"$(INTDIR)\kgrxcall.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\kmesg.c

"$(INTDIR)\kmesg.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\kprocess.c

"$(INTDIR)\kprocess.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\kshell.c

"$(INTDIR)\kshell.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\kshlcmd.c

"$(INTDIR)\kshlcmd.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ksignal.c

"$(INTDIR)\ksignal.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ksyscall.c

"$(INTDIR)\ksyscall.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Ldr.c

"$(INTDIR)\Ldr.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Lucifer.c

"$(INTDIR)\Lucifer.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Malloc.c

"$(INTDIR)\Malloc.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\memory.c

"$(INTDIR)\memory.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\module.c

"$(INTDIR)\module.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mouse.c

"$(INTDIR)\mouse.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Myasm.c

"$(INTDIR)\Myasm.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mydbg2.c

"$(INTDIR)\mydbg2.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\nic.c

"$(INTDIR)\nic.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\paging.c

"$(INTDIR)\paging.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\pci.c

"$(INTDIR)\pci.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\rsh_serv.c

"$(INTDIR)\rsh_serv.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\schedule.c

"$(INTDIR)\schedule.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\semaph.c

"$(INTDIR)\semaph.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\serial.c

"$(INTDIR)\serial.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Stk.c

"$(INTDIR)\Stk.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Symtbl.c

"$(INTDIR)\Symtbl.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\tss.c

"$(INTDIR)\tss.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\tty.c

"$(INTDIR)\tty.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\util.c

"$(INTDIR)\util.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\v86.c

"$(INTDIR)\v86.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\vconsole.c

"$(INTDIR)\vconsole.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\vesa.c

"$(INTDIR)\vesa.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 

