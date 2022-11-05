#ifndef DIGIT_HEADER_jj
#define DIGIT_HEADER_jj

#ifdef WIN32TEST
	#include <windows.h>
	#include <stdlib.h>
	#include <stdio.h>
	#include <io.h>
#endif
#ifdef BELLONA2
	#include <types.h>
	#include <util.h>
#endif

extern unsigned long atoul( char *pS );

#endif