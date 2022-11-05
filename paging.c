#include "bellona2.h"

DWORD *pAllocPageTable();

// allocate one physical page
DWORD dwAllocPhysPage()
{
	int		nI;
	UCHAR	*pTbl;

	pTbl = bell.pPhysRefTbl;

	// search after 64K 
	for( nI = 16; nI < bell.nPhysRefSize; nI++ )
	{
		// if the address is video memory then next search is started at 1.5M area
		if( nI == ( (int)0xA0000 / 4096 ) )
			nI = ( (int)0x180000 / 4096 );

		if( pTbl[nI] == 0 )
		{
			pTbl[nI]++;		// increase counter
			return( (DWORD)( (DWORD)nI * (DWORD)4096 ) );
		}
	}
	return( 0 );	// allocation failed
}

// 물리 페이지를 해제한다.
int nFreePage( DWORD dwPhysAddr )
{
	DWORD dwI;
	UCHAR	*pTbl;


	dwI = (DWORD)( dwPhysAddr / (DWORD)4096 );

	pTbl = bell.pPhysRefTbl;
	if( pTbl[dwI] == 0 )
		return( -1 );			// 할당되지 않은 메모리이다.

	pTbl[dwI]--;

	return( 1 );
}

// 페이지 디렉토리 엔트리를 초기화한다.
static int nSetPDEntry( DWORD *pEntry, DWORD dwPTBase,  DWORD dwBit )
{
	// 주소가 4096정렬인가?
	if( (dwPTBase & (DWORD)0xFFF) != 0  )
		return( -1 );	
	
	dwBit = (DWORD)(dwBit & 0xFFF );
	pEntry[0] = (DWORD)( dwPTBase | dwBit );

	return( 0 );
}

// 페이지 테이블 엔트리를 초기화한다.
static int nSetPTEntry( DWORD *pEntry, DWORD dwPageBase,  DWORD dwBit )
{
	// 주소가 4096정렬인가?
	if( (dwPageBase & (DWORD)0xFFF) != 0  )
		return( -1 );	

	dwBit = (DWORD)(dwBit & 0xFFF );
	pEntry[0] = (DWORD)( dwPageBase | dwBit );

	return( 0 );
}

static DWORD dwKernelMappingFlag = 1;

// update kernel mapping flag
int update_kernel_mapping_flag()
{
	dwKernelMappingFlag++;

	return( 0 );
}

// get kernel mapping flag
DWORD get_kernel_mapping_flag()
{
	return( dwKernelMappingFlag );
}

int forced_mapping( DWORD *pPD, DWORD dwVAddr, DWORD dwPhysAddr )
{
	DWORD	dwT;
	DWORD	*pPT;
	DWORD	dwBitFlag;

	dwBitFlag = (DWORD)( PT_BIT_W | PT_BIT_P );

	// calculate index in the page directory ( divide by 4 )
	dwT = dwVAddr / 0x400000;  

	// get page table address from the page directory and index
	pPT = (DWORD*)( pPD[dwT] & (DWORD)0xFFFFF000 );
	if( pPT == NULL )	// no page table exists. it must be newly mapped.
	{	// get new page table ( new page dir is allocated between 1M and 1.5M area )
		pPT = pAllocPageTable();	
		nSetPDEntry( &pPD[dwT], (DWORD)pPT,  dwBitFlag );
	}	

	// check page table entry
	dwT = (dwVAddr % 0x400000) / 0x1000;  
	//if( ( pPT[dwT] & PT_BIT_P ) != 0 )  // the page is already mapped
	//{
	//	kdbg_printf( "the virtual address 0x%08X is already mapped!\n", dwVAddr );
	//	return( -1 );
	//}
		
	// set page table entry
	nSetPTEntry( &pPT[dwT], dwPhysAddr,  dwBitFlag );
	
	// if the virtual address is lower than 2GB then increase counter
	if( dwVAddr < 0x80000000 )
	{	// copy lower 2GB mapping
		memcpy( bell.pPD, pPD, 4096 / 2 );
		update_kernel_mapping_flag();
	}					   

	return( 0 );
}

// make page table entry and page directory entry if needed.
static int internal_mapping( DWORD *pPD, DWORD dwVAddr, DWORD dwPhysAddr, DWORD dwBitFlag )
{
	DWORD	dwT;
	DWORD	*pPT;

	// calculate index in the page directory ( divide by 4 )
	dwT = dwVAddr / 0x400000;  

	// get page table address from the page directory and index
	pPT = (DWORD*)( pPD[dwT] & (DWORD)0xFFFFF000 );
	if( pPT == NULL )	// no page table exists. it must be newly mapped.
	{	// get new page table ( new page dir is allocated between 1M and 1.5M area )
		pPT = pAllocPageTable();	
		nSetPDEntry( &pPD[dwT], (DWORD)pPT,  dwBitFlag );
	}	

	// check page table entry
	dwT = (dwVAddr % 0x400000) / 0x1000;  
	if( ( pPT[dwT] & PT_BIT_P ) != 0 )  // the page is already mapped
	{
		kdbg_printf( "The virtual address 0x%08X is already mapped!\n", dwVAddr );
		return( -1 );
	}
		
	// set page table entry
	nSetPTEntry( &pPT[dwT], dwPhysAddr,  dwBitFlag );
	
	// if the virtual address is lower than 2GB then increase counter
	if( dwVAddr < 0x80000000 )
	{
		// copy lower 2GB mapping
		memcpy( bell.pPD, pPD, 4096 / 2 );

		update_kernel_mapping_flag();
	}					   

	return( 0 );
}

// superuser level mapping -> user level mapping
int change_mapping_to_user( DWORD dwVAddr )
{
	DWORD	dwAttr, dwY, dwT, dwX, *pPT, *pPD;
	
	if( ( dwVAddr & 0x80000000 ) != 0 )
		return( -1 );	// 이미 user영역이면 에러.

	_asm {
		MOV EAX,CR3
		AND EAX,0xFFFFF000
		MOV pPD, EAX
	}

	dwY = dwVAddr / 0x400000;  
	pPT = (DWORD*)( pPD[dwY] & (DWORD)0xFFFFF000 );
	if( pPT == NULL )
		return( -1 );

	dwT = ( dwVAddr % 0x400000 ) / 0x1000;  
	if( ( pPT[dwT] & PT_BIT_P ) == 0 )
		return( -1 );

	// 페이지 테이블의 매핑 속성을 변경한다.  
	// (R3에서 액세스 하려면 반드시 PTE 뿐만 아니라 PDE의 속성도 변경해야 한다.)
	dwX = pPD[dwY];
	dwAttr = (DWORD)( dwX & 0xFFF ) | (DWORD)PD_BIT_U;
	nSetPDEntry( &pPD[dwY], (dwX & (DWORD)0xFFFFF000), dwAttr );
	
	// 페이지 테이블 엔트리의 매핑 속성을 변경한다.
	dwX = pPT[dwT];
	dwAttr = ( dwX & 0xFFF ) | PT_BIT_U;
	nSetPTEntry( &pPT[dwT], (dwX & (DWORD)0xFFFFF000), dwAttr );

	return( 0 );
}

// supervisor level mapping
int _nMappingVAddr( DWORD *pPD, DWORD dwVAddr, DWORD dwPhysAddr )
{
	int		nR;

	nR = internal_mapping( pPD, dwVAddr, dwPhysAddr, (DWORD)( PT_BIT_W | PT_BIT_P ) );

	return( nR );
}

// user level mapping
int _nMappingUserVAddr( DWORD *pPD, DWORD dwVAddr, DWORD dwPhysAddr )
{
	int		nR;

	nR = internal_mapping( pPD, dwVAddr, dwPhysAddr, (DWORD)( PT_BIT_U | PT_BIT_W | PT_BIT_P ) );

	return( nR );
}

// release mapping and return physical address
// return value 0 meand an error.
DWORD dwReleaseMappingVAddr( DWORD *pPD, DWORD dwVAddr )
{
	DWORD	dwT, *pPT, dwPhysAddr;

	// index in the page directory (divide by 4M)
	dwT = dwVAddr / 0x400000;  

	// get page table address from page directory and index
	pPT = (DWORD*)( pPD[dwT] & (DWORD)0xFFFFF000 );
	if( pPT == NULL )	// no page table. error!
	{
		kdbg_printf( "dwReleaseMappingVAddr( 0x%08X) - page table not found!\n", dwVAddr );
		return( 0 );
	}	

	// get page table entry  ( divide the by 4K )
	dwT = (dwVAddr % 0x400000) / 0x1000;  
	if( ( pPT[dwT] & PT_BIT_P ) == 0 )  // the page is not mapped
	{
		kdbg_printf( "dwReleaseMappingVAddr( 0x%08X) - the page is not mapped!\n", dwVAddr );
		return( 0 );
	}
	
	// get physical address
	dwPhysAddr = (DWORD)( pPT[dwT] & (DWORD)0xFFFFF000 );

	// clear page table entry
	pPT[dwT] = 0;

	return( dwPhysAddr );
}

// 물리 페이지의 Ref Count를 1 증가 시킨다.
static int nIncRefCount( DWORD dwPhysAddr )
{
	int nI;

	nI = dwPhysAddr / 4096;
	
	if( bell.pPhysRefTbl[nI] == 255 )
		return( -1 );

	bell.pPhysRefTbl[nI]++;

	return( 0 );
}

// 물리 페이지의 Ref Count를 1 감소 시킨다.
static int nDecRefCount( DWORD dwPhysAddr )
{
	int nI;

	nI = dwPhysAddr / 4096;

	if( bell.pPhysRefTbl[nI] == 0 )
		return( -1 );

	bell.pPhysRefTbl[nI]--;

	return( 0 );
}

// 커널의 페이지 디렉토리와 페이지 테이블을 초기화한다.
void vInitKernelPage( DWORD *pPD, DWORD dwImageAddr, DWORD dwImageSize )
{
	DWORD	*pT0, *pT1;
	DWORD	dwAddr, dwT;

	dwT = (DWORD)pPD;
	
	pT0 = (DWORD*)( dwT + 4096 );
	pT1 = (DWORD*)( dwT + 4096*2 );
	
	memset( pT0, 0, 4096 );
	memset( pT1, 0, 4096 );

	// 1-3M, 4-7M까지 2개의 페이지 테이블( 페이지 디렉토리 인덱스 0, 1 ) 두개를 잡아준다.
	nSetPDEntry( &pPD[0], (DWORD)pT0,  (DWORD)(PD_BIT_U | PD_BIT_W | PD_BIT_P) );
	nSetPDEntry( &pPD[1], (DWORD)pT1,  (DWORD)(PD_BIT_U | PD_BIT_W | PD_BIT_P) );

	// 하위 0M의 172K를 매핑한다.  (FDD DMA 64k, HDD DMA 64k, V86Lib 64k,   )
	for( dwAddr = 0; dwAddr < 1024*256; dwAddr += 4096 )
	{
		_nMappingUserVAddr( pPD, dwAddr, dwAddr );	// 가상주소와 실주소를 같이 매핑한다.
		nIncRefCount( dwAddr );					// 해당 물리페이지가 매핑되어 사용중임을 표시한다.
	}

	// 0xA0000 - 0x100000 까지 shadow mapping.	( Video Mem, ROM BIOS )
	for( dwAddr = 0xA0000; dwAddr < 0x100000; dwAddr += 4096 )
	{								
		_nMappingUserVAddr( pPD, dwAddr, dwAddr );	// 가상주소와 실주소를 같이 매핑한다.
		nIncRefCount( dwAddr );					// 해당 물리페이지가 매핑되어 사용중임을 표시한다.
	}

	// 커널의 이미지 영역
	for( dwAddr = dwImageAddr; dwAddr < dwImageAddr + dwImageSize; dwAddr += 4096 )
	{
		_nMappingVAddr( pPD, dwAddr, dwAddr );	 
		nIncRefCount( dwAddr );
	}

	// 커널의 스택 영역
	for( dwAddr = KERNEL_INIT_STACK_BASE - KERNEL_STACK_SIZE; dwAddr < KERNEL_INIT_STACK_BASE; dwAddr += 4096 )
	{
		_nMappingVAddr( pPD, dwAddr, dwAddr );	 
		nIncRefCount( dwAddr );
	}  
}

// mapping virtual address by dwSize.
// nLevel means the access level (user/superuser)
static int mapping( DWORD *pPD, DWORD dwVAddr, DWORD dwSize, int nLevel )
{
	int		nR;
	DWORD	dwI, dwPhysPage;

	dwSize = (DWORD)( dwSize + 4095 );
	dwSize = (DWORD)( dwSize / 4096 ) * 4096;

	for( dwI = 0; dwI < dwSize; dwI += 4096 )
	{
		// allocate physical page
		dwPhysPage = dwAllocPhysPage();
		if( dwPhysPage == 0 )
			return( -1 );

		// mapping the physical page
		if( nLevel ==  0 )
			nR = _nMappingVAddr( pPD, dwVAddr + dwI, dwPhysPage );
		else
			nR = _nMappingUserVAddr( pPD, dwVAddr + dwI, dwPhysPage );

		if( nR < 0 )
		{
			kdbg_printf( "nMapping() - _nMappingVAddr failed!\n" );
			return( -1 );
		}
	}	

	return( 0 );
}

// superuser level virtual address mapping
int nMapping( DWORD *pPD, DWORD dwVAddr, DWORD dwSize )
{
	int nR;												

	// mapping for supervisor only.
	nR = mapping( pPD, dwVAddr, dwSize, 0 );

	return( nR );
}

// user level virtual address mapping
int nMappingUser( DWORD *pPD, DWORD dwVAddr, DWORD dwSize )
{
	int nR;

	// user mapping
	nR = mapping( pPD, dwVAddr, dwSize, 3 );

	return( nR );
}

// PG 비트를 1로 만들어서 페이징 가능하도록 한다.
void vEnablePaging( DWORD *pPageDir )
{
	// CR3 값을 설정한다.
	_asm{
		PUSH EAX
		PUSHFD
		CLI

		// CR3 설정.
		MOV  EAX, CR3		   
		MOV  EAX, pPageDir
		MOV  CR3, EAX
		MOV  EAX, CR0
		OR   EAX, 0x80000000   // PG비트를 1로 설정한다.
		MOV  CR0, EAX
		
		POPFD
		POP  EAX
	}
}

// 페이지 디렉토리 혹은 페이지 테이블로 사용할 4096 한 페이지를 할당한다.
// 아무 물리 페이지든 하나를 할당해서 선형주소 1M, 3M대에 매핑하여 리턴한다.
// 1,2,3M대 영역은 처음부터 페이지 테이블이 존재하기 때문에 
// 페이지 테이블을 할당하기 위해 다시 페이지 페이블을 할당해야 하는 일이
// 발생하지 않는다.  이 영역에 256 * 3개의 페이지 테이블을 할당할 수 있다.
// !!! 아무 페이지나 할당해서 매핑해주면 페이지 테이블에 접근하기가 매우 곤란하다.
// 페이지 테이블은 물리 주소와 가상 주소가 같은 곳에 매핑해야 한다.
static int nTotalPageTbl = 0;
DWORD *pAllocPageTable()
{
	UCHAR	*pB;
	DWORD   dwAddr;			
	int		nI, nR;

	pB = (UCHAR*)0x100000;		// 1M영역
	for( nI = 0; nI < nTotalPageTbl; nI++ )
	{
        if( check_memory_validity( 0, (DWORD)pB ) < 0 )
            continue;

		if( memcmp( pB, "FREE", 4 ) == 0 )
			break;

		pB += 4096;
	}

    dwAddr = (DWORD)0x100000 + ( (DWORD)4096 * (DWORD)nI );
	if( nI >= nTotalPageTbl )		// 새로 매핑해 주어야 한다.
	{
		nR = _nMappingVAddr( bell.pPD, dwAddr, dwAddr );
		if( nR < 0 )
		{
            kdbg_printf( "pAllocaPageTable() - _nMappingVAddr failed!\n" );
			return( 0 );
		}

		nIncRefCount( dwAddr );

		nTotalPageTbl++;
	}
	
    memset( (UCHAR*)dwAddr,	0, 4096 );

	return( (DWORD*)dwAddr );	
}

// release page table.  
// just copy string "FREE" onto start of the page. 
int nFreePageTable( DWORD dwAddr )
{
	memcpy( (UCHAR*)dwAddr, "FREE", 4 );
	return( 0 );
}

// release mapping 
int nReleaseMapping( DWORD *pPD, char *pVAddr, long lSize )
{
	int		nR;
	DWORD	dwAddr, dwI, dwPhysAddr;

	dwAddr = (DWORD)pVAddr;

	for( dwI = 0; dwI < (DWORD)lSize; dwI += 4096 )
	{
		// release mapping and return physical page	
		dwPhysAddr = dwReleaseMappingVAddr( pPD, dwAddr + dwI );	
		if( dwPhysAddr == 0 )
			return( -1 );

		// decrease physical page's reference counter by 1 
		nR = nDecRefCount( dwPhysAddr );
		if( nR == -1 )
			return( -1 );
	}

	return( 0 );
}

// 사용자 영역의 매핑을 모두 해제한다.
int __release_user_area( AddrSpaceStt *pA, int nAll )
{
	DWORD   *pPD;
	DWORD	dwI, dwK, *pPT, dwAddr;

	if( pA == NULL )
		return( -1 );
	pPD = pA->pPD;

	_asm {
		PUSHFD
		CLI
	}

	for( dwI = 512; dwI < 1024; dwI++ )
	{
		if( pPD[dwI] & PD_BIT_P )
		{	// get page table base address
			pPT = (DWORD*)( pPD[dwI] & (DWORD)0xFFFFF000 );

			for( dwK = 0; dwK < 1024; dwK++ )
			{
				if( pPT[dwK] & PT_BIT_P )
				{	// RW만 지우는 것인데 W BIt가 '0'이면 스킵.
					if( nAll == 0 && ( pPT[dwK] & PT_BIT_W ) == 0 )
						continue;

					dwAddr = (DWORD)( pPT[dwK] & (DWORD)0xFFFFF000 );
					nDecRefCount( dwAddr );		// decrease page reference counter
				}
			}
			nDecRefCount( (DWORD)pPT );			// decrease page reference counter
		}	
	}	

	_asm POPFD

	return( 0 );
}

// 사용자 영역 전체를 해제한다.
int release_user_area( AddrSpaceStt *pA )
{
	int nR;
	nR = __release_user_area( pA, 1 );	// 1 = releae all
	return( nR );
}

// 사용자 영역 가운데 rw 매핑된 페이지만 해제한다.
int release_rw_user_area( AddrSpaceStt *pA )
{
	int nR;
	nR = __release_user_area( pA, 0 );	// 0 = rw only
	return( nR );
}

// 매모리 매핑 내역을 출력한다.
int disp_map( DWORD dwVAddr )
{
	int		nDIndex, nTIndex;
	DWORD	*pPD, *pPT, dwPhysAddr;
	char	szAttr[32];

	_asm {
		MOV EAX,CR3
		AND EAX,0xFFFFF000
		MOV pPD, EAX
	}

	dwVAddr = (dwVAddr / 4096) * 4096;

	nDIndex = (int)( dwVAddr / 0x400000 );
	nTIndex = (int)( ( dwVAddr % 0x400000 ) / 0x1000 );

	if( !( (DWORD)pPD[nDIndex] & PD_BIT_P) )
	{
		kdbg_printf( "CR3(0x%08X) page table not found!\n", (DWORD)pPD );
		return( -1 );
	}

	pPT = (DWORD*)( pPD[nDIndex] & (DWORD)0xFFFFF000 );

	if( !(DWORD)( pPT[nTIndex] & PT_BIT_P ) )
	{
		kdbg_printf( "CR3(0x%08X) page table entry not found!\n", (DWORD)pPD );
		return( -1 );
	}

	//get attribute
	strcpy( szAttr, "d a u w (PD-Ent: u w)" );
	if( pPT[nTIndex] & (DWORD)PT_BIT_D ) szAttr[0] = 'D';
	if( pPT[nTIndex] & (DWORD)PT_BIT_A ) szAttr[2] = 'A';
	if( pPT[nTIndex] & (DWORD)PT_BIT_U ) szAttr[4] = 'U';
	if( pPT[nTIndex] & (DWORD)PT_BIT_W ) szAttr[6] = 'W';	
	if( pPD[nDIndex] & (DWORD)PD_BIT_W ) szAttr[19] = 'W';	
	if( pPD[nDIndex] & (DWORD)PD_BIT_U ) szAttr[17] = 'U';	

	// get physical address
	dwPhysAddr = (DWORD)( pPT[nTIndex] & (DWORD)0xFFFFF000 );

	kdbg_printf( "0x%08X -> 0x%08X %s\n", dwVAddr, dwPhysAddr, szAttr );

	return( 0 );
}

static DWORD find_next_linear2( DWORD *pPD, DWORD dwVAddr, int nPresent )
{
	DWORD	*pPT;
	int		nTIndex, nDIndex, nI;

	dwVAddr = (dwVAddr / 4096) * 4096;

	nDIndex = (int)( dwVAddr / 0x400000 );
	nTIndex = (int)( ( dwVAddr % 0x400000 ) / 0x1000 );
			
	if( ((DWORD)pPD[nDIndex] & (DWORD)PD_BIT_P) == 0 )
	{
		if( nPresent == 0 )	
			return( dwVAddr );		// i got it!
		else
			return(0);

	}
	else
	{
		if( nPresent != 0 )
			return( dwVAddr );
		else
			return(0);
	}

	pPT = (DWORD*)( pPD[nDIndex] & (DWORD)0xFFFFF000 );
	for( nI = nTIndex; nI < 1024; nI++ )
	{	// not present
		if( (DWORD)( pPT[nI] & PT_BIT_P ) == 0 )
		{
			if( nPresent == 0 )
				return( dwVAddr );
		}
		else // present (already mapped)
		{
			if( nPresent != 0 )
				return( dwVAddr );
		}

		dwVAddr += 0x1000;	// + 4k
	}

	return( 0 );
}

static DWORD find_next_linear( DWORD *pPD, DWORD dwVAddr, int nPresent )
{
	int		nI;
	DWORD	dwAddr;
	
	dwVAddr = (dwVAddr / 4096) * 4096;

	// get page directory index
	nI = (int)( dwVAddr / 0x400000 );
	for( ; nI < 1024; nI++ )
	{
		dwAddr = find_next_linear2( pPD, dwVAddr, nPresent );
		if( dwAddr != 0 )
			return( dwAddr );

		dwVAddr += 0x400000;	// + 4M
	}

	return( 0 );
}	

// 커널 모듈을 로드하기 위해서만 사용한다.!!
DWORD find_free_linear_space( DWORD *pPD, DWORD dwStart, DWORD dwSize )
{
	int		nPage;
	DWORD	dwNextFreeAddr, dwNextUsedAddr;

	nPage			= dwSize / 4096;
	dwNextFreeAddr	= dwStart;

	for( ; dwNextFreeAddr < LOAD_MODULE_START_ADDR + LOAD_MODULE_SPACE_SIZE; )
	{
		// find next free addr
		dwNextFreeAddr = find_next_linear( pPD, dwNextFreeAddr, 0 );
		if( dwNextFreeAddr == 0 )
		{
			kdbg_printf( "find_free_linear_space() - dwNextFreeAddr is 0\n" );
			return( 0 );
		}

		// find next_used addr
		dwNextUsedAddr = find_next_linear( pPD, dwNextFreeAddr, 1 );
		if( dwNextUsedAddr == 0 )
			dwNextUsedAddr = LOAD_MODULE_START_ADDR + LOAD_MODULE_SPACE_SIZE;
		
		if( dwSize < dwNextUsedAddr - dwNextFreeAddr )
			return( dwNextFreeAddr );
		
		if( dwNextUsedAddr == dwNextFreeAddr )
			return( 0 );

		dwNextFreeAddr = dwNextUsedAddr;
	}

	return( 0 );
}

int check_memory_validity( DWORD dwCR3, DWORD dwAddr )
{
	DWORD	*pPD, *pPT;
	int		nDIndex, nTIndex;

	if( dwCR3 == 0 )
	{
		_asm {
			MOV EAX,CR3
			AND EAX,0xFFFFF000
			MOV pPD, EAX
		}
	}
	else
		pPD = (DWORD*)dwCR3;

	nDIndex = (int)( dwAddr / 0x400000 );
	nTIndex = (int)( ( dwAddr % 0x400000 ) / 0x1000 );

	if( !( (DWORD)pPD[nDIndex] & PD_BIT_P) )
		goto INVALID;

	pPT = (DWORD*)( pPD[nDIndex] & (DWORD)0xFFFFF000 );
	if( !(DWORD)( pPT[nTIndex] & PT_BIT_P ) )
		goto INVALID;

	return( 0 );

INVALID:

	kdbg_printf( "\nInvalid memory: CR3 = 0x%X, Addr = 0x%X\n", (DWORD)pPD, dwAddr );

	return( -1 );
}

// 가상 주소를 물리 주소로 변경한다.
DWORD get_physical_address( DWORD dwAddr )
{
	DWORD	*pPD, *pPT, dwPhysical, dwSparse;
	int		nDIndex, nTIndex;

	_asm {
		MOV EAX,CR3
		AND EAX,0xFFFFF000
		MOV pPD, EAX
	}

	dwSparse = (DWORD)( dwAddr & 0xFFF );
	nDIndex = (int)( dwAddr / 0x400000 );
	nTIndex = (int)( ( dwAddr % 0x400000 ) / 0x1000 );

	if( !( (DWORD)pPD[nDIndex] & PD_BIT_P) )
		return( 0 );

	pPT = (DWORD*)( pPD[nDIndex] & (DWORD)0xFFFFF000 );
	if( !(DWORD)( pPT[nTIndex] & PT_BIT_P ) )
		return( 0 );

	dwPhysical = (DWORD)(pPT[nTIndex] & (DWORD)0xFFFFF000 );
	dwPhysical += dwSparse;
	return( dwPhysical );
}

// Page Table, Page Directory를 복제한다.
int dup_page_dir( ProcessStt *pDestProcess, ProcessStt *pSrcProcess, int nStartIndex )
{
	int		nI, nK;
	DWORD   *pSrc, *pDest;
	DWORD	*pPT, dwBitFlag;

	// fork로 인한 참조 카운터를 증가시킨다.
	pSrcProcess->pAddrSpace->nForkRefCounter++;
	pDestProcess->pAddrSpace->pForkRef = pSrcProcess->pAddrSpace;

	pSrc  = (DWORD*)get_process_page_dir( pSrcProcess );
	pDest = (DWORD*)get_process_page_dir( pDestProcess );

	for( nI = nStartIndex; nI < 1024; nI++ )
	{
		if( pSrc[nI] == 0 )
		{
			pDest[nI] = 0;
			continue;
		}

		// 페이지 테이블을 할당한다.
		pPT = pAllocPageTable();
		if( pPT == NULL )
		{
			kdbg_printf( " dup_page_dir: pAllocPageDir failed!\n" );
			return( -1 );
		}

		// 할당된 페이지 디렉토리 엔트리를 설정한다.  (PD 엔트리는 RW 속성으로 준다.)
		dwBitFlag = (DWORD)( pSrc[nI] & 0xFFF ) | (DWORD)PD_BIT_W;
		dwBitFlag &= (DWORD)~(DWORD)PD_BIT_A;	  // A 속성을 클리어 한다.
		nSetPDEntry( &pDest[nI], (DWORD)( (DWORD)pPT & (DWORD)0xFFFFF000 ),  dwBitFlag );

		// 페이지 테이블을 복제한다. (memcpy를 사용해도 된다.)
		memcpy( pPT, (DWORD*)((DWORD)( pSrc[nI] & 0xFFFFF000 )), 4096 );

		// 페이지 테이블의 RW, A 속성을 모두 날린다.
		for( nK = 0; nK < 1024; nK++ )
		{
			pPT[nK] &= (DWORD)~( (DWORD)PT_BIT_W | (DWORD)PT_BIT_A );
		}
	}	

	return( 0 );
}

int set_cr3_in_tss( TSSStt *pTSS, DWORD dwCR3 )
{
	if( pTSS == NULL )
		return( -1 );
	
	pTSS->dwCR3 = dwCR3; 

	return( 0 );
}

int set_cur_cr3_in_tss( DWORD dwCR3 )
{
	int nR;
	nR = set_cr3_in_tss( get_current_tss(), dwCR3 );
	return( nR );
}

// RDOnly로 매핑된 dwAddr에 새로운 물리 페이지를 할당하여 RDWR로 매핑한다.
static BYTE	cow_buff[4096];
int copy_on_write( DWORD *pPD, DWORD dwAddr )
{
	static DWORD	dwOrgCR3, dwLinAddr;
	int				nDIndex, nTIndex;
	DWORD			dwOldPhys, dwPhys, *pPT;

	if( ( dwAddr & 0x80000000 ) == 0 )
	{	// 커널 영역은 제외한다.
		kdbg_printf( "copy_on_write: address (0x%X) is kernel area\n", dwAddr );
		return( -1 );
	}

	dwAddr &= (DWORD)0xFFFFF000;

	// 미리 복사해 둔다.
	memcpy( cow_buff, (void*)dwAddr, sizeof( cow_buff ) );

	// 페이지 속성 비트를 비교한다. (P는 있는가?, W는 없는가?)
	nDIndex = (int)( dwAddr / 0x400000 );
	nTIndex = (int)( ( dwAddr % 0x400000 ) / 0x1000 );
	pPT = (DWORD*)( pPD[nDIndex] & (DWORD)0xFFFFF000 );
	if( (DWORD)( pPT[nTIndex] & PT_BIT_W ) )
	{
		kdbg_printf( "copy_on_write: address(0x%X) is writable!\n", dwAddr );
		return( -1 );	// 이미 RW로 매핑되어 있다.
	}
	if( (DWORD)( pPT[nTIndex] & PT_BIT_P ) == 0 )
	{
		kdbg_printf( "copy_on_write: address(0x%X) is nonpresent!\n", dwAddr );
		return( -1 );	// 없는 페이지.
	}

	// 물리 페이지를 할당한다.
	dwPhys = dwAllocPhysPage();
	if( dwPhys == 0 )
		return( -1 );

	// 물리 주소를 지우고 주소와 W비트를 새로 설정한다.
	dwOldPhys = pPT[nTIndex] & 0xFFFFF000;
	pPT[nTIndex] &= (DWORD)0xFFF;
	pPT[nTIndex] |= dwPhys;
	pPT[nTIndex] |=	(DWORD)PT_BIT_W;

	// 매핑이 변경되는 물리 주소를 출력한다.
	//kdbg_printf( "copy_on_write(CR3=0x%X): Addr=0x%X (0x%X -> 0x%X)\n", (DWORD)pPD, dwAddr, dwOldPhys, dwPhys );

	dwLinAddr = dwAddr;	// CR3가 바뀌면 dwAddr에 접근하지 못한다.	
	set_cur_cr3_in_tss( (DWORD)pPD ); // !!!  빼먹으면 CR3가 안바뀐다.
	// 데이터를 복사한다.
	_asm {
		MOV EAX, CR3
		MOV dwOrgCR3, EAX
		MOV EAX, pPD
		MOV CR3, EAX
		FLUSH_TLB

		PUSHFD////////////////////////////////////////////////////////////
		CLD							// memcpy 대신 사용.
		PUSH ESI
		PUSH EDI
			MOV ESI, OFFSET cow_buff
			MOV EDI, dwLinAddr
			MOV ECX, 4096 / 4
			REP MOVSD
		POP EDI
		POP ESI
		POPFD////////////////////////////////////////////////////////////
	}
	set_cur_cr3_in_tss( dwOrgCR3 );	  // !!!  빼먹으면 CR3가 안바뀐다.
	_asm {
		MOV EAX, dwOrgCR3
		MOV CR3, EAX
		FLUSH_TLB
	}


	return( 0 );
}

static int get_next_pdptent( DWORD *pPD, DWORD *pAddr, DWORD *pPDEnt, DWORD *pPTEnt )
{
	DWORD		*pT, dwAddr;
	int			nD, nT, dwPTEnt;

	dwAddr = pAddr[0];
	nD = (int)( dwAddr / 0x400000 );
	nT = (int)( ( dwAddr % 0x400000 ) / 0x1000 );

	for( ; ; nD++ )
	{
		if( nD >= 1024 )
			return( -1 );
		
		// 페이지 디렉토리 엔트리를 검사한다.
		pPDEnt[0] = pPD[nD];
		pT = (DWORD*)( (DWORD)pPD[nD] & (DWORD)0xFFFFF000 );
		if( pT == 0 )
			continue;

		for( ;; nT++ )
		{
			if( nT >= 1024 )
				break;

			// 페이지 테이블 엔트리를 검사한다.
			dwPTEnt = ( (DWORD)pT[nT] & (DWORD)0xFFFFF000 );
			if( dwPTEnt != 0 )
			{
				pPTEnt[0] = pT[nT];
				pAddr[0] = (DWORD)( nD * 0x400000 ) + (DWORD)( nT * 0x1000 );
				return( 0 );
			}
		}
		nT = 0;
	}

	return( -1 );
}

static int get_prev_pdptent( DWORD *pPD, DWORD *pAddr, DWORD *pPDEnt, DWORD *pPTEnt )
{
	DWORD		*pT, dwAddr;
	int			nD, nT, dwPTEnt;

	dwAddr = pAddr[0];
	nD = (int)( dwAddr / 0x400000 );
	nT = (int)( ( dwAddr % 0x400000 ) / 0x1000 );

	for( ; ; nD-- )
	{
		if( nD < 0 )
			return( -1 );
		
		// 페이지 디렉토리 엔트리를 검사한다.
		pPDEnt[0] = pPD[nD];
		pT = (DWORD*)( (DWORD)pPD[nD] & (DWORD)0xFFFFF000 );
		if( pT == NULL )
			continue;

		for( ;; nT-- )
		{
			if( nT < 0 )
				break;

			// 페이지 테이블 엔트리를 검사한다.
			dwPTEnt = ( (DWORD)pT[nT] & (DWORD)0xFFFFF000 );
			if( dwPTEnt != 0 )
			{
				pPTEnt[0] = pT[nT];
				pAddr[0] = (DWORD)( nD * 0x400000 ) + (DWORD)( nT * 0x1000 );
				return( 0 );
			}
		}
		nT = 1023;
	}

	return( -1 );
}

static char *str_pdent_attr( char *pS, DWORD dwEnt )
{
	dwEnt &= (DWORD)0xFFF;

	strcpy( pS, "actuwp" );

	if( PD_BIT_A	& dwEnt ) pS[0] = 'A';
	if( PD_BIT_PCD	& dwEnt ) pS[1] = 'C';
	if( PD_BIT_PWT	& dwEnt ) pS[2] = 'T';
	if( PD_BIT_U	& dwEnt ) pS[3] = 'U';
	if( PD_BIT_W	& dwEnt ) pS[4] = 'W';
	if( PD_BIT_P	& dwEnt ) pS[5] = 'P';
	return( pS );
}

static char *str_ptent_attr( char *pS, DWORD dwEnt )
{
	dwEnt &= (DWORD)0xFFF;

	strcpy( pS, "dactuwp" );

	if( PT_BIT_D	& dwEnt ) pS[0] = 'D';
	if( PT_BIT_A	& dwEnt ) pS[1] = 'A';
	if( PT_BIT_PCD	& dwEnt ) pS[2] = 'C';
	if( PT_BIT_PWT	& dwEnt ) pS[3] = 'T';
	if( PT_BIT_U	& dwEnt ) pS[4] = 'U';
	if( PT_BIT_W	& dwEnt ) pS[5] = 'W';
	if( PT_BIT_P	& dwEnt ) pS[6] = 'P';
	return( pS );
}

// 문자열 pS가 문자열 pChar에 포함된 문자를 가지고 있는 개수를 리턴한다.
static int count_char( char *pS, char *pChar )
{
	int nX, nY, nTotal;

	nTotal = 0;
	for( nX = 0; pChar[nX] != 0; nX++ )
	{
		for( nY = 0; pS[nY] != 0; nY++ )
		{
			if( pS[nY] ==  pChar[nX] )
			{
				nTotal++;
				break;
			}
		}
	}
	return( nTotal );
}

// 페이지 디렉토리와 페이지 테이블 엔트리를 출력한다.
int disp_page_dir( DWORD *pPD, DWORD dwStartAddress, char *pAttrStr )
{
	DWORD	dwAddr, dwPDEnt, dwPTEnt;
	char	szPDEnt[32], szPTEnt[32];
	int		nTotal, nKey, nR, nDirection, nMatch, nLength;

RESTART:
	dwAddr = dwStartAddress;

	nTotal = 0;
	nDirection = 1;
	for( ;; nTotal++ )
	{
		if( nDirection > 0 )
			nR = get_next_pdptent( pPD, &dwAddr, &dwPDEnt, &dwPTEnt );
		else
			nR = get_prev_pdptent( pPD, &dwAddr, &dwPDEnt, &dwPTEnt );
		if( nR < 0 )
		{
			kdbg_printf( "end of entry\n" );
			break;
		}

		str_pdent_attr( szPDEnt, dwPDEnt );
		str_ptent_attr( szPTEnt, dwPTEnt );

		// 속성이 지정되어 있으면 비교한다.
		nLength = strlen( pAttrStr );
		if( nLength > 0 )
		{
			nMatch = count_char( szPDEnt, pAttrStr );
			if( nMatch < nLength )
			{
				nMatch = count_char( szPTEnt, pAttrStr );
				if( nMatch < nLength )
					goto NEXT_ADDRESS;
			}
		}		

		// PD, PT Entry를 출력한다.
		kdbg_printf( "0x%08X: 0x%08X[%s] -> 0x%08X[%s]\n", dwAddr, 
			(DWORD)(dwPDEnt & 0xFFFFF000), szPDEnt,  
			(DWORD)(dwPTEnt & 0xFFFFF000), szPTEnt );

		if( nTotal >= 15 )
		{
			nTotal = -1;
			kdbg_printf( "[?]Help, [ESC]stop, [ENTER]Next, [Home]Rewind, [PGUP]Upward, [PGDN]Downward \n" );
			for( ;; )
			{
				nKey = getchar();
				if( nKey == BK_ESC )
					goto QUIT;
				else if( nKey == '?' )
					kdbg_printf( "[D]irty, [A]ccess, [C]ache disable, write [T]hrough, [U]ser, [W]rite, [P]resent\n" );
				else if( nKey == BK_ENTER )
					break;
				else if( nKey == BK_HOME )
					goto RESTART;
				else if( nKey == BK_PGUP )
				{
					nDirection = -1;
					break;
				}
				else if( nKey == BK_PGDN )
				{
					nDirection = 1;
					break;
				}
			}
		}

NEXT_ADDRESS:
		if( nDirection > 0 )
			dwAddr += 0x1000;
		else
			dwAddr -= 0x1000;
	}
QUIT:
	return( 0 );
}







