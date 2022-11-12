#pragma once
#define MEGA_BYTES 1048576
#define KILO_BYTES 1024

#define PAGE_ALIGN_DOWN(value)				((value) & ~(PAGE_SIZE - 1))
#define PAGE_ALIGN_UP(value)				(((value) & (PAGE_SIZE - 1)) ? (PAGE_ALIGN_DOWN((value)) + PAGE_SIZE) : (value))

typedef struct tag_BootInfo
{
	uint32_t* _hyperPTE;
	uint32_t _memsize;
	uint32_t _kernel_load_address;
	uint32_t _kernel_boot_stack_address;
	uint32_t _kernel_boot_stack_size;
	uint32_t _kernel_heap_address;
	uint32_t _kernel_heap_size;
	uint32_t _paging;
	uint32_t _isDll;
	char	 _kernelName[32];
}BootInfo;

