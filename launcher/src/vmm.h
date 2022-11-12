#pragma once
#include "stdint.h"
#include "PageDirectoryEntry.h"
#include "PageTableEntry.h"
#include "MultiBoot.h"

using namespace PageTableEntry;
using namespace PageDirectoryEntry;

// i86 아키텍쳐에서는 페이지테이블이나 페이지 디렉토리가 각각 1024개의 엔트리를 가진다
// 32비트에서 엔트리의 크기는 4바이트다. 
// 크기(4 * 1024 = 4K)
// 따라서 한 프로세스는 가상주소 4기가를 전부 표현하기 위해 약 4메가바이트의
// 메모리 공간을 필요로 한다(4K(PDE) + 1024 * 4K(PTE))
// 하나의 페이지 테이블은 4MB를 표현할 수 있고 페이지 디렉토리는 1024개의 페이지 테이블을
// 표현할 수 있으므로 4MB * 1024 = 4GB, 즉 4GB 바이트 전체를 표현할 수 있다.

#define PAGES_PER_TABLE		1024
#define PAGES_PER_DIRECTORY	1024
#define PAGE_TABLE_SIZE		4096

//페이지 테이블 하나당 주소 공간 : 4MB
#define PTABLE_ADDR_SPACE_SIZE 0x400000
//페이지 디렉토리 하나가 표현할 수 있는 있는 주소 공간 4GB
#define DTABLE_ADDR_SPACE_SIZE 0x100000000

#define PAGE_DIRECTORY_INDEX(x) (((x) >> 22) & 0x3ff)
#define PAGE_TABLE_INDEX(x) (((x) >> 12) & 0x3ff)
#define PAGE_GET_OFFSET(x) (x & 0xfff)

#define MAX_PAGE_DIRECTORY_COUNT 40

typedef struct tag_PageTable 
{
	PTE m_entries[PAGES_PER_TABLE];
}PageTable;

typedef struct tag_PageDirectory 
{
	PDE m_entries[PAGES_PER_DIRECTORY];
}PageDirectory;

namespace vmm
{
	void Initialize(multiboot_info_t* pBootInfo);
	uint32_t AllocatePage();
	void MapPage(unsigned va, unsigned pa, bool writable);

	//가상 메모리 시스템을 초기화한다.			
	bool SetupPageDirectory(int pageTableCount);
	//가상주소로부터 실제 물리주소를 얻어낸다
	uint32_t GetPhysicalAddress(PageDirectory* dir, uint32_t va);

	uint32_t MapAddress(PageDirectory* dir, uint32_t startAddress, uint32_t pageCoount);
	bool UnmapAddress(PageDirectory* dir, uint32_t startAddress, uint32_t pageCount);
	
	void FreePageDirectory(PageDirectory* dir);	

	extern int identityPageTableCount;
}