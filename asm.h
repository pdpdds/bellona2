#ifndef BELLONA2_ASM_CODE_HEADER_oh
#define BELLONA2_ASM_CODE_HEADER_oh

#include "gdt.h"

extern void  vAsmInitFPU		();
extern void  vEnableInterrupt	();
extern void  vSendEOI			( DWORD dwNo );							// EOI Signal을 보낸다.
extern void  vAsmSetKBDLed		( DWORD dwCode );
extern void  vSetTimerInterval	( int nInterval );
extern void  vReprogramPIC		( UCHAR *pData );
extern void  vResetGDT			( DescriptorStt *pGDT, GDTRStt *pGDTR, DWORD dwMaxGDT );
extern void  vReadPort			( DWORD dwPort,  UCHAR *pByTe );			// 포트에서 1 바이트 읽어들이기
extern void  vWritePort			( DWORD dwPort, DWORD dwByte );			// 포트에 1 바이트를 쓴다.
extern void  vWritePortWord		( DWORD dwPort, DWORD dwX );			// 포트에 2 바이트를 쓴다.
extern void  vWritePortDword	( DWORD dwPort, DWORD dwX );		// 포트에 4 바이트를 쓴다.
extern void  vReadPortWord		( DWORD dwPort,  UINT16 *pWord );		// 포트에서 2 바이트 읽어들이기
extern void  vReadPortDword		( DWORD dwPort,  DWORD *pDword );	// 포트에서 4 바이트 읽어들이기
extern void  vReadPortMultiWord	( DWORD dwPort, UINT16 *pWord, DWORD dwWords );		

extern void  vRebootSystem();

#endif

