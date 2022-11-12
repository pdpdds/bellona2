#pragma once
#include "MultibootUtil.h"

enum class PE_TYPE
{
	PE_TYPE_INVALID,
	PE_TYPE_FILE_NOT_FOUND,
	PE_TYPE_32,
	PE_TYPE_64,
};

PE_TYPE CheckPEFormat(Module* pKernel);
bool IsDll(void* image);

uint32_t FindPE64Entry(char* buf);
