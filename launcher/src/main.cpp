#include "main.h"
#include "log.h"
#include "PEUtil.h"


extern "C" char* heap_base;
extern "C" int heap_size;


BootInfo g_bootInfo;




extern "C" int kmain(unsigned long magic, unsigned long addr)
{
	InitCRT();

	multiboot_info_t* mb_info = (multiboot_info_t*)addr;

	InitLogManager(LOG_DEBUG);
	
	LOG(LOG_INFO, "YUZA OS Framework Start\n");
	LOG(LOG_INFO, "BootLoader : %s, Magic : %x\n", mb_info->boot_loader_name, magic);

	InitGDT();
	
	InitIDT(0x8);
	//InitTSS(5, 0x10, 0);
	LOG(LOG_INFO, "BootLoader : %s, Magic : %x\n", mb_info->boot_loader_name, magic);
	/*InstallTrapHandler();

	InitPIC(0x20, 0x28);

	if (InitFPU())
		EnableFPU();
	else
		LOG(LOG_FATAL, "FPU Init Fail!!\n");*/

	LOG(LOG_INFO, "CPU Init Pass\n");
	
	//pmm::Initialize(mb_info);
	//vmm::Initialize(mb_info);

	g_bootInfo._hyperPTE = 0;
	g_bootInfo._kernel_load_address = 0x00200000;
	g_bootInfo._kernel_boot_stack_address = 0xC0000000;
	g_bootInfo._kernel_boot_stack_size = 0x00100000;
	g_bootInfo._kernel_heap_address = 0xC0200000;
	g_bootInfo._kernel_heap_size = 0x00c00000;
	g_bootInfo._paging = 1;

	//heap_base = (char*)0xC0200000;
	//heap_size = 0x00c00000;

	g_bootInfo._memsize = 1024 * 1024 * 512;
	g_bootInfo._isDll = 0;
	strcpy(g_bootInfo._kernelName, "bellona2.bin");

	Module* pKernel = FindModule(mb_info, g_bootInfo._kernelName);

	if (pKernel == 0)
		LOG(LOG_FATAL, "Kernel Not Found %s\n", g_bootInfo._kernelName);
	
	/*PE_TYPE peType = CheckPEFormat(pKernel);

	g_bootInfo._isDll = IsDll((void*)pKernel->ModuleStart);

	if (PE_TYPE::PE_TYPE_INVALID == peType || PE_TYPE::PE_TYPE_FILE_NOT_FOUND == peType)
		LOG(LOG_FATAL, "Check PE Format Fail : %d\n", peType);*/

	LOG(LOG_INFO, "Kernel Found %s\n", g_bootInfo._kernelName);
	
	
	//if(PE_TYPE::PE_TYPE_32 == peType)
		Boot32BitKernel(mb_info);
	//else
		//Boot64BitKernel(mb_info);	
	
	return 0;
}

int MakePagingEnviornment(Module* pKernel)
{
	
	uint32_t fileSize = GetModuleSize(pKernel);

	if (!pKernel || !fileSize)
		LOG(LOG_FATAL, "Kernel %s Find Fail!!\n", g_bootInfo._kernelName);

	PIMAGE_DOS_HEADER DOS = (PIMAGE_DOS_HEADER)pKernel->ModuleStart;
	PIMAGE_NT_HEADERS NT = (PIMAGE_NT_HEADERS)(pKernel->ModuleStart + DOS->e_lfanew);

	uint32_t kernelSize = NT->OptionalHeader.SizeOfImage;

	uint32_t page_count = kernelSize / PAGE_SIZE;

	if (kernelSize % PAGE_SIZE)
		page_count += 1;


	uint32_t hyper_PTE = vmm::AllocatePage();
	g_bootInfo._hyperPTE = (uint32_t*)hyper_PTE;
	
	for (uint32_t va = g_bootInfo._kernel_load_address; va < (g_bootInfo._kernel_load_address + page_count * PAGE_SIZE); va += PAGE_SIZE)
		vmm::MapPage(va, vmm::AllocatePage(), true);

	for (uint32_t va = g_bootInfo._kernel_boot_stack_address; va < g_bootInfo._kernel_boot_stack_address + g_bootInfo._kernel_boot_stack_size; va += PAGE_SIZE)
		vmm::MapPage(va, vmm::AllocatePage(), true);

	for (uint32_t va = g_bootInfo._kernel_heap_address; va < g_bootInfo._kernel_heap_address + g_bootInfo._kernel_heap_size; va += PAGE_SIZE)
		vmm::MapPage(va, vmm::AllocatePage(), true);

	EnablePaging(true);
	
	return page_count;
}

DWORD LoadKernel(Module* pKernel, int pageCount)
{

	/*LOG(LOG_INFO, "LOAD KERNEL\n");

	LOAD_DLL_INFO p;
	strcpy(p.moduleName, pKernel->Name);
	p.refCount = 1;

	DWORD res = LoadDLLFromMemory((void*)pKernel->ModuleStart, pageCount, 0, &p);
	if (res != ELoadDLLResult_OK)
		LOG(LOG_FATAL, "LoadPEFromMemory Fail. Name : %s, Result : %d\n", pKernel->Name, res);

	LOG(LOG_INFO, "kernel 0x%x Relocation Success\n", g_bootInfo._kernel_load_address);
	LOG(LOG_INFO, "jump to the kernel entry 0x%x\n", (DWORD)p.dll_main);

	for (;;);

	return (DWORD)p.dll_main;*/

	

	memcpy((void*)g_bootInfo._kernel_load_address, (void*)pKernel->ModuleStart, ((int)pKernel->ModuleEnd - (int)pKernel->ModuleStart));

	IMAGE_DOS_HEADER* dosHeader = 0;
	IMAGE_NT_HEADERS* ntHeaders = 0;

	dosHeader = (IMAGE_DOS_HEADER*)g_bootInfo._kernel_load_address;
	ntHeaders = (IMAGE_NT_HEADERS*)(dosHeader->e_lfanew + (uint32_t)g_bootInfo._kernel_load_address);

	DWORD imageBase = g_bootInfo._kernel_load_address;
	uint32_t entryPoint = (uint32_t)ntHeaders->OptionalHeader.AddressOfEntryPoint + imageBase;

	LOG(LOG_INFO, "kernel 0x%x Relocation Success\n", g_bootInfo._kernel_load_address);
	LOG(LOG_INFO, "jump to the kernel entry 0x%x\n", (DWORD)entryPoint);

	return entryPoint;
}

BootInfo* MakeBootParam()
{
	uint32_t stackPointer = g_bootInfo._kernel_boot_stack_address + g_bootInfo._kernel_boot_stack_size;
	BootInfo* info = (BootInfo*)(stackPointer - sizeof(BootInfo));
	
	memcpy((void*)info, &g_bootInfo, sizeof(BootInfo));
	return info;

}

bool Boot32BitKernel(multiboot_info_t* mb_info)
{
	Module* pKernel = FindModule(mb_info, g_bootInfo._kernelName);
	//int pageCount = 0;

	
	//pageCount = MakePagingEnviornment(pKernel);

	//if (pageCount == 0)
		//LOG(LOG_FATAL, "Identity Page Table Count 0\n");
	

	PIMAGE_DOS_HEADER DOS = (PIMAGE_DOS_HEADER)pKernel->ModuleStart;
	PIMAGE_NT_HEADERS NT = (PIMAGE_NT_HEADERS)(pKernel->ModuleStart + DOS->e_lfanew);

	uint32_t kernelSize = NT->OptionalHeader.SizeOfImage;

	DWORD mainFunc = LoadKernel(pKernel, kernelSize);

	//BootInfo* bootInfo = MakeBootParam();

	//DWORD bootStackAddress = (DWORD)bootInfo - 4;

	//LOG(LOG_FATAL, "Jump to Kernel 0x%x\n", mainFunc);

	//__asm call mainFunc
	
	StartKernel(mainFunc, 0x500000, (DWORD)0, g_bootInfo._isDll);

	return true;
}


void StartKernel(uint32_t entryPoint, uint32_t stackPointer, uint32_t param, uint32_t isDll)
{
	__asm
	{
		MOV     AX, 0x10;
		MOV     DS, AX
		MOV     ES, AX
		MOV     FS, AX
		MOV     GS, AX

		MOV     ESP, stackPointer
		PUSH	param; //parameter

		
		PUSH	0; //EBP
		PUSH    0x200; EFLAGS
		PUSH    0x08; CS
		PUSH    entryPoint; EIP
		IRETD
	}
}



