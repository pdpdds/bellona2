#include "PEImage.h"
#include "PEUtil.h"
#include "log.h"
#include "Constants.h"

extern BootInfo g_bootInfo;

//32비트 PE파일 이미지 유효성 검사
static bool IsPE32Image(void* image)
{
	IMAGE_DOS_HEADER* dosHeader = 0;
	IMAGE_NT_HEADERS* ntHeaders = 0;

	dosHeader = (IMAGE_DOS_HEADER*)image;
	if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE)
		return false;

	if (dosHeader->e_lfanew == 0)
		return false;

	//NT Header 체크
	ntHeaders = (IMAGE_NT_HEADERS*)(dosHeader->e_lfanew + (uint32_t)image);
	if (ntHeaders->Signature != IMAGE_NT_SIGNATURE)
		return false;

	// only supporting for i386 archs 
	if (ntHeaders->FileHeader.Machine != IMAGE_FILE_MACHINE_I386)
		return false;

	// only support 32 bit executable images 
	if (!(ntHeaders->FileHeader.Characteristics & (IMAGE_FILE_EXECUTABLE_IMAGE | IMAGE_FILE_32BIT_MACHINE)))
		return false;

	//Note: 1st 4 MB remains idenitity mapped as kernel pages as it contains
	//kernel stack and page directory. If you want to support loading below 1MB,
	//make sure to move these into kernel land

	// only support 32 bit optional header format 
	if (ntHeaders->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC)
		return false;

	//유효한 32비트 PE 파일이다.
	return true;
}

PE_TYPE CheckPEFormat(Module* pKernel)
{
	if (pKernel == 0)
		return PE_TYPE::PE_TYPE_FILE_NOT_FOUND;

	if (IsPE32Image((void*)pKernel->ModuleStart))
		return PE_TYPE::PE_TYPE_32;

	

	return PE_TYPE::PE_TYPE_INVALID;
}

bool IsDll(void* image)
{
	IMAGE_DOS_HEADER* dosHeader = 0;
	IMAGE_NT_HEADERS* ntHeaders = 0;

	dosHeader = (IMAGE_DOS_HEADER*)image;
	if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE)
		return false;

	if (dosHeader->e_lfanew == 0)
		return false;

	//NT Header 체크
	ntHeaders = (IMAGE_NT_HEADERS*)(dosHeader->e_lfanew + (uint32_t)image);
	if (ntHeaders->Signature != IMAGE_NT_SIGNATURE)
		return false;

	if (ntHeaders->FileHeader.Characteristics & IMAGE_FILE_DLL)
		return true;

	return false;
}



