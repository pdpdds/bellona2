#ifndef BELLONA_PAGING_HEADER_oh
#define BELLONA_PAGING_HEADER_oh

#define PAGE_FAULT_P	1
#define PAGE_FAULT_W	2
#define PAGE_FAULT_U	4

//#define FLUSH_TLB _asm int 0x53;
#define FLUSH_TLB2(addr) _asm invlpg addr

// Page Directory 비트값.  PS,  0, A, PCD,     PWT, U, W, P
#define PD_BIT_PS	0x80	// 페이지 사이즈 ( 0 = 4K, 1= 4M )
#define PD_BIT_A	0x20	// Accessed Bit
#define PD_BIT_PCD	0x10	// Page Cache Disable ( 1 = Disable )
#define PD_BIT_PWT	0x08	// Page write Through ( 0 = Write Back, 1 = Write Through )
#define PD_BIT_U	0x04	// User/Supervisor ( 0 = OS, 1 = APP )
#define PD_BIT_W	0x02	// Read Write ( 0 = read-Only, 1 = Read/Write )
#define PD_BIT_P	0x01	// Present ( 1 = Present )
// Page Table 비트값.  D,A,PCD, pWT,U,W,P  (Dirty 하나만 틀리고 다 같다.)
#define PT_BIT_D	0x40	// Page Dirty
#define PT_BIT_A	0x20	// Accessed Bit
#define PT_BIT_PCD	0x10	// Page Cache Disable ( 1 = Disable )
#define PT_BIT_PWT	0x08	// Page write Through ( 0 = Write Back, 1 = Write Through )
#define PT_BIT_U	0x04	// User/Supervisor ( 0 = OS, 1 = APP )
#define PT_BIT_W	0x02	// Read Write ( 0 = read-Only, 1 = Read/Write )
#define PT_BIT_P	0x01	// Present ( 1 = Present )

extern int		copy_on_write				( DWORD *pPD, DWORD dwAddr );
extern int		_nMappingUserVAddr			( DWORD *pPD, DWORD dwVAddr, DWORD dwPhysAddr );
extern int		_nMappingVAddr				( DWORD *pPD, DWORD dwVAddr, DWORD dwPhysAddr );
extern int		change_mapping_to_user		( DWORD dwVAddr );
extern int		disp_map					( DWORD dwVAddr );
extern int		forced_mapping				( DWORD *pPD, DWORD dwVAddr, DWORD dwPhysAddr );
extern int		nFreePage					( DWORD dwPhysAddr );
extern int		nFreePageTable				( DWORD dwAddr );
extern int		nMapping					( DWORD *pPD, DWORD dwVAddr, DWORD dwSize );
extern int		nMappingUser				( DWORD *pPD, DWORD dwVAddr, DWORD dwSize );
extern int		nReleaseMapping				( DWORD *pPD, char *pVAddr, long lSize );
extern int		release_user_area			( struct AddrSpaceTag *pA );
extern int		release_rw_user_area		( struct AddrSpaceTag *pA );
extern int		update_kernel_mapping_flag	();
extern int      check_memory_validity		( DWORD dwCR3, DWORD dwAddr );
extern int      dup_page_dir				( struct ProcessTag *pDestProcess, struct ProcessTag *pSrcProcess, int nStartIndex );
extern int		disp_page_dir				( DWORD *pPD, DWORD dwStartAddress, char *pAttrStr );
extern int		set_cur_cr3_in_tss			( DWORD dwCR3 );
extern int 		set_cr3_in_tss				( struct TSSTag *pTSS, DWORD dwCR3 );


extern void		vEnablePaging				( DWORD *pPageDir );
extern void		vInitKernelPage				( DWORD *pPD, DWORD dwImageAddr, DWORD dwImageSize );

extern DWORD	*pAllocPageTable			();
extern DWORD	dwAllocPhysPage				();
extern DWORD	find_free_linear_space		( DWORD *pPD, DWORD dwStart, DWORD dwSize );
extern DWORD	get_kernel_mapping_flag		();
extern DWORD	get_physical_address		( DWORD dwLinear );
extern DWORD    dwReleaseMappingVAddr		( DWORD *pPD, DWORD dwVAddr );

#endif
