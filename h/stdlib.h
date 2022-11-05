#ifndef BELLONA_STDLIB_HEADER
#define BELLONA_STDLIB_HEADER

extern BELL_EXPORT int  pause			();
extern BELL_EXPORT int	abs				( int nValue );
extern BELL_EXPORT int  delay			( DWORD dwMiliSecond );
extern BELL_EXPORT int	sprintf			( char *pS, char *pFmt, ... );

extern BELL_EXPORT void srand			( DWORD dwS );
extern BELL_EXPORT void free			( void *pAddr );
extern BELL_EXPORT void exit			( int nExitCode );
extern BELL_EXPORT void Sleep			( DWORD dwMiliSecond );
extern BELL_EXPORT void *malloc			( unsigned long dwSize );
extern BELL_EXPORT void *memcpy			( void* pD, void *pS, long lSize );
extern BELL_EXPORT void *memset			( void *pD, unsigned char byTe, long lSize );

extern BELL_EXPORT DWORD rand			();

#endif