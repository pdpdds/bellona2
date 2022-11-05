#ifndef BELLONA2_GLOBAL_CONST_HEADER_oh
#define BELLONA2_GLOBAL_CONST_HEADER_oh

#define IMAGE_BASE 0x200000			// Bload.com에서 이미지를 올려 놓는 부분의 주소
#define RELOC_BASE 0x400000			// 재배치 한 후에 이미지가 위치할 곳, 4메가 부분에 들어간다.
									// 원래 PE파일이 4M으로 베이스가 잡히므로 그게 편하다.

#define BDF_CACHE_SIZE 0x100000
#define BDF_CACHE_BASE 0x300000

#define VGA_VIDEO_BASE 0xB8000      // VGA 카드의 Video Base 주소

#define MAX_GDT 64					// 디폴트로 먼저 64개의 gdt공간을 확보해 둔다.
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
