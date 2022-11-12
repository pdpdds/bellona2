#pragma once

#include "x86arch.h"
#include "PEImage.h"
#include "MultiBoot.h"
#include "MultibootUtil.h"
#include "pmm.h"
#include "vmm.h"
#include "GDT.h"
#include "IDT.h"
#include "FPU.h"
#include "PIC.h"
#include "TSS.h"
#include "string.h"
#include "Constants.h"
#include "PIT.h"
#include "Interrupt.h"
#include "intrinsic.h"
#include "LoadDLL.h"

extern "C" void InitCRT();
void StartKernel(uint32_t entryPoint, uint32_t stackPointer, uint32_t param, uint32_t isDll);
void InstallTrapHandler();
bool Boot32BitKernel(multiboot_info_t* mb_info);


