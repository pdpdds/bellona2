[BITS 32]      

SECTION .text

%define TRAP_ERRC(vector) \
					global _trap##vector; \
					_trap##vector:			\
					push vector;\
					jmp _SetupTrapCommon;

; Processor exceptions
global _trap0
global _trap1
global _trap2
global _trap3
global _trap4
global _trap5
global _trap6
global _trap7
global _trap8
global _trap9
global _trap10
global _trap11
global _trap12
global _trap13
global _trap14
global _trap16
global _trap17
global _trap18

_trap0:
	push 0;
	push 0;
	jmp _SetupTrapCommon;
_trap1:
	push 0;
	push 1;
	jmp _SetupTrapCommon;
_trap2:
	push 0;
	push 2;
	jmp _SetupTrapCommon;
_trap3:
	push 0;
	push 3;
	jmp _SetupTrapCommon;
_trap4:
	push 0;
	push 4;
	jmp _SetupTrapCommon;
_trap5:
	push 0;
	push 5;
	jmp _SetupTrapCommon;
_trap6:
	push 0;
	push 6;
	jmp _SetupTrapCommon;
_trap7:
	push 0;
	push 7;
	jmp _SetupTrapCommon;
_trap8:
	push 8;
	jmp _SetupTrapCommon;
_trap9:
	push 0;
	push 9;
	jmp _SetupTrapCommon;
_trap10:
	push 10;
	jmp _SetupTrapCommon;
_trap11:
	push 11;
	jmp _SetupTrapCommon;
_trap12:
	push 12;
	jmp _SetupTrapCommon;
_trap13:
	push 13;
	jmp _SetupTrapCommon;
_trap14:
	push 14;
	jmp _SetupTrapCommon;
_trap16:
	push 0;
	push 16;
	jmp _SetupTrapCommon;
_trap17:
	push 17;
	jmp _SetupTrapCommon;
_trap18:
	push 0;push 18;
	jmp _SetupTrapCommon;

; IO Interrupts
global _trap32
global _trap33
global _trap34
global _trap35
global _trap36
global _trap37
global _trap38
global _trap39
global _trap40
global _trap41
global _trap42
global _trap43
global _trap44
global _trap45
global _trap46
global _trap47

_trap32:
	push 0
	push 32
	jmp _SetupTrapCommon
_trap33:
	push 0;
	push 33;
	jmp _SetupTrapCommon;
_trap34:
	push 0;
	push 34;
	jmp _SetupTrapCommon;
_trap35:
	push 0;
	push 35;
	jmp _SetupTrapCommon;
_trap36:
	push 0;
	push 36;
	jmp _SetupTrapCommon;
_trap37:
	push 0;
	push 37;
	jmp _SetupTrapCommon;
_trap38:
	push 0;
	push 38;
	jmp _SetupTrapCommon;
_trap39:
	push 0;
	push 39;
	jmp _SetupTrapCommon;
_trap40:
	push 0;
	push 40;
	jmp _SetupTrapCommon;
_trap41:
	push 0;
	push 41;
	jmp _SetupTrapCommon;
_trap42:
	push 0;
	push 42;
	jmp _SetupTrapCommon;
_trap43:
	push 0;
	push 43;
	jmp _SetupTrapCommon;
_trap44:
	push 0;
	push 44;
	jmp _SetupTrapCommon;
_trap45:
	push 0;
	push 45;
	jmp _SetupTrapCommon;
_trap46:
	push 0;
	push 46;
	jmp _SetupTrapCommon;
_trap47:
	push 0;
	push 47;
	jmp _SetupTrapCommon;

; System call
global _trap50

_trap50:push 0;push 50;jmp _SetupTrapCommon;


						; Bad Trap
						global 	_bad_trap;						
_BadTrap:				push 	0xffffffff;
						push 	0xffffffff;
						jmp		_SetupTrapCommon

						; All interrupt handler stubs jump here, which saves
						; the rest of the machine state and calls into HandleTrap.
						EXTERN _HandleTrap						
						global _SetupTrapCommon
_SetupTrapCommon:									
						pushad
						cld
						call _HandleTrap
						popad
						add esp, 8						
						iretd