#ifndef BELLONA_LIB_STDIO_HEADER
#define BELLONA_LIB_STDIO_HEADER

#define HANDLE_STD_IN		0
#define HANDLE_STD_OUT		1
#define HANDLE_STD_ERR		2

typedef struct TTimeTag {
	int nYear;  // 년
	int nMon;	// 월
	int nDay;	// 일
	int nWeek;  // 요일
	int nHour;	// 시
	int nMin;	// 분
	int nSec;	// 초
} TTimeStt;

extern BELL_EXPORT int getch();
extern BELL_EXPORT int kbhit();
extern BELL_EXPORT int printf( char *pFormat, ... );
extern BELL_EXPORT int get_time( TTimeStt *pT );

#endif

