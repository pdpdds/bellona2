#ifndef BELLONA_C_LIBRARY_HEADER_jj
#define BELLONA_C_LIBRARY_HEADER_jj

#include <bellona2.h>

#include <types.h> 
#include <stdio.h>
#include <stdlib.h>
#include <shmem.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <process.h>
#include <io.h>
#include <conio.h>
#include <mem.h>

#include <fileapi.h>
#include <ipc.h>
#include <uarea.h>	  
#include <dirent.h>

#include <etc.h>

extern int system_call( int nType, DWORD dwParam );
extern int syscall_stub( int nType, ... );

extern BELL_EXPORT void main_startup( DWORD dwMain, DWORD dwSigStruct );

#endif