#ifndef B2OS_MEMORY_MAP_HEADER_jj
#define B2OS_MEMORY_MAP_HEADER_jj

#define V86LIB_PARAM_ADDR	( 1024 * 64 )   // 이걸 변경하면 안된다.
#define V86LIB_ADDR 		( V86LIB_PARAM_ADDR + 0x100 )
#define V86LIB_SIZE 		( 1024 * 64 ) 	// 크기는 64k

#define HDD_DMA_BUFF_ADDR	( 1024 * 128 )
#define FDD_DMA_BUFF_ADDR	( 1024 * 192 )

#endif


