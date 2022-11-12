#pragma once
#include "MultiBoot.h"
#include "string.h"

uint32_t GetTotalMemory(multiboot_info* bootinfo);
uint32_t GetModuleEnd(multiboot_info* bootinfo);
Module* FindModule(multiboot_info_t* pInfo, const char* szFileName);
uint32_t GetModuleSize(Module* pModule);
uint32_t GetFileSize(multiboot_info* bootinfo, char* filename);
