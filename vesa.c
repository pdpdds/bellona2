#include "bellona2.h"

static GuiInfoStt gui_info;

GuiExportStt *get_gui_exp()
{
	return( gui_info.pGuiExp );
}

void set_system_gui_info( int nNewFlag, GuiExportStt *pGuiExp )
{
	gui_info.nGuiFlag = nNewFlag;
	gui_info.pGuiExp  = pGuiExp;
}

static int get_mode_list( VESAStt *pVesa )
{
	int				nR;
	int				nI;
	DWORD			dwR;
	UINT16			*pNo;
	VBEInfoBlockStt	*pVI;
	
	// Set function number and parameter.
	nR = set_v86_param( V86FUNC_GET_VESA_INFO, 0 );
	if( nR < 0 )
		return( -1 );

	memset( pVesa, 0, sizeof( VESAStt ) );

	// Activate the V86 Thread.
	dwR = call_to_v86_thread();
	if( dwR != 0 )
		return( -1 );
				 
	// Check if the function success.
	pVI = (VBEInfoBlockStt*)get_v86_buff();
	if( pVI->VbeSignature != 0x41534556 ) // VESA
		return( -1 );

	// Version
	pVesa->wRevision = pVI->VbeVersion;
	pVesa->dwTotalMemory = (DWORD)( pVI->TotalMemory / (1024/64) ); // �ް�����Ʈ
	strcpy( pVesa->szProduct, (char*)segoffset_to_offset32( pVI->OemProductNamePtr ) );
	strcpy( pVesa->szVendor, (char*)segoffset_to_offset32( pVI->OemVendorNamePtr ) );

	// ��带 ���Ѵ�.
	pNo = (UINT16*)segoffset_to_offset32( pVI->VideoModePtr );
	for( nI = 0; pNo[nI] != 0xFFFF && nI < MAX_VESA_MODE; nI++ )
	{	// �ϴ� ��� ��ȣ�� ���� �д�.
		pVesa->mode[nI].wNo = pNo[nI];
		pVesa->nTotalMode++;
	}

	return( 0 );
}

static int get_mode_info( VESAModeStt *pVM )
{
	int					nR;
	DWORD				dwR;
	ModeInfoBlockStt	*pM;

	// �ķ����͸� �����Ѵ�.
	nR = set_v86_param( V86FUNC_GET_MODE_INFO, (DWORD)pVM->wNo );
	if( nR < 0 )
		return( -1 );
	
	// �ٷ� V86 �����带 Ȱ��ȭ�Ͽ� �����Ѵ�.
	dwR = call_to_v86_thread();
	if( dwR != 0 )
		return( -1 );

	pM = (ModeInfoBlockStt*)get_v86_buff();
	pVM->byBPP			= pM->BitsPerPixel;
	pVM->wAttr			= pM->ModeAttributes;
	pVM->wX				= pM->XResolution;
	pVM->wY				= pM->YResolution;
	pVM->dwPhysBaseAddr = pM->PhysBasePtr;
	pVM->BlueMaskSize	= pM->BlueMaskSize;  
	pVM->GreenMaskSize	= pM->GreenMaskSize; 
	pVM->RedMaskSize	= pM->RedMaskSize;   
	pVM->LinBytesPerScanLine = pM->LinBytesPerScanLine;

	gui_info.vesa.nValidFlag = 1;
	
	return( 0 );
}

#define VMODE_GRAPHIC	(UINT16)0x10
#define VMODE_LINBUFF	(UINT16)0x80
static char *get_vesa_mode_str( UINT16 wAttr, char *pS )
{
	pS[0] = 0;

	if( wAttr & VMODE_GRAPHIC )
		strcpy( pS, "GRA" );
	else
		strcpy( pS, "TXT" );

	if( wAttr & VMODE_LINBUFF )
		strcat( pS, " , LIN" );
	else
		strcat( pS, " , BNK" );

	return( pS );
}	

int disp_vesa_info()
{
	int			nI;
	VESAModeStt	*pV;
	char		szT[128];

	kdbg_printf( "Vendor/Product : %s (%s)\n", gui_info.vesa.szVendor, gui_info.vesa.szProduct );
	kdbg_printf( "Memory Size : %dMb\n", gui_info.vesa.nTotalMode );
	kdbg_printf( "Total mode  : %d\n", gui_info.vesa.nTotalMode );

	// ��庰 �� ������ ���Ѵ�.
	for( nI = 0; nI < gui_info.vesa.nTotalMode; nI++ )
	{
		pV = &gui_info.vesa.mode[nI];
		kdbg_printf( " [0x%04X] : %4d * %4d (%3d) 0x%08X %s", 
				pV->wNo, 
				pV->wX, 
				pV->wY, 
				pV->byBPP, 
				pV->dwPhysBaseAddr, 
				get_vesa_mode_str( gui_info.vesa.mode[nI].wAttr, szT ) 
				);
		if( pV->wAttr & VMODE_GRAPHIC )
			kdbg_printf( "(%d:%d:%d)\n", pV->RedMaskSize, pV->GreenMaskSize, pV->BlueMaskSize );
		else
			kdbg_printf( "\n" );
	}

	return( 0 );
}

int get_vesa_info()
{
	int			nR, nI;

	// ��� ����Ʈ�� ���Ѵ�.
	nR = get_mode_list( &gui_info.vesa );
	if( nR < 0 )
	{
		kdbg_printf( "get_mode_list() - error\n" );
		return( nR );
	}

	// ��庰 �� ������ ���Ѵ�.
	for( nI = 0; nI < gui_info.vesa.nTotalMode; nI++ )
		nR = get_mode_info( &gui_info.vesa.mode[nI] );

	return( 0 );
}

static int vesa_linear_mapping( DWORD dwLinear, DWORD dwPhys, DWORD dwSize )
{
	int			nR;
	DWORD		dwI;
	ThreadStt	*pThread;

	dwSize = (DWORD)( dwSize + 4095 );
	dwSize = (DWORD)( dwSize / 4096 ) * 4096;

	pThread = get_current_thread();
	if( pThread == NULL )
		return( -1 );
	
	for( dwI = 0; dwI <= dwSize; dwI += 4096 )
	{
		nR = forced_mapping( (DWORD*)get_thread_page_dir( pThread ), dwLinear, dwPhys );
		if( nR < 0 )
		{
			kdbg_printf( "vesa_linear_mapping() - mapping error! (Linear=0x%08X, Phys=0x%08X)\n", dwLinear, dwPhys );
			break;
		}
		dwLinear += 4096;
		dwPhys   += 4096;
	}

	return( 0 );
}

static VESAModeStt *find_vmode( DWORD dwMode )
{
	int			nI;

	for( nI = 0; nI < gui_info.vesa.nTotalMode; nI++ )
	{
		if( gui_info.vesa.mode[nI].wNo == (UINT16)dwMode )
			return( &gui_info.vesa.mode[nI] );
	}
	return( NULL );
}

// ��� �����ϰ� �޸� ������ �� �������� �ּҿ� �޸� ũ�⸦ �����Ѵ�. 
BYTE *set_vesa_mode( VESAModeStt *pVMode, DWORD *pMemSize )
{
	int		nR;
	BYTE	*pB;
	DWORD	dwSize, dwMode;

	dwMode = (DWORD)pVMode->wNo | (DWORD)0x4000;  // Bit14 = 1 (linear flat mode) Bit15 (Clear Screen)
	
	// �޸� ������ ��´�.
	if( pVMode->dwPhysBaseAddr != 0 )
	{
		if( pVMode->byBPP == 8 )
			dwSize = (DWORD)( pVMode->wX ) * (DWORD)( pVMode->wY );
		else if( pVMode->byBPP == 16 )
			dwSize = (DWORD)( pVMode->wX ) * (DWORD)( pVMode->wY ) * 2;
		else
			dwSize = (DWORD)( pVMode->wX ) * (DWORD)( pVMode->wY ) * 4;
		vesa_linear_mapping( VESA_LINEAR_BASE, pVMode->dwPhysBaseAddr, dwSize );
		pB = (BYTE*)VESA_LINEAR_BASE;

		// ���� �޸𸮸� �����.
		memset( pB, 0, dwSize );
	}
	else
	{	// Non-linear buffer mode
		dwSize = (DWORD)0xC0000 - (DWORD)0xA0000;
		pB = (BYTE*)0xA0000;
	}

	// �޸� ũ�⸦ �����Ѵ�. 
	if( pMemSize != NULL )
		pMemSize[0] = dwSize;

	// �ķ����͸� �����Ѵ�.
	nR = set_v86_param( V86FUNC_SET_VESA_MODE, dwMode );
	if( nR < 0 )
		return( NULL );
	
	// �ٷ� V86 �����带 Ȱ��ȭ�Ͽ� �����Ѵ�.
	call_to_v86_thread();

	return( pB );
}

int vesa_mode_test( DWORD dwMode )
{
	UINT16			*pW;
	BYTE			*pB;
	int				nX, nY;
	VESAModeStt		*pVMode;
	DWORD			dwSize, dwColor, *pD;

	// ��带 ã�´�.
	pVMode = find_vmode( dwMode );
	if( pVMode == NULL )
	{
		kdbg_printf( "set_video_mode() - mode 0x%X is not found!\n", dwMode );
		return( -1 );
	}			   	

	// V86 Function�� �̿��Ͽ� ��� ��ȯ�Ѵ�. 
	pB =  set_vesa_mode( pVMode, &dwSize );
	if( pB == NULL )
		return( -1 );

	dwColor = 0;
	pW = (UINT16*)pB;
	pD = (DWORD*)pB;

	for( nY = 0; nY < (int)pVMode->wY; nY++ )
	{
		for( nX = 0; nX < pVMode->wX; nX++ )
		{
			if( pVMode->byBPP <= 8 )
				pB[nX + nY * pVMode->wX ] = (BYTE)( dwColor & (BYTE)0xFF ); 
			else if( pVMode->byBPP <= 16 )
				pW[nX + nY * pVMode->wX ] = RGB16( 127,127, 255 );
			else
				pD[nX + nY * pVMode->wX ] = RGB32(152, 152, 232);//dwColor;
			dwColor++;
		}					  
	}

	getchar();

	// ǥ������ ������.
	lines_xx( get_vertical_line_size() );

	kdbg_printf( "VMODE [0x%X] %d * %d (BPP=%d) Size=%d\n", dwMode, pVMode->wX, pVMode->wY, pVMode->byBPP, dwSize );

	return( 0 );
}

typedef int (*GUI_START_FUNC)			( VESAModeStt *pVM );
typedef int (*GUI_END_FUNC)				();
static char	*pGuiStartFuncName 		= "enter_gui";
static char	*pGuiEndFuncName   		= "leave_gui";
static char *pGuiThreadName    		= "gui_thread";
// GUI ��忡�� �ٽ� Text ���� ���ƿ´�.
// gui_thsread�� ã�� �ñ׳��� ������.
// KBD Handler���� ȣ��ǹǷ� interrupt disable�� ����.
int end_gui()
{
	ThreadStt		*pT;
	
	if( gui_info.nGuiFlag == 0 )
	{
		kdbg_printf( "\nsystem is not gui mode.\n" );
		return( -1 );
	}
	
	// gui.mod���� ������ �����带 ã�� �ñ׳��� ������.
	pT = find_thread_by_alias( pGuiThreadName );
	if( pT == NULL )
	{	// gui �����带 ã�� �� ����. 
		kdbg_printf( "gui thread not found!\n" );
		return( -1 );
	}

	// �ñ׳��� ������.
	send_signal_to_thread( pT->dwID, SIG_USER_0 );

	return( 0 );
}

int is_gui_mode()
{
	return( gui_info.nGuiFlag );
}

// �ػ󵵸� ������ �� GUI���� ��ȯ�Ѵ�. 
int start_gui( DWORD dwMode )
{
	int				nR;
	ModuleStt		*pM;
	GUI_START_FUNC	pFunc;
	VESAModeStt 	*pVideoMode;
	
	if( is_gui_mode() != 0 )
	{
		kdbg_printf( "system is already in gui mode.\n" );
		return( -1 );
	}

	// GUI ����� �ε�Ǿ� �ִ��� Ȯ���Ѵ�. 
	pM = find_module_by_alias( "gui.mod", NULL );
	if( pM == NULL )
	{	// GUI.MOD�� ���� �ε��Ͽ��� �Ѵ�. 
		kdbg_printf( "Module gui.mod is not found!\n" );
		return( -1 );
	}

	// "vmode" ����� �� ���� �������� ���� ���
	if( gui_info.vesa.nValidFlag == 0 )
	{
		nR = get_vesa_info( &gui_info.vesa );
		if( nR < 0 )
		{
			kdbg_printf( "No available video mode!\n" );
			return( nR );
		}
	}

	// ��带 ã�� ���� ���� ���̸� �׳� ���ư���. 
	pVideoMode = find_vmode( dwMode );
	if( pVideoMode == NULL )
	{
		kdbg_printf( "video mode 0x%X is not found!\n", dwMode );
		return( -1 );
	}
	
	// BPP�� 16�� �ƴϸ� �׳� ���ư���. 
	if( pVideoMode->byBPP != 16 || pVideoMode->dwPhysBaseAddr == 0 || (pVideoMode->wAttr & VMODE_GRAPHIC) == 0 )
	{
		kdbg_printf( "video mode 0x%X is not available!\n", dwMode );
		return( -1 );
	}

	// gui.mod�� change_screen_res() �Լ��� ã�´�. 
	pFunc = (GUI_START_FUNC)find_function_address( pM, pGuiStartFuncName );
	if( pFunc == NULL )
	{
		kdbg_printf( "Function %s() not found!\n", pGuiStartFuncName );
		return( -1 );
	}

	// �Լ��� ȣ���Ѵ�. 
	nR = pFunc( pVideoMode );
	if( nR < 0 )
	{
		kdbg_printf( "enter_gui() : err_code = %d\n", nR );  	  
	    return( -1 );
	}
	else
		kdbg_printf( "enter_gui() : ok\n" );

			
	// �̰��� ���⼭ �� �� �ʿ䰡 ����.  
	// enter_gui()���� set_system_gui_info()�� ���� �����ȴ�.
	//gui_info.nGuiFlag = 1;
	
	return( 0 );
}

