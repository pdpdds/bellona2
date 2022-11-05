#ifndef BELLONA2_GUI_WIN_CONST_20020523
#define BELLONA2_GUI_WIN_CONST_20020523

#include <grxdef.h>

// 윈도우 타이틀의 스트링 길이
#define MAX_WINDOW_TITLE		64

#define WALL_BK_COLOR			RGB16( 127, 127, 255 )
#define _3D_BORDER_DIMM			RGB16( 200, 180, 245 ) 
#define _3D_BORDER_LIGHT        RGB16( 242, 218, 235 ) 
#define _3D_BORDER_DK			RGB16( 115,  76,  84 )

// 윈도우 메시지 별 함수.
typedef DWORD (*GUIOBJ_FUNC)( struct GuiObjTag *pObj, DWORD dwWMesgID, DWORD dwParamA, DWORD dwParamB );
typedef struct {
	DWORD			dwID;
	GUIOBJ_FUNC		pPreFunc;
	GUIOBJ_FUNC 	pPostFunc;
} GuiObjFuncStt;

#define GUI_OBJ_MAGIC						0xF555

// GUI OBJECT 타입
#define GUI_OTYPE_WINDOW					0x0001
#define GUI_OTYPE_BUTTON					0x0002
#define GUI_OTYPE_MENUENT					0x0003

//////////////////////////////////////////////////
//----------- 컨트롤의 공통 상태.-----------------
#define CONTROL_STATE_FOCUSED			0x00000001		// 포커스를 가진 상태.
#define CONTROL_STATE_ON_MOUSE			0x00000002		// 마우스가 위에 올라간 상태.

//----------- 버튼의 상태.------------------------
#define BTN_STATE_PRESSED				0x00010000		// 버튼이 눌린 상태.
//////////////////////////////////////////////////


typedef struct GuiObjTag {
	UINT16				wMagic;
	UINT16				wType;
	RectStt				r;					// GuiObj의 위치와 크기.
	GuiObjFuncStt		*pFuncArray;
	struct GuiObjTag	*pOwner;
	struct GuiObjTag	*pPre,   *pNext;
	struct GuiObjTag	*pStart, *pEnd;					  
	struct GuiObjTag	*pFocus;			// 마우스 클릭된 상태에서 Focus를 잡고 있는 Object
	struct GuiObjTag    *pCat;				// 쥐(Mouse)를 잡고 있는 고양이.
} GuiObjStt;			  

#endif

