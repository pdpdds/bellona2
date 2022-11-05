#ifndef USH_HEADER_jj
#define USH_HEADER_jj

#ifdef WIN32
	#include <wtypes.h>	  // h/w32
#endif

// h/
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <conio.h>
#include <process.h>

// h/common/
#include <env.h>
#include <funckey.h>
#include <uarea.h>
#include <fileapi.h>
#include <etc.h>
#include <dirent.h>

#ifndef WIN32
	#include <unistd.h>
#endif
#ifdef WIN32
	#include <direct.h>
	#include <wtypes.h>	  // h/w32
#endif

extern void print_version();
extern int interactive_mode();
extern int list_directory( char *pPath, char *pOption );

#endif
