#ifndef BELLONA2_GUI_BUTTON_20020525
#define BELLONA2_GUI_BUTTON_20020525

#define MAX_BUTTON_TEXT		64

// ������ ��ư ����ü
struct ButtonTag {
	GuiObjStt			obj;
	GraBuffStt			gb; 								// �׷��� ����.
	WinStt				*pParentWin;						// ��ư�� ���� ������.
	DWORD				dwAttr;								// �Ӽ�
	DWORD				dwState;							// ����
	DWORD				dwWinMesg;							// ������ �� ���޵� �޽���.
	DWORD				dwParamA, dwParamB;					// ���޵� �޽����� �ķ�����.
	int					nImgID;								// �̹��� ID
	struct ImageTag		*pImg;								// ������ �̹���
	char				szText[ MAX_BUTTON_TEXT + 1 ];		// ��ư�� ����.
	struct FontTag		*pFont;								// ��ư�� ��Ʈ.
	UINT16				wTextColor, wBkColor;				// ��ư�� ����.
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
 
// create_button�� ���̻� ������� �ʴ´�.  kcreate_button_ex�� ���� ��ü. 2003-12-10
//extern ButtonStt	*create_button			( struct WinTag *pParentWin, int nID, char *pText );

#endif
