#ifndef BELLONA2_GLOBAL_CONST_HEADER_oh
#define BELLONA2_GLOBAL_CONST_HEADER_oh

#define IMAGE_BASE 0x200000			// Bload.com���� �̹����� �÷� ���� �κ��� �ּ�
#define RELOC_BASE 0x400000			// ���ġ �� �Ŀ� �̹����� ��ġ�� ��, 4�ް� �κп� ����.
									// ���� PE������ 4M���� ���̽��� �����Ƿ� �װ� ���ϴ�.

#define BDF_CACHE_SIZE 0x100000
#define BDF_CACHE_BASE 0x300000

#define VGA_VIDEO_BASE 0xB8000      // VGA ī���� Video Base �ּ�

#define MAX_GDT 64					// ����Ʈ�� ���� 64���� gdt������ Ȯ���� �д�.
#define MAX_LDT  2
#define MAX_IDT 256

// initial stack base of INIT TASK
#define KERNEL_INIT_STACK_BASE    0x500000					

// initial stack base of DBG TASK
#define KERNEL_DBG_STACK_BASE	  (0x500000 - (1024*64))

// Page Fault Stack
#define PAGE_FAULT_STACK_BASE	  (0x500000 - (1024*128))

// initial kernel stack size (INIT+DBG)
#define KERNEL_STACK_SIZE	      (1024*(64*3))				

#define MAX_ARGV	32

#endif
