#ifndef BELLONA2_MAIN_HEADER_oh
#define BELLONA2_MAIN_HEADER_oh

// original process definition : WIN32,_DEBUG,_CONSOLE,_MBCS

#include "types.h"
#include "mmap.h"
#include "bell_fs\vfs.h"
#include "bell_fs\jcommand.h"

#include "ksignal.h"
#include "major.h"
#include "const.h"
#include "pefile.h"
#include "util.h"
#include "asm.h"
#include "gdt.h"
#include "int.h"
#include "kbd.h"
#include "memory.h"
#include "paging.h"
#include "malloc.h"
#include "kdebug.h"
#include "kshlcmd.h"
#include "cursor.h"
#include "kshell.h"
#include "ffmt.h"
#include "fdd.h"
#include "tss.h"
#include "eflag.h"
#include "dbgreg.h"
#include "CodeTbl.h"
#include "LUCIFER.H"
#include "myasm.h"
#include "stk.h"
#include "ksyscall.h"
#include "kmesg.h"
#include "kprocess.h"
#include "schedule.h"
#include "init.h"
#include "hdd.h"
#include "event.h"
#include "mydbg2.h"
#include "fork.h"
#include "ldr.h"
#include "serial.h"
#include "chardev.h"
#include "kbddrv.h"
#include "rsh_serv.h"
#include "tty.h"
#include "pci.h"
#include "cdrom.h"
#include "semaph.h"
#include "module.h"
#include "3c905b.h"
#include "nic.h"
#include "v86.h"
#include "mouse.h"
#include "vesa.h"
#include "export.h"
#include "cs.h"
#include "vconsole.h"

#include "h\process.h"
#include "h\common\uarea.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

//--------------------------------------------------------------//
typedef struct BellonaTag{
	DWORD           dwTimerInterval;			// 타이머 인터럽트 발생빈도를 조정하기 위한 값. 3600이면 1초에 18.2회, 200이면 18.2회 * 18회
	DWORD           dwTick;					// 타이머 인터럽트가 한 번 발생할 때마다 1씩 증가되는 카운터 
	DWORD           dwTickCarry;			// dwTick에서 최대값에서 0으로 Overflow가 발생했으면 하나 증가한다.
	DWORD           dwIdleTick;				// Running State에 있는 프로세스가 하나도 없어서 Idle 상태로 시간을 보낸 Tick
	DWORD           dwTimerIntPerSecond;	// 1초에 발생하는 타이머 인터럽트의 회수
	int				nPhysSize;				// 물리 메모리 크기 (Byte단위)
	int				nPhysRefSize;			// 물리 메모리 참조 테이블 크기
	UCHAR			*pPhysRefTbl;			// 물리 메모리 참조 테이블 주소
	DWORD			*pPD;					// 페이지 디렉토리의 주소
	DWORD			dwBuiltInV86Lib;		// 내장 V86 Lib의 오프셋
	int				nBuiltInV86LibSize;		// 내장 V86 Lib의 사이즈
	DWORD			dwLastImageByte;		// Bellona2커널의 마지막 바이트 위치
	DWORD			dwNextProcessThreadID;	// 다음 번에 할당할 프로세스와 쓰레드 ID
	DWORD			dwInitPhase;			// 초기화 진행 단계
	
	DWORD			dwDbgMappingFlag;		// dwKernelMappingFlag와의 동기화를 위한 것.
	
	MY_IMAGE_EXPORT_DIRECTORY		*pExp;	// Export Directory의 주소
};
typedef struct BellonaTag BellonaStt;


extern DescriptorStt	gdt[MAX_GDT];  // GDT
extern DescriptorStt	ldt[MAX_LDT];  // LDT
extern IDTStt			idt[MAX_IDT];  // IDT
extern IDTRStt			idtr;		   // IDTR
extern GDTRStt			gdtr;		   // GDTR의 값
extern int				nTotalGDTEnt;
extern BellonaStt		bell;
extern MyCoffDbg2Stt	*pMy;;

extern void bellona2_main();
extern int nWriteToVideoMem_Len( int x, int y, char *pS, int nLen );
extern int nWriteToVideoMem( int x, int y, char *pS );

#endif
