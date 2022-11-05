#ifndef BELLONA2_BASIC_TYPES_Header
#define BELLONA2_BASIC_TYPES_Header

#define BELL_EXPORT __declspec( dllexport )

#define UNALIGNED
#define NTAPI
#define NULL    ((void *)0)

#define _INTSIZEOF(n)   ( (sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1) )
#define va_start(ap,v)  ( ap = (va_list)&v + _INTSIZEOF(v) )
#define va_arg(ap,t)    ( *(t *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)) )
#define va_end(ap)      ( ap = (va_list)0 )

typedef char *  va_list;

typedef unsigned int	size_t;
typedef	char			boolean;
typedef	char			BOOLEAN;
typedef	char			CHAR;
typedef unsigned short	UINT16;
typedef void			VOID;
typedef short    int	SHORT;
typedef long			LONG;
typedef unsigned char	BYTE;
typedef unsigned short	WORD;
typedef unsigned short	WCHAR;
typedef unsigned long	ULONG;
typedef unsigned char	UCHAR;
typedef unsigned long	DWORD;
typedef unsigned long	time_t;

typedef void			*PVOID;
typedef char			*PCHAR;
typedef short    int	*PSHORT;
typedef long			*PLONG;
typedef unsigned char	*PBYTE;
typedef unsigned short	*PWORD;
typedef unsigned short	*PWCHAR;
typedef unsigned long	*PULONG;
typedef unsigned char	*PUCHAR;
typedef unsigned long	*PDWORD;

typedef DWORD COLORREF;
//#define RGB32(r,g,b,d)			((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8)) | (((DWORD)(BYTE)(b))<<16)) | (((DWORD)(BYTE)(d))<< 24) )
#define RGB32(r,g,b)			((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8)) | (((DWORD)(BYTE)(b))<<16)) | (((DWORD)(BYTE)(0))<< 24) )
#define RGB16(r,g,b)			(UINT16)( ((UINT16)((UINT16)b >>3) & (BYTE)0x1F) | ((((UINT16)g>>2)&(UINT16)0x3F) << 5) | ((((UINT16)r>>3)&(UINT16)0x1F) << 11) )

#endif

