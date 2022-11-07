# Microsoft Developer Studio Generated NMAKE File, Based on Bellona2.dsp
!IF "$(CFG)" == ""
CFG=Bellona2 - Win32 Debug
!MESSAGE No configuration specified. Defaulting to Bellona2 - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "Bellona2 - Win32 Release" && "$(CFG)" != "Bellona2 - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Bellona2.mak" CFG="Bellona2 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Bellona2 - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "Bellona2 - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "Bellona2 - Win32 Release"

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
	-@erase "$(INTDIR)\kshell.obj"
	-@erase "$(INTDIR)\kshlcmd.obj"
	-@erase "$(INTDIR)\ksignal.obj"
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
	-@erase "$(INTDIR)\process.obj"
	-@erase "$(INTDIR)\rsh_serv.obj"
	-@erase "$(INTDIR)\schedule.obj"
	-@erase "$(INTDIR)\semaph.obj"
	-@erase "$(INTDIR)\serial.obj"
	-@erase "$(INTDIR)\Stk.obj"
	-@erase "$(INTDIR)\Symtbl.obj"
	-@erase "$(INTDIR)\syscall.obj"
	-@erase "$(INTDIR)\tss.obj"
	-@erase "$(INTDIR)\tty.obj"
	-@erase "$(INTDIR)\util.obj"
	-@erase "$(INTDIR)\v86.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vesa.obj"
	-@erase "$(OUTDIR)\Bellona.bin"
	-@erase "$(OUTDIR)\Bellona.map"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /G4 /Zp1 /ML /W3 /vmg /vd0 /GX /Od /Gf /D "BELLONA" /FAc /Fa"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Bellona2.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=/nologo /base:"0x400000" /entry:"bellona2_main" /subsystem:console /incremental:no /pdb:"$(OUTDIR)\Bellona.pdb" /map:"$(INTDIR)\Bellona.map" /machine:I386 /nodefaultlib /out:"$(OUTDIR)\Bellona.bin" 
LINK32_OBJS= \
	"$(INTDIR)\3c905b.obj" \
	"$(INTDIR)\bellona2.obj" \
	"$(INTDIR)\cdrom.obj" \
	"$(INTDIR)\chardev.obj" \
	"$(INTDIR)\Codetbl.obj" \
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
	"$(INTDIR)\kshell.obj" \
	"$(INTDIR)\kshlcmd.obj" \
	"$(INTDIR)\ksignal.obj" \
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
	"$(INTDIR)\process.obj" \
	"$(INTDIR)\rsh_serv.obj" \
	"$(INTDIR)\schedule.obj" \
	"$(INTDIR)\semaph.obj" \
	"$(INTDIR)\serial.obj" \
	"$(INTDIR)\Stk.obj" \
	"$(INTDIR)\Symtbl.obj" \
	"$(INTDIR)\syscall.obj" \
	"$(INTDIR)\tss.obj" \
	"$(INTDIR)\tty.obj" \
	"$(INTDIR)\util.obj" \
	"$(INTDIR)\v86.obj" \
	"$(INTDIR)\vesa.obj" \
	".\asm.obj" \
	".\bell_fs.lib"

"$(OUTDIR)\Bellona.bin" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Bellona2 - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\Bellona2.org"


CLEAN :
	-@erase "$(INTDIR)\3c905b.obj"
	-@erase "$(INTDIR)\bellona2.obj"
	-@erase "$(INTDIR)\cdrom.obj"
	-@erase "$(INTDIR)\chardev.obj"
	-@erase "$(INTDIR)\Codetbl.obj"
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
	-@erase "$(INTDIR)\kshell.obj"
	-@erase "$(INTDIR)\kshlcmd.obj"
	-@erase "$(INTDIR)\ksignal.obj"
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
	-@erase "$(INTDIR)\process.obj"
	-@erase "$(INTDIR)\rsh_serv.obj"
	-@erase "$(INTDIR)\schedule.obj"
	-@erase "$(INTDIR)\semaph.obj"
	-@erase "$(INTDIR)\serial.obj"
	-@erase "$(INTDIR)\Stk.obj"
	-@erase "$(INTDIR)\Symtbl.obj"
	-@erase "$(INTDIR)\syscall.obj"
	-@erase "$(INTDIR)\tss.obj"
	-@erase "$(INTDIR)\tty.obj"
	-@erase "$(INTDIR)\util.obj"
	-@erase "$(INTDIR)\v86.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vesa.obj"
	-@erase "$(OUTDIR)\Bellona2.map"
	-@erase "$(OUTDIR)\Bellona2.org"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /G4 /Zp1 /MLd /W3 /vd0 /Od /Gf /X /u /D "BELLONA" /FAcs /Fa"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /Yd /c 

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

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Bellona2.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=/nologo /base:"0x400000" /entry:"bellona2_main" /subsystem:console /pdb:none /map:"$(INTDIR)\Bellona2.map" /machine:I386 /nodefaultlib /out:"$(OUTDIR)\Bellona2.org" /FIXED 
LINK32_OBJS= \
	"$(INTDIR)\3c905b.obj" \
	"$(INTDIR)\bellona2.obj" \
	"$(INTDIR)\cdrom.obj" \
	"$(INTDIR)\chardev.obj" \
	"$(INTDIR)\Codetbl.obj" \
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
	"$(INTDIR)\kshell.obj" \
	"$(INTDIR)\kshlcmd.obj" \
	"$(INTDIR)\ksignal.obj" \
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
	"$(INTDIR)\process.obj" \
	"$(INTDIR)\rsh_serv.obj" \
	"$(INTDIR)\schedule.obj" \
	"$(INTDIR)\semaph.obj" \
	"$(INTDIR)\serial.obj" \
	"$(INTDIR)\Stk.obj" \
	"$(INTDIR)\Symtbl.obj" \
	"$(INTDIR)\syscall.obj" \
	"$(INTDIR)\tss.obj" \
	"$(INTDIR)\tty.obj" \
	"$(INTDIR)\util.obj" \
	"$(INTDIR)\v86.obj" \
	"$(INTDIR)\vesa.obj" \
	".\asm.obj" \
	".\bell_fs.lib"

"$(OUTDIR)\Bellona2.org" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\Bellona2.org"
   copy debug\*.cod cod /y
	copy debug\*.map cod /y
	copy debug\bellona2.lib disk
	copy debug\bellona2.org debug\bellona2-d.bin
	copy debug\bellona2.org debug\bellona2-r.bin
	cod\codemap \oh\os\bellona2\cod\bellona2.map \oh\os\bellona2\debug\bellona2-d.bin
	stubv86\release\stubv86 debug\bellona2-d.bin v86lib\v86lib.bin
	stubv86\release\stubv86 debug\bellona2-r.bin v86lib\v86lib.bin
	copy debug\bellona2-d.bin disk
	copy debug\bellona2-r.bin disk
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("Bellona2.dep")
!INCLUDE "Bellona2.dep"
!ELSE 
!MESSAGE Warning: cannot find "Bellona2.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "Bellona2 - Win32 Release" || "$(CFG)" == "Bellona2 - Win32 Debug"
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


SOURCE=.\kshell.c

"$(INTDIR)\kshell.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\kshlcmd.c

"$(INTDIR)\kshlcmd.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ksignal.c

"$(INTDIR)\ksignal.obj" : $(SOURCE) "$(INTDIR)"


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


SOURCE=.\process.c

"$(INTDIR)\process.obj" : $(SOURCE) "$(INTDIR)"


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


SOURCE=.\syscall.c

"$(INTDIR)\syscall.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\tss.c

"$(INTDIR)\tss.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\tty.c

"$(INTDIR)\tty.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\util.c

"$(INTDIR)\util.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\v86.c

"$(INTDIR)\v86.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\vesa.c

"$(INTDIR)\vesa.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 

