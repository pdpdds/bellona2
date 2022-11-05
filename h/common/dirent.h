#ifndef BELLONA2_LIB_DIRENT_HEADER
#define BELLONA2_LIB_DIRENT_HEADER

#define MAX_PATH_SIZE	260

struct dirent {
	/** 파일 이름 */
	char 	d_name[ MAX_PATH_SIZE ];   
	/** 파일 사이즈 */
	int	 	d_size;					
	/** 파일 속성 */
	int		d_attr;	
	/** 접근 시간 */					
	time_t	t_access;
	/** 기록 시간 */					
	time_t	t_write;
	/** 생성 시간 */					
	time_t	t_create;
};

#define DIR_TYPE_FAT	1

typedef struct DIRTag {
	/** 파일 핸들 (내부적으로 사용) */
	long 			lHandle;
	/** 파일 시스템 타입 */
	unsigned short	wDirType;
	/** 디렉토리 엔트리 정보. (readdir 함수에서 내부적으로 사용) */
	struct dirent	ent;
	/** ent가 valid한 것인지의 여부 */
	BYTE			byValidEnt;
} DIR;

extern BELL_EXPORT int closedir( DIR *pDir );
extern BELL_EXPORT DIR *opendir( char *pPath );
extern BELL_EXPORT struct dirent *readdir( DIR *pDir );
extern BELL_EXPORT char *get_abs_path( char *pRPath, int nRSize, char *pPath );

#endif

