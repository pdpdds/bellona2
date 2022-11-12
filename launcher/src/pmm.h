#pragma once
#include "stdint.h"
#include <MultiBoot.h>

	
#define PMM_BLOCK_SIZE	4096	
#define PMM_BLOCK_ALIGN	BLOCK_SIZE
#define PMM_BITS_PER_INDEX	32

namespace pmm
{
	void	Initialize(multiboot_info_t* pBootInfo);

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
	uint32_t	AllocBlock();
	void	FreeBlock(void* pa);

	uint32_t	AllocBlocks(size_t);
	void	FreeBlocks(void* pa, size_t);

	void MarkMemBitmap(uint32_t base, size_t size);
	void UnmarkMemBitmap(uint32_t base, size_t size);

	int		GetBitmapEnd();
	
	
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
	uint32_t	GetTotalMemory();
	uint32_t	GetFreeMemory();
	void	Dump(); //Debug
}	
