#include "vmm.h"
#include "pmm.h"
#include "intrinsic.h"
#include <Constants.h>
#include "log.h"
#include <x86arch.h>
#include <string.h>
#include "MultibootUtil.h"

enum PageFlags {
	kPresent = 1,
	kWritable = 2,
	kGlobal = 256,
};

namespace vmm
{
	int identityPageTableCount = 0;
	uint32_t* g_pagedir;
	

	//Internal Method
	void MapPhysicalAddressToVirtualAddresss(PageDirectory* dir, uint32_t virt, uint32_t phys, uint32_t flags);
	void UnmapPhysicalAddress(PageDirectory* dir, uint32_t virt);
	void UnmapPageTable(PageDirectory* dir, uint32_t virt);
	//페이지 디렉토리를 생성한다.
	PageDirectory* CreatePageDirectory();
	//페이지 테이블을 생성한다. 페이지 테이블의 크기는 4K다.
	bool CreatePageTable(PageDirectory* dir, uint32_t virt, uint32_t flags);

	//페이지를 할당한다.
	bool AllocPage(PTE* e);
	//페이지를 회수한다.
	void FreePage(PTE* e);

	//가상 주소와 매핑된 물리 주소를 얻어낸다.
	uint32_t GetPhysicalAddress(PageDirectory* dir, uint32_t va)
	{
		PDE* pd = dir->m_entries;
		PDE pde = pd[PAGE_DIRECTORY_INDEX(va)];
		PTE* pt = (PTE*)(pde & ~0xfff);

		if (pde == 0)
			return 0;

		PTE pte = pt[PAGE_TABLE_INDEX(va)];
		if (pte == 0)
			return 0;

		uint32_t pa = pte & ~0xfff;
		uint32_t offset = PAGE_GET_OFFSET(va);

		return pa + offset;
	}

	//페이지 디렉토리 엔트리 인덱스가 0이 아니면 이미 페이지 테이블이 존재한다는 의미
	bool CreatePageTable(PageDirectory* dir, uint32_t va, uint32_t flags)
	{
		PDE* pd = dir->m_entries;
		if (pd[PAGE_DIRECTORY_INDEX(va)] == 0)
		{
			uint32_t pt = pmm::AllocBlock();
			if (pt == 0)
				return false;

			//20181130			
			memset((void*)pt, 0, sizeof(PageTable));
			pd[PAGE_DIRECTORY_INDEX(va)] = (pt) | flags;
		}
		return true;
	}

	//PDE나 PTE의 플래그는 같은 값을 공유
	//가상주소를 물리 주소에 매핑
	void MapPhysicalAddressToVirtualAddresss(PageDirectory* dir, uint32_t virt, uint32_t phys, uint32_t flags)
	{
		int flag = DisableInterrupts();
		PDE* pageDir = dir->m_entries;

		if (pageDir[virt >> 22] == 0)
		{
			CreatePageTable(dir, virt, flags);
		}

		uint32_t mask = (uint32_t)(~0xfff);
		uint32_t* pageTable = (uint32_t*)(pageDir[virt >> 22] & mask);

		pageTable[virt << 10 >> 10 >> 12] = phys | flags;

		RestoreInterrupts(flag);
	}

	void FreePageDirectory(PageDirectory* dir)
	{
		PDE* pageDir = dir->m_entries;
		for (int i = 0; i < PAGES_PER_DIRECTORY; i++)
		{
			PDE& pde = pageDir[i];

			if (pde != 0)
			{
				/* get mapped frame */
				void* frame = (void*)(pageDir[i] & 0x7FFFF000);
				pmm::FreeBlock(frame);
				pde = 0;
			}
		}
	}

	void UnmapPageTable(PageDirectory* dir, uint32_t virt)
	{
		PDE* pageDir = dir->m_entries;
		if (pageDir[virt >> 22] != 0) {

			/* get mapped frame */
			void* frame = (void*)(pageDir[virt >> 22] & 0x7FFFF000);

			/* unmap frame */
			pmm::FreeBlock(frame);
			pageDir[virt >> 22] = 0;
		}
	}

	void UnmapPhysicalAddress(PageDirectory* dir, uint32_t virt)
	{
		PDE* pagedir = dir->m_entries;
		if (pagedir[virt >> 22] != 0)
			UnmapPageTable(dir, virt);
	}

	PageDirectory* CreatePageDirectory()
	{
		PageDirectory* dir = (PageDirectory*)pmm::AllocBlock();
		if (!dir)
			return 0;

		return dir;
	}

	bool AllocPage(PTE* e)
	{
		uint32_t p = PageTableEntry::GetFrame(*e);
		
		if (p)
			return false;

		p = pmm::AllocBlock();

		if (p == 0)
			return false;

		PageTableEntry::SetFrame(e, p);
		PageTableEntry::AddAttribute(e, I86_PTE_PRESENT);

		return true;
	}

	void FreePage(PTE* e)
	{
		void* p = (void*)PageTableEntry::GetFrame(*e);
		if (p)
			pmm::FreeBlock(p);

		PageTableEntry::DelAttribute(e, I86_PTE_PRESENT);
	}

	//페이지 디렉토리 생성. 
	//페이지 디렉토리는 1024개의 페이지테이블을 가진다
	//1024 * 1024(페이지 테이블 엔트리의 개수) * 4K(프레임의 크기) = 4G	

	bool SetupPageDirectory(int identiyPageTableCount)
	{
		PageDirectory* _page_directory = (PageDirectory*)pmm::AllocBlocks(sizeof(PageDirectory) / PMM_BLOCK_SIZE);
		memset(_page_directory, 0, sizeof(PageDirectory));

		uint32_t frame = 0x00000000 + PAGE_SIZE;
		uint32_t virt = 0x00000000 + PAGE_SIZE;

		//페이지 테이블을 생성
		for (int i = 0; i < identiyPageTableCount; i++)
		{
			PageTable* identityPageTable = (PageTable*)pmm::AllocBlock();
			if (identityPageTable == 0)
			{
				return false;
			}

			memset(identityPageTable, 0, sizeof(PageTable));

			//널 포인터 에러를 위해 첫번째 페이지 테이블 엔트리는 설정하지 않는다.
			for (int j = 1; j < PAGES_PER_TABLE; j++, frame += PAGE_SIZE, virt += PAGE_SIZE)
			{
				PTE page = 0;
				PageTableEntry::AddAttribute(&page, I86_PTE_PRESENT | I86_PTE_WRITABLE | I86_PTE_CPU_GLOBAL);
				PageTableEntry::SetFrame(&page, frame);
				identityPageTable->m_entries[PAGE_TABLE_INDEX(virt)] = page;
			}

			//페이지 디렉토리에 페이지 디렉토리 엔트리(PDE)를 한 개 세트한다
			//0번째 인덱스에 PDE를 세트한다(가상주소가 0X00000000일시 참조됨)
			//앞에서 생성한 아이덴티티 페이지 테이블을 세트한다
			//가상주소 = 물리주소
			PDE* identityEntry = &_page_directory->m_entries[PAGE_DIRECTORY_INDEX((virt - 0x00400000))];
			PageDirectoryEntry::AddAttribute(identityEntry, I86_PDE_PRESENT | I86_PDE_WRITABLE);
			PageDirectoryEntry::SetFrame(identityEntry, (uint32_t)identityPageTable);
		}

		//페이지 디렉토리를 PDBR 레지스터에 로드한다		
		SetPDBR((uint32_t)_page_directory);

		return true;
	}
	

	uint32_t MapAddress(PageDirectory* dir, uint32_t startAddress, uint32_t pageCount)
	{
		uint32_t allocated_memory = pmm::AllocBlocks(pageCount);

		if (allocated_memory == 0)
		{
			LOG(LOG_FATAL, "MapAddress Fail!!\n");
		}

		uint32_t endAddress = (uint32_t)startAddress + pageCount * PMM_BLOCK_SIZE;

		for (int i = 0; i < (int)pageCount; i++)
		{
			uint32_t virt = (uint32_t)startAddress + i * PAGE_SIZE;
			uint32_t phys = allocated_memory + i * PAGE_SIZE;

			MapPhysicalAddressToVirtualAddresss(dir, virt, phys, I86_PTE_PRESENT | I86_PTE_WRITABLE | I86_PTE_CPU_GLOBAL);
		}

		return allocated_memory;
	}

	bool UnmapAddress(PageDirectory* pd, uint32_t startAddress, uint32_t pageCount)
	{

		for (int i = 0; i < (int)pageCount; i++)
		{
			uint32_t virt = (uint32_t)startAddress + i * PAGE_SIZE;

			UnmapPhysicalAddress(pd, virt);
		}

		return true;
	}

	void Initialize(multiboot_info_t* pBootInfo)
	{
		uint32_t moduleEndAddress = GetModuleEnd(pBootInfo);

		//4MB Identity Mapping
		identityPageTableCount = moduleEndAddress / (MEGA_BYTES * 4);

		if (moduleEndAddress % (MEGA_BYTES * 4) > 0)
			identityPageTableCount += 1;

		if (identityPageTableCount == 0)
			LOG(LOG_FATAL, "invalid mask PageCount\n");
		
		SetupPageDirectory(identityPageTableCount);

		g_pagedir = (unsigned int*)GetPDBR();

		LOG(LOG_INFO, "Page Directory Address : %x\n", g_pagedir);
		LOG(LOG_INFO, "identity Page Table Count : %x\n", identityPageTableCount);
	}

	uint32_t AllocatePage()
	{
		uint32_t frame = (uint32_t)pmm::AllocBlock();
		memset((void*)frame, 0, PAGE_SIZE);
		return frame;
	}

	void MapPage(unsigned va, unsigned pa, bool writable)
	{
		int pdindex = va / PAGE_SIZE / 1024;
		if (!(g_pagedir[pdindex] & kPresent))
			g_pagedir[pdindex] = AllocatePage() | kPresent | kGlobal | kWritable;

		unsigned* pagetable = (unsigned*)(g_pagedir[pdindex] & 0xfffff000);
		pagetable[(va / PAGE_SIZE) % 1024] = pa | kPresent | kGlobal | (writable ? kWritable : 0);

		InvalidateTLB(va);
	}
}