#ifndef BELLONA2_GUI_WIN_CONST_20020523
#define BELLONA2_GUI_WIN_CONST_20020523

#include <grxdef.h>

// ������ Ÿ��Ʋ�� ��Ʈ�� ����
#define MAX_WINDOW_TITLE		64

#define WALL_BK_COLOR			RGB16( 127, 127, 255 )
#define _3D_BORDER_DIMM			RGB16( 200, 180, 245 ) 
#define _3D_BORDER_LIGHT        RGB16( 242, 218, 235 ) 
#define _3D_BORDER_DK			RGB16( 115,  76,  84 )

// ������ �޽��� �� �Լ�.
typedef DWORD (*GUIOBJ_FUNC)( struct GuiObjTag *pObj, DWORD dwWMesgID, DWORD dwParamA, DWORD dwParamB );
typedef struct {
	DWORD			dwID;
	GUIOBJ_FUNC		pPreFunc;
	GUIOBJ_FUNC 	pPostFunc;
} GuiObjFuncStt;

#define GUI_OBJ_MAGIC						0xF555

// GUI OBJECT Ÿ��
#define GUI_OTYPE_WINDOW					0x0001
#define GUI_OTYPE_BUTTON					0x0002
#define GUI_OTYPE_MENUENT					0x0003

//////////////////////////////////////////////////
//----------- ��Ʈ���� ���� ����.-----------------
#define CONTROL_STATE_FOCUSED			0x00000001		// ��Ŀ���� ���� ����.
#define CONTROL_STATE_ON_MOUSE			0x00000002		// ���콺�� ���� �ö� ����.

//----------- ��ư�� ����.------------------------
#define BTN_STATE_PRESSED				0x00010000		// ��ư�� ���� ����.
//////////////////////////////////////////////////


typedef struct GuiObjTag {
	UINT16				wMagic;
	UINT16				wType;
	RectStt				r;					// GuiObj�� ��ġ�� ũ��.
	GuiObjFuncStt		*pFuncArray;
	struct GuiObjTag	*pOwner;
	struct GuiObjTag	*pPre,   *pNext;
	struct GuiObjTag	*pStart, *pEnd;					  
	struct GuiObjTag	*pFocus;			// ���콺 Ŭ���� ���¿��� Focus�� ��� �ִ� Object
	struct GuiObjTag    *pCat;				// ��(Mouse)�� ��� �ִ� �����.
} GuiObjStt;			  

#endif

