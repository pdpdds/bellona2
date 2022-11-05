#ifndef FDD35_BLKDEV_HEADER_jj
#define FDD35_BLKDEV_HEADER_jj

#define FDD35_MAX_TRACK	80
#define FDD35_MAX_SIDE	2

// TRACK ACCESS FLAG
//////////////////////////////////////////
#define FDD35_TRACK_ACCESS	0			//
#define FDD35_TRACK_DIRTY	1			//
//////////////////////////////////////////

typedef struct FDTrackTag {
	int  nFlag;
	char *pBuff;
};
typedef struct FDTrackTag FDDTrackStt;

typedef struct FDD35Tag {
	int			nTotalBuffered;			// 버퍼링된 트랙의 개수
	FDDTrackStt	track[ FDD35_MAX_SIDE ][ FDD35_MAX_TRACK ];
};
typedef struct FDD35Tag FDD35Stt;

extern int init_fdd35_driver();
extern int close_fdd35_driver();

#endif
