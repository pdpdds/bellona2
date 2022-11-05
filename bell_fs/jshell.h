#ifndef JSHELL_HEADER_jj
#define JSHELL_HEADER_jj

#ifdef WIN32TEST
	#include <windows.h>
	#include <stdlib.h>
	#include <stdio.h>
	#include <io.h>
	#include <string.h>
	#include <conio.h>
	#include <sys/stat.h>
	#include <fcntl.h>
#endif

#include "wkeycode.h"

extern int jshell( char *pScript );


#endif
