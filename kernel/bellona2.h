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
	DWORD           dwTimerInterval;			// Ÿ�̸� ���ͷ�Ʈ �߻��󵵸� �����ϱ� ���� ��. 3600�̸� 1�ʿ� 18.2ȸ, 200�̸� 18.2ȸ * 18ȸ
	DWORD           dwTick;					// Ÿ�̸� ���ͷ�Ʈ�� �� �� �߻��� ������ 1�� �����Ǵ� ī���� 
	DWORD           dwTickCarry;			// dwTick���� �ִ밪���� 0���� Overflow�� �߻������� �ϳ� �����Ѵ�.
	DWORD           dwIdleTick;				// Running State�� �ִ� ���μ����� �ϳ��� ��� Idle ���·� �ð��� ���� Tick
	DWORD           dwTimerIntPerSecond;	// 1�ʿ� �߻��ϴ� Ÿ�̸� ���ͷ�Ʈ�� ȸ��
	int				nPhysSize;				// ���� �޸� ũ�� (Byte����)
	int				nPhysRefSize;			// ���� �޸� ���� ���̺� ũ��
	UCHAR			*pPhysRefTbl;			// ���� �޸� ���� ���̺� �ּ�
	DWORD			*pPD;					// ������ ���丮�� �ּ�
	DWORD			dwBuiltInV86Lib;		// ���� V86 Lib�� ������
	int				nBuiltInV86LibSize;		// ���� V86 Lib�� ������
	DWORD			dwLastImageByte;		// Bellona2Ŀ���� ������ ����Ʈ ��ġ
	DWORD			dwNextProcessThreadID;	// ���� ���� �Ҵ��� ���μ����� ������ ID
	DWORD			dwInitPhase;			// �ʱ�ȭ ���� �ܰ�
	
	DWORD			dwDbgMappingFlag;		// dwKernelMappingFlag���� ����ȭ�� ���� ��.
	
	MY_IMAGE_EXPORT_DIRECTORY		*pExp;	// Export Directory�� �ּ�
};
typedef struct BellonaTag BellonaStt;


extern DescriptorStt	gdt[MAX_GDT];  // GDT
extern DescriptorStt	ldt[MAX_LDT];  // LDT
extern IDTStt			idt[MAX_IDT];  // IDT
extern IDTRStt			idtr;		   // IDTR
extern GDTRStt			gdtr;		   // GDTR�� ��
extern int				nTotalGDTEnt;
extern BellonaStt		bell;
extern MyCoffDbg2Stt	*pMy;;

extern void bellona2_main();
extern int nWriteToVideoMem_Len( int x, int y, char *pS, int nLen );
extern int nWriteToVideoMem( int x, int y, char *pS );

#endif
