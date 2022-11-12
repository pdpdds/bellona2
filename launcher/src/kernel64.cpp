#include "Kernel64.h"
#include "log.h"
#include "PEUtil.h"
#include "PEImage.h"
#include "Page.h"

extern "C" void SwitchAndExecute64bitKernel(int pml4EntryAddress, int kernelEntry, int bootinfo);

bool Is64BitSwitchPossible()
{
	if (DetectionCPUID() == false)
		return false;

	LOG(LOG_DEBUG, "CPUID Detected..\n");

	if (IsLongModeCheckPossible() == false)
		return false;

	LOG(LOG_DEBUG, "Long Mode Check Possible..\n");

	if (IsLongModePossible() == false)
		return false;

	LOG(LOG_DEBUG, "Long Mode Possible..\n");

	return true;
}


bool DetectionCPUID()
{
	bool result = false;
	__asm
	{
		pushfd
		pop eax

		; Copy to ECX as well for comparing later on
		mov ecx, eax

		; Flip the ID bit
		xor eax, 1 << 21

		; Copy EAX to FLAGS via the stack
		push eax
		popfd

		; Copy FLAGS back to EAX(with the flipped bit if CPUID is supported)
		pushfd
		pop eax

		; Restore FLAGS from the old version stored in ECX(i.e.flipping the ID bit
		; back if it was ever flipped).
		push ecx
		popfd

		; Compare EAXand ECX.If they are equal then that means the bit wasn't
		; flipped, and CPUID isn't supported.
		xor eax, ecx
		jz NoCPUID
		mov result, 1
		NoCPUID:

	}

	return result;
}

bool IsLongModeCheckPossible()
{
	bool result = false;
	__asm
	{
		mov eax, 0x80000000; Set the A - register to 0x80000000.
		cpuid; CPU identification.
		cmp eax, 0x80000001; Compare the A - register with 0x80000001.
		jb NoLongMode; It is less, there is no long mode.
		mov result, 1
		NoLongMode:
	}

	return result;
}

bool IsLongModePossible()
{
	bool result = false;
	__asm
	{
		mov eax, 0x80000001; Set the A - register to 0x80000001.
		cpuid; CPU identification.
		test edx, 1 << 29; Test if the LM - bit, which is bit 29, is set in the D - register.
		jz NoLongMode; They aren't, there is no long mode.
		mov result, 1
		NoLongMode:
	}

	return result;
}
