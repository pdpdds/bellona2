#ifndef BELLONA2_VESA_HEADER
#define  BELLONA2_VESA_HEADER

#define VESA_LINEAR_BASE	( 0x80000000 - 0x2000000 )  // 2GB 바로 32M 아래

typedef struct {
	DWORD					VbeSignature;        
	unsigned short int		VbeVersion;          
	DWORD					OemStringPtr;        
	BYTE					Capabilities[4];     
	DWORD					VideoModePtr;        
	unsigned short int		TotalMemory;         
	unsigned short int		OemSoftwareRev;      
	DWORD					OemVendorNamePtr;    
	DWORD					OemProductNamePtr;   
	DWORD					OemProductRevPtr;    
	char					Reserved[222];       
	char					OemData[256];        
} VBEInfoBlockStt;

typedef struct {
	unsigned short int		ModeAttributes;
	char					WinAAttributes;
	char					WinBAttributes;
	unsigned short int		WinGranularity;
	unsigned short int		WinSize;
	unsigned short int		WinASegment;
	unsigned short int		WinBSegment;
	DWORD					WinFuncPtr;
	unsigned short int		BytesPerScanLine;
	unsigned short int		XResolution;
	unsigned short int		YResolution;
	char					XCharSize;
	char					YCharSize;
	char					NumberOfPlanes;
	char					BitsPerPixel;
	char					NumberOfBanks;
	char					MemoryModel;
	char					BankSize;
	char					NumberOfImagePages;
	char					Reserved0;
	char					RedMaskSize;
	char					RedFieldPosition;
	char					GreenMaskSize;
	char					GreenFieldPosition;
	char					BlueMaskSize;
	char					BlueFieldPosition;
	char					RsvdMaskSize;
	char					RsvdFieldPosition;
	char					DirectColorModeInfo;
	DWORD					PhysBasePtr;
	DWORD					Reserved1;
	unsigned short int		Reserved2;
	unsigned short int		LinBytesPerScanLine;
	char					BnkNumberOfImagePages;
	char					LinNumberOfImagePages;
	char					LinRedMaskSize;
	char					LinRedFieldPosition;
	char					LinGreenMaskSize;
	char					LinGreenFieldPositiondb;
	char					LinBlueMaskSize;
	char					LinBlueFieldPosition;
	char					LinRsvdMaskSize;
	char					LinRsvdFieldPosition;
	DWORD					MaxPixelClock;
	char					Reserved3[189];
} ModeInfoBlockStt;

#define MAX_VESA_MODE	64

typedef struct {
	unsigned short int	wNo;	// Mode Number
	unsigned short int	wAttr;	// Attribute
	unsigned short int	wX;		// X Resolution
	unsigned short int	wY;		// Y Resolution
	BYTE				byBPP;	// Bits Per Pixel
	DWORD				dwPhysBaseAddr;
	unsigned short int  LinBytesPerScanLine;
	char				RedMaskSize;
	char				GreenMaskSize;
	char				BlueMaskSize;
}VESAModeStt;

typedef struct {
	int					nValidFlag;
	DWORD				dwTotalMemory;
	unsigned short int	wRevision;
	char				szVendor[64];
	char				szProduct[64];
	int					nTotalMode;
	VESAModeStt			mode[MAX_VESA_MODE];
} VESAStt;


typedef struct GuiExportTag {
	struct KShlFuncTag	*pFTbl;
} GuiExportStt;

typedef struct GuiInfoTag {
	int 				nGuiFlag;
	VESAStt				vesa;	
	GuiExportStt		*pGuiExp;
} GuiInfoStt;

/*=====================================================================================*/ 
extern BELL_EXPORT void set_system_gui_info		( int nNewFlag, GuiExportStt *pGuiExp );
extern BELL_EXPORT BYTE *set_vesa_mode			( VESAModeStt *pVMode, DWORD *pMemSize );
extern BELL_EXPORT int is_gui_mode				();

extern int end_gui					();
extern int get_vesa_info			();
extern int disp_vesa_info			();
extern int start_gui				( DWORD dwmode );
extern int vesa_mode_test			( DWORD dwMode );

extern GuiExportStt *get_gui_exp	();

#endif
