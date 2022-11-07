#ifndef BELLONA2_V86_HEADER_jj
#define BELLONA2_V86_HEADER_jj

#define V86PARAM_MAGIC			0x1128
#define V86LIB_ID				"V86LIB"

//////////////////////////////////////////
#define V86FUNC_LINES50			1		//
#define V86FUNC_GET_VESA_INFO	2		// 2002-05-02
#define V86FUNC_GET_MODE_INFO   3		// 2002-05-02
#define V86FUNC_SET_VESA_MODE	4		// 2002-05-06
#define V86FUNC_LINES25			5		// 2002-05-06 Return to the standard text video mode (25*80)
//////////////////////////////////////////

#define V86_ERR_OK				0
#define V86_ERR_INVALID_MAGIC	1
#define V86_ERR_UNKNOWN_FUNC	2

typedef struct {
	UINT16	wMagic;				// 구조체의 valid flag
	UINT16	wFunc;				// 펑션 번호
	UINT16	wResult;			// V86 펑션을 실행한 결과
	DWORD	dwParam;			// 펑션에 대한 파러메터
}
V86ParamStt;

extern BELL_EXPORT DWORD lines_xx( int nLine );

extern void  *get_v86_buff();
extern void  v86_system_call();
extern void  int_10h_emulator();
extern void  int_1Ah_emulator();
extern void  int_6Dh_emulator();
extern DWORD call_to_v86_thread();
extern int   v86lib_thread( void *pPram );
extern int   load_v86_lib( char *pFileName );
extern int   set_v86_param( UINT16 wFunc, DWORD dwParam );

#endif