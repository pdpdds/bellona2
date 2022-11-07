#ifndef BELLONA2_ASM_CODE_HEADER_oh
#define BELLONA2_ASM_CODE_HEADER_oh

#include "gdt.h"

extern void  vAsmInitFPU		();
extern void  vEnableInterrupt	();
extern void  vSendEOI			( DWORD dwNo );							// EOI Signal�� ������.
extern void  vAsmSetKBDLed		( DWORD dwCode );
extern void  vSetTimerInterval	( int nInterval );
extern void  vReprogramPIC		( UCHAR *pData );
extern void  vResetGDT			( DescriptorStt *pGDT, GDTRStt *pGDTR, DWORD dwMaxGDT );
extern void  vReadPort			( DWORD dwPort,  UCHAR *pByTe );			// ��Ʈ���� 1 ����Ʈ �о���̱�
extern void  vWritePort			( DWORD dwPort, DWORD dwByte );			// ��Ʈ�� 1 ����Ʈ�� ����.
extern void  vWritePortWord		( DWORD dwPort, DWORD dwX );			// ��Ʈ�� 2 ����Ʈ�� ����.
extern void  vWritePortDword	( DWORD dwPort, DWORD dwX );		// ��Ʈ�� 4 ����Ʈ�� ����.
extern void  vReadPortWord		( DWORD dwPort,  UINT16 *pWord );		// ��Ʈ���� 2 ����Ʈ �о���̱�
extern void  vReadPortDword		( DWORD dwPort,  DWORD *pDword );	// ��Ʈ���� 4 ����Ʈ �о���̱�
extern void  vReadPortMultiWord	( DWORD dwPort, UINT16 *pWord, DWORD dwWords );		

extern void  vRebootSystem();

#endif

