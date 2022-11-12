#include "log.h"
#include "stdio.h"
#include "stdarg.h"
#include "string.h"
#include "BootConsole.h"

using namespace BootConsole;

static uint8_t g_log_level = LOG_DEBUG;
void LOG(uint8_t loglevel, char* fmt, ...)
{
	if (loglevel < g_log_level)
		return;

	va_list arglist;
	va_start(arglist, fmt);

	vPrint(fmt, arglist);

	va_end(arglist);

	if (loglevel == LOG_FATAL)
	{
		for (;;);
	}
}

void SetLogLevel(uint8_t loglevel)
{
	g_log_level = loglevel;
}

void InitLogManager(uint8_t loglevel)
{
	InitConsole();
	SetLogLevel(loglevel);
}
