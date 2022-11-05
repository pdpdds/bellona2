#include "windows.h"

#include "bidt.h"

DWORD dwGetIDTOffset( IDTStt *pIdt )
{
	DWORD dwR, dwTemp;

	dwR    = (DWORD)pIdt->wOffs0;
	dwTemp = (DWORD)pIdt->wOffs1;

	dwTemp = ( dwTemp << 16 );
	dwR += dwTemp;
}

DWORD dwSetIDTOffset( IDTStt *pIdt, dwOffs )
{
	DWORD dwR, dwTemp0, dwTemp1;

	dwR = dwGetIDTOffset( pIdt );

	dwTemp0 = dwOffs;
	dwTemp0 = dwTemp & 0xFFFF;

	dwTemp1 = dwOffs;
	dwTemp1 = (dwTemp1 >> 16);
	// 오프셋 하위와 상위를 설정한다.
	pIdt->wOffs0 = (UINT16)dwTemp0;
	pIdt->wOffs1 = (UINT16)dwTemp1;
	// 기존의 값을 리턴한다.
	return( dwR);
}
