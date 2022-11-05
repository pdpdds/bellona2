#ifndef BELLONA2_STDINOUT_HEADER_jj
#define BELLONA2_STDINOUT_HEADER_jj

extern int std_in_read( struct VNodeTag *pVNode, DWORD dwOffs, char *pBuff, int nSize );
extern int std_out_write( struct VNodeTag *pVNode, DWORD dwOffs, char *pBuff, int nSize );
extern int std_err_write( struct VNodeTag *pVNode, DWORD dwOffs, char *pBuff, int nSize );

#endif