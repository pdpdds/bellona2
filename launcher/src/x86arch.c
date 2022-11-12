#include <x86arch.h>

void EnablePaging(bool state)
{
	_asm
	{
		mov	eax, cr0
		cmp[state], 1
		je	enable
		jmp disable

	enable :
		or eax, 0x80000000		//set bit 31
		mov	cr0, eax
		jmp done
		
	disable :		
		and eax, 0x7FFFFFFF		//clear bit 31
		mov	cr0, eax
	done :
	}
}

bool IsPaging()
{
	uint32_t res = 0;

	_asm 
	{
		mov	eax, cr0
		mov[res], eax
	}

	return (res & 0x80000000) ? false : true;
}

void InvalidateTLB(unsigned int va)
{
	__asm
	{
		invlpg	va
	}
}

void SetPDBR(uint32_t dir)
{
	_asm
	{
		mov	eax, [dir]
		mov	cr3, eax		// PDBR is cr3 register in i86
	}
}

uint32_t GetPDBR()
{
	unsigned int val;
	__asm
	{
		mov eax, cr3
		mov val, eax
	}

	return val;
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



