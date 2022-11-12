#include "Interrupt.h"
#include "log.h"
#include "idt.h"


extern "C"
{
	void trap0();
	void trap1();
	void trap2();
	void trap3();
	void trap4();
	void trap5();
	void trap6();
	void trap7();
	void trap8();
	void trap9();
	void trap10();
	void trap11();
	void trap12();
	void trap13();
	void trap14();
	void trap16();
	void trap17();
	void trap18();
	void trap32();
	void trap33();
	void trap34();
	void trap35();
	void trap36();
	void trap37();
	void trap38();
	void trap39();
	void trap40();
	void trap41();
	void trap42();
	void trap43();
	void trap44();
	void trap45();
	void trap46();
	void trap47();
	void trap50();

	void HandleTrap(InterruptFrame iframe);
};

enum
{
	eHandleDivideByZero = 0,
	eHandleSingleStepTrap = 1,
	eHandleNMITrap = 2,
	eHandleBreakPointTrap = 3,
	eHandleOverflowTrap = 4,
	eHandleBoundsCheckFault = 5,
	eHandleInvalidOpcodeFault = 6,
	eHandleNoDeviceFault = 7,
	eHandleDoubleFaultAbort = 8,
	ekHandleInvalidTSSFault = 10,
	eHandleSegmentFault = 11,
	eHandleStackFault = 12,
	eHandleGeneralProtectionFault = 13,
	eHandlePageFault = 14,
	eHandlefpu_fault = 16,
	eHandleAlignedCheckFault = 17,
	eHandleMachineCheckAbort = 18,
	eHandleSIMDFPUFault = 19,
};

void InstallTrapHandler()
{
	SetInterruptVector(eHandleDivideByZero, (void(__cdecl&)(void))trap0);
	SetInterruptVector(eHandleSingleStepTrap, (void(__cdecl&)(void))trap1);
	SetInterruptVector(eHandleNMITrap, (void(__cdecl&)(void))trap2);
	SetInterruptVector(eHandleBreakPointTrap, (void(__cdecl&)(void))trap3);
	SetInterruptVector(eHandleOverflowTrap, (void(__cdecl&)(void))trap4);
	SetInterruptVector(eHandleBoundsCheckFault, (void(__cdecl&)(void))trap5);
	SetInterruptVector(eHandleInvalidOpcodeFault, (void(__cdecl&)(void))trap6);
	SetInterruptVector(eHandleNoDeviceFault, (void(__cdecl&)(void))trap7);
	SetInterruptVector(eHandleDoubleFaultAbort, (void(__cdecl&)(void))trap8);
	SetInterruptVector(9, (void(__cdecl&)(void))trap9);
	SetInterruptVector(ekHandleInvalidTSSFault, (void(__cdecl&)(void))trap10);
	SetInterruptVector(eHandleSegmentFault, (void(__cdecl&)(void))trap11);
	SetInterruptVector(eHandleStackFault, (void(__cdecl&)(void))trap12);
	SetInterruptVector(eHandleGeneralProtectionFault, (void(__cdecl&)(void))trap13);
	SetInterruptVector(eHandlePageFault, (void(__cdecl&)(void))trap14);
	SetInterruptVector(eHandlefpu_fault, (void(__cdecl&)(void))trap16);
	SetInterruptVector(eHandleAlignedCheckFault, (void(__cdecl&)(void))trap17);
	SetInterruptVector(eHandleMachineCheckAbort, (void(__cdecl&)(void))trap18);
}

extern "C" void HandleTrap(InterruptFrame iframe)
{
	LOG(LOG_FATAL, "Trap Handler 0x%x 0x%x 0x%x\n", iframe.vector, iframe.user_esp, iframe.user_ss);
}