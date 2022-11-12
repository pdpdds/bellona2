#include "cpu_asm.h"

/// Return the physical address of the current page directory
extern "C" unsigned int GetCurrentPageDir()
{
	unsigned int val;
	__asm
	{
		mov eax, cr3
		mov val, eax
	}

	return val;
}

/// Set the physical address of the current page directory
extern "C" void SetCurrentPageDir(unsigned int addr)
{
	_asm
	{
		mov	eax, [addr]
		mov	cr3, eax
	}
}

inline cpu_flags DisableInterrupts()
{
	int fl = 0;
	__asm	PUSHFD
	__asm	POP fl
	__asm	CLI
	return fl;
}

inline void RestoreInterrupts(const cpu_flags flags)
{
	__asm	PUSH	flags
	__asm	POPFD
}

inline void EnableInterrupts()
{
	__asm sti
}