#ifndef BELLONA2_LIB_DIRENT_HEADER
#define BELLONA2_LIB_DIRENT_HEADER

#define MAX_PATH_SIZE	260

struct dirent {
	/** ���� �̸� */
	char 	d_name[ MAX_PATH_SIZE ];   
	/** ���� ������ */
	int	 	d_size;					
	/** ���� �Ӽ� */
	int		d_attr;	
	/** ���� �ð� */					
	time_t	t_access;
	/** ��� �ð� */					
	time_t	t_write;
	/** ���� �ð� */					
	time_t	t_create;
};

#define DIR_TYPE_FAT	1

typedef struct DIRTag {
	/** ���� �ڵ� (���������� ���) */
	long 			lHandle;
	/** ���� �ý��� Ÿ�� */
	unsigned short	wDirType;
	/** ���丮 ��Ʈ�� ����. (readdir �Լ����� ���������� ���) */
	struct dirent	ent;
	/** ent�� valid�� �������� ���� */
	BYTE			byValidEnt;
} DIR;

extern BELL_EXPORT int closedir( DIR *pDir );
extern BELL_EXPORT DIR *opendir( char *pPath );
extern BELL_EXPORT struct dirent *readdir( DIR *pDir );
extern BELL_EXPORT char *get_abs_path( char *pRPath, int nRSize, char *pPath );

#endif

