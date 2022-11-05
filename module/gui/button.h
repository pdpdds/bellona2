#ifndef BELLONA2_GUI_BUTTON_20020525
#define BELLONA2_GUI_BUTTON_20020525

#define MAX_BUTTON_TEXT		64

// 아이콘 버튼 구조체
struct ButtonTag {
	GuiObjStt			obj;
	GraBuffStt			gb; 								// 그래픽 버퍼.
	WinStt				*pParentWin;						// 버튼이 속한 윈도우.
	DWORD				dwAttr;								// 속성
	DWORD				dwState;							// 상태
	DWORD				dwWinMesg;							// 눌렸을 때 전달될 메시지.
	DWORD				dwParamA, dwParamB;					// 전달될 메시지의 파러메터.
	int					nImgID;								// 이미지 ID
	struct ImageTag		*pImg;								// 아이콘 이미지
	char				szText[ MAX_BUTTON_TEXT + 1 ];		// 버튼의 글자.
	struct FontTag		*pFont;								// 버튼의 폰트.
	UINT16				wTextColor, wBkColor;				// 버튼의 색상.
};
typedef struct ButtonTag ButtonStt;

extern int 			kclose_button			( ButtonStt *pBtn );
extern int 			kflush_button_gb		( GraBuffStt *pGB, ButtonStt *pButton );
extern int			kdraw_button_gb 		( ButtonStt *pButton );
extern ButtonStt 	*kcreate_button			( WinStt	*pParentWin, 
											  int		nImgID, 	
											  char		*pText, 	
											  DWORD		dwAttr,
											  UINT16	wBackColor,
											  UINT16	wTextColor,
											  int		nFontID,
											  int		nX,
											  int		nY,
											  int		nH,
											  int		nV,
											  DWORD 	dwWinMesg,
											  DWORD 	dwParamA,
											  DWORD 	dwParamB	   );


extern ButtonStt 	*kcreate_button_ex		( WinStt	*pParentWin, 
							  				struct ImageTag	*pImg,
							  				char		*pText, 	
							  				DWORD 		dwAttr,
							 				UINT16		wBackColor,
							  				UINT16		wTextColor,
							  				int			nFontID,
							  				int			nX,
							  				int			nY,
							  				int			nH,
							  				int			nV,
							  				DWORD 		dwWinMesg,
							  				DWORD 		dwParamA,
							  				DWORD 		dwParamB		);
 
// create_button은 더이상 사용하지 않는다.  kcreate_button_ex로 완전 대체. 2003-12-10
//extern ButtonStt	*create_button			( struct WinTag *pParentWin, int nID, char *pText );

#endif
