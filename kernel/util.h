#ifndef BELLONA2_UTIL_HEADER_oh
#define BELLONA2_UTIL_HEADER_oh

extern int 		memcmp					( void* pD, void *pS, int lSize );
extern int   	is_char					( char ch );
extern int		vsprintf				( char *buffer, char *format, va_list argptr );

extern char  	*pGetPureFileName		( char *pS );
extern char  	*get_pure_filename		( char *pS );
extern DWORD 	segoffset_to_offset32	( DWORD dwSegOffs );
extern void 	i64_shr					( __int64 *pLDW, int nC );

extern BELL_EXPORT int	 is_digit		( char *pS );
extern BELL_EXPORT void  uppercase		( char *pS );
extern BELL_EXPORT void  lowercase		( char *pS );
extern BELL_EXPORT DWORD dwHexValue 	( char *pS );
extern BELL_EXPORT DWORD dwDecValue 	( char *pS );

#endif
