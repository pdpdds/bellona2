#ifndef BELLONA2_APP_LIB_IPC_HEADER_jj
#define BELLONA2_APP_LIB_IPC_HEADER_jj

extern BELL_EXPORT int	 sem_close	( DWORD dwSema );
extern BELL_EXPORT int   sem_post	( DWORD dwSema, DWORD dwOption );
extern BELL_EXPORT int   sem_wait	( DWORD dwSema );

extern BELL_EXPORT DWORD sem_create	( char *pName, int nMaxCounter, DWORD dwAttrib );
extern BELL_EXPORT DWORD sem_open	( char *pName );

#endif