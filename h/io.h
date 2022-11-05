#ifndef BELLONA2_LIB_IO_HEADER_jj
#define BELLONA2_LIB_IO_HEADER_jj

#include "types.h"
#include "fileapi.h"

#define SEEK_SET   0
#define SEEK_CUR   1
#define SEEK_END   2

#define O_RDONLY   0
#define O_RDWR     1
#define O_CREAT    2
#define _O_RDONLY  O_RDONLY
#define _O_RDWR    O_RDWR  
#define _O_CREAT   O_CREAT 


#define S_IREAD    0
#define S_IWRITE   1

extern BELL_EXPORT int open( char *pFileName, int nOFlag, ... );
extern BELL_EXPORT int close( int nHandle );
extern BELL_EXPORT int read( int nHandle, char *pBuff, int nSize );
extern BELL_EXPORT int write( int nHandle, char *pBuff, int nSize );
extern BELL_EXPORT long lseek( int nHandle, long offset, int nOrigin );
extern BELL_EXPORT int rename( char *pOldName, char *pNewName );

#endif