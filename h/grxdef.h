#ifndef GRAPHIX_DEF_HEADEr_jj
#define GRAPHIX_DEF_HEADEr_jj

#define MAX_PREDEF_WIN_STYLE  	3
///// predefined window style ///
#define WSTYLE_SIMPLE			1
#define WSTYLE_FLAT 			2
#define WSTYLE_FRAMEW			3
/////////////////////////////////

// k_3d_look의 nOuter 필드에 들어갈 값.
#define LOOK_3D_IN			0
#define LOOK_3D_OUT 		1

// k_3d_look의 nType 필드에 들어갈 값.
#define LOOK_3D_NORMAL		0	 // 보통
#define LOOK_3D_DEPRESSED	1	 // 볼록
#define LOOK_3D_PRESSED 	2	 // 오목

// Window Message Handler Return Value
#define WMHRV_ABORT 			0x20000000	// Style Window의 메시지 핸들러를 수행하지 말것.
#define WMHRV_ERROR 			0x40000000	// 에러 Bit(30)
#define WMHRV_CONTINUE			0x80000000	// Style Window의 메시지 핸들러를 수행할 것. Bit(31)

//----------- 컨트롤의 공통 속성.---------------//
#define CONTROL_ATTR_BORDER 			0x00000004		// 항상 BORDER를 그린다.
#define CONTROL_ATTR_BORDER_ON_MOUSE	0x00000008		// 마우스가 위에 올라간 경우에만 BORDER를 그린다.
//----------------------------------------------//
typedef enum {
	COLOR_INDEX_MENU_DK_TEXT = 0,
	COLOR_INDEX_MENU_LT_TEXT	,
	COLOR_INDEX_MENU_BACK		,
	COLOR_INDEX_MENU_DK_BACK	,
	COLOR_INDEX_MENU_DK 		,
	COLOR_INDEX_MENU_LT 		,
	//.........................//
	END_OF_COLOR_INDEX
} SYS_COLOR_TAG;

// WIN.C의 WMESGSTR에도 추가할 것. 
typedef enum {
	WMESG_UNKNOWN = 0,
	WMESG_CREATE = 1,
	WMESG_CLOSE,
	WMESG_DESTROY,
	WMESG_PAINT,   
	WMESG_LBTN_DN,
	WMESG_LBTN_UP,
	WMESG_RBTN_DN,
	WMESG_RBTN_UP,
	WMESG_MOUSE_MOVE_OUT,
	WMESG_MOUSE_MOVE_IN, 
	WMESG_MINIMIZE,
	WMESG_MAXIMIZE,
	WMESG_MOUSE_MOVE,
	WMESG_WIN_MOVE,
	WMESG_CONTROL,
	WMESG_MODAL,
	WMESG_SHOW,
	WMESG_TIMER,
	WMESG_REMAKE,			// 해당 윈도우의 지정된 영역을 remake한다.
	WMESG_WIN_RESIZE,
	
	MAX_WMESG_FUNC
} WMESG_TAG;

// 메시지 구조체
typedef struct {
	DWORD	dwID;
	DWORD	dwParamA;
	DWORD	dwParamB;
} WMesgStt;

typedef struct RectTag{
	int	nX, nY, nH, nV;
} RectStt;


// 윈도우 메시지 별 함수.
typedef DWORD (*WMESG_FUNC)( struct WinTag *pWin, DWORD dwWMesgID, DWORD dwParamA, DWORD dwParamB );
typedef struct WMFuncTag{
	DWORD			dwID;
	WMESG_FUNC		pFunc;
} WMFuncStt;

#define MAX_WIN_RES 	64
#define MAX_RES_NAME	4

typedef struct {
	unsigned short int	wType;
	DWORD				dwAddr;
	DWORD				dwSize;
	unsigned short int	wID;
	int 				nPainIndex; 	// ICON, CURSOR의 경우 쌍을 이루는 그룹 인덱스
} WinResEntStt;

typedef struct {
	unsigned short int	wType;
	char				szName[32];
} ResNameStt;

typedef struct {
	int 			nTotal;
	WinResEntStt	ent[ MAX_WIN_RES ];
	int 			nCursorIndex;
	int 			nIconIndex;
	int 			nGroupCursorIndex;
	int 			nTotalCursor;
	int 			nGroupIconIndex;
	int 			nTotalIcon;

	// 사용자 정의 리소스의 이름
	int 			nTotalResName;
	ResNameStt		resname[MAX_RES_NAME];
} WinResStt;

// 화면에 바로 뿌릴 수 있는 이미지
typedef struct ImageTag {
	int 	nBPP;
	int 	nH;
	int 	nV;
	int 	nSize;					// 헤더를 포함한 전체 크기
	int 	nMaskSize;				// 마스크 크기.
	int 	nColorSize; 			// 컬러 데이터 크기.
	BYTE	*pMask;
	union {
		unsigned short int	*pW;
		DWORD				*pD;
	} b;
} ImageStt;

#define ROTYPE_TBICON	0x50001001

// 리소스 오브젝트 구조체.
typedef struct {
	DWORD dwType;
	DWORD dwPtr;
	DWORD dwData;
} ResObjStt;

// 사용자 프로그램에서 스크린 정보를 얻어가기 위한 구조체.
typedef struct ScrInfoTag {
	int 	nBPP, nH, nV;
} ScrInfoStt;

#endif	
