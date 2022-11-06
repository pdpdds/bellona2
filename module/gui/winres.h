#ifndef BELLONA2_WINRES_HEADER_jj
#define BELLONA2_WINRES_HEADER_jj

#ifdef __cplusplus
extern "C" {
#endif	

typedef struct {
	BYTE        bWidth;          // Width of the image     
	BYTE        bHeight;         // Height of the image (times 2)     
	BYTE        bColorCount;     // Number of colors in image (0 if >=8bpp)     
	BYTE        bReserved;       // Reserved     
	WORD        wPlanes;         // Color Planes     
	WORD        wBitCount;       // Bits per pixel     
	DWORD       dwBytesInRes;    // how many bytes in this resource?
	DWORD       dwImageOffset;   // where in the file is this image 
} ICONDIRENTRY;

typedef struct  
{     
	WORD		   idReserved;   	// Reserved     
	WORD           idType;       	// resource type (1 for icons)     
	WORD           idCount;      	// how many images?     
	ICONDIRENTRY   idEntries[1]; 	// entries for each image (idCount of 'em) 
} ICONDIR;

#pragma pack( push )
#pragma pack( 2 )
typedef struct
{
   BYTE   bWidth;               // Width, in pixels, of the image
   BYTE   bHeight;              // Height, in pixels, of the image
   BYTE   bColorCount;          // Number of colors in image (0 if >=8bpp)
   BYTE   bReserved;            // Reserved
   WORD   wPlanes;              // Color Planes
   WORD   wBitCount;            // Bits per pixel
   DWORD  dwBytesInRes;         // how many bytes in this resource?
   WORD   nID;                  // the ID
} GRPICONDIRENTRY, *LPGRPICONDIRENTRY;
#pragma pack( pop )

#ifndef _RTEST_ /////////////////////////////////////////////////////////////////
typedef struct tagRGBQUAD {
  BYTE    rgbBlue; 
  BYTE    rgbGreen; 
  BYTE    rgbRed; 
  BYTE    rgbReserved; 
} RGBQUAD; 

typedef struct {
  DWORD  biSize; 
  LONG   biWidth; 
  LONG   biHeight; 
  WORD   biPlanes; 
  WORD   biBitCount; 
  DWORD  biCompression; 
  DWORD  biSizeImage; 
  LONG   biXPelsPerMeter; 
  LONG   biYPelsPerMeter; 
  DWORD  biClrUsed; 
  DWORD  biClrImportant; 
} BITMAPINFOHEADER;

typedef struct {
  BITMAPINFOHEADER bmiHeader; 
  RGBQUAD          bmiColors[1]; 
} BITMAPINFO;
#endif ////////////////////////////////////////////////////////////////////////////
  


typedef struct {
	unsigned short int wSize;
	unsigned short int ustr[1];
} UniStrStt;	// UnicodeString

extern BELL_EXPORT int win_resource	( unsigned long dwModule, WinResStt *pWR );

extern ResObjStt 		*kalloc_res_obj					( DWORD dwType );

extern int 				preload_sys_icon				();
extern int 				release_preload_sys_icon		();
extern struct ImageTag 	*get_sys_icon					( int nID );	
extern char 			*get_res_name					( unsigned short int wId, WinResStt *pWR );
extern WinResEntStt 	*load_winres					( WinResStt *pWR, unsigned short int wResID );
extern int 				load_cursor						( WinResStt *pWR, void *pVoidBC, unsigned short int wID );

#ifdef __cplusplus
}
#endif	


#endif
