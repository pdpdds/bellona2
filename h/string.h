#ifndef BELLONA_LIB_STRING_HEADER
#define BELLONA_LIB_STRING_HEADER

extern BELL_EXPORT int  strlen( void*   pS );
extern BELL_EXPORT char *strcpy( char *pDest, char *pSrc );
extern BELL_EXPORT char *strcat( char *pS, char *pT );
extern BELL_EXPORT int  strcmp( char *pA, char *pB );
extern BELL_EXPORT int  strcmpi( char *pA, char *pB );

#endif