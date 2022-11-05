#ifndef GRAPHIX_DEF_HEADEr_jj
#define GRAPHIX_DEF_HEADEr_jj

#define MAX_PREDEF_WIN_STYLE  	3
///// predefined window style ///
#define WSTYLE_SIMPLE			1
#define WSTYLE_FLAT 			2
#define WSTYLE_FRAMEW			3
/////////////////////////////////

// k_3d_look�� nOuter �ʵ忡 �� ��.
#define LOOK_3D_IN			0
#define LOOK_3D_OUT 		1

// k_3d_look�� nType �ʵ忡 �� ��.
#define LOOK_3D_NORMAL		0	 // ����
#define LOOK_3D_DEPRESSED	1	 // ����
#define LOOK_3D_PRESSED 	2	 // ����

// Window Message Handler Return Value
#define WMHRV_ABORT 			0x20000000	// Style Window�� �޽��� �ڵ鷯�� �������� ����.
#define WMHRV_ERROR 			0x40000000	// ���� Bit(30)
#define WMHRV_CONTINUE			0x80000000	// Style Window�� �޽��� �ڵ鷯�� ������ ��. Bit(31)

//----------- ��Ʈ���� ���� �Ӽ�.---------------//
#define CONTROL_ATTR_BORDER 			0x00000004		// �׻� BORDER�� �׸���.
#define CONTROL_ATTR_BORDER_ON_MOUSE	0x00000008		// ���콺�� ���� �ö� ��쿡�� BORDER�� �׸���.
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

// WIN.C�� WMESGSTR���� �߰��� ��. 
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
	WMESG_REMAKE,			// �ش� �������� ������ ������ remake�Ѵ�.
	WMESG_WIN_RESIZE,
	
	MAX_WMESG_FUNC
} WMESG_TAG;

// �޽��� ����ü
typedef struct {
	DWORD	dwID;
	DWORD	dwParamA;
	DWORD	dwParamB;
} WMesgStt;

typedef struct RectTag{
	int	nX, nY, nH, nV;
} RectStt;


// ������ �޽��� �� �Լ�.
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
	int 				nPainIndex; 	// ICON, CURSOR�� ��� ���� �̷�� �׷� �ε���
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

	// ����� ���� ���ҽ��� �̸�
	int 			nTotalResName;
	ResNameStt		resname[MAX_RES_NAME];
} WinResStt;

// ȭ�鿡 �ٷ� �Ѹ� �� �ִ� �̹���
typedef struct ImageTag {
	int 	nBPP;
	int 	nH;
	int 	nV;
	int 	nSize;					// ����� ������ ��ü ũ��
	int 	nMaskSize;				// ����ũ ũ��.
	int 	nColorSize; 			// �÷� ������ ũ��.
	BYTE	*pMask;
	union {
		unsigned short int	*pW;
		DWORD				*pD;
	} b;
} ImageStt;

#define ROTYPE_TBICON	0x50001001

// ���ҽ� ������Ʈ ����ü.
typedef struct {
	DWORD dwType;
	DWORD dwPtr;
	DWORD dwData;
} ResObjStt;

// ����� ���α׷����� ��ũ�� ������ ���� ���� ����ü.
typedef struct ScrInfoTag {
	int 	nBPP, nH, nV;
} ScrInfoStt;

#endif	
