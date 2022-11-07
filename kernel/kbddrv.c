// it is not a real keyboard driver but a dummy logical wrapper.
#include "bellona2.h"

static int logical_kbd_open( int nMinor, void *pVoidObj, void *pVDev )
{
	LogicalKbdStt	*pLK;
	CharDevObjStt	*pObj;
	ThreadStt		*pThread;

	// get current thread
	pThread = get_current_thread();
	if( pThread == NULL )
		return( -1 );

	pObj = (CharDevObjStt*)pVoidObj;

	pLK = (LogicalKbdStt*)MALLOC( sizeof( LogicalKbdStt ) );
	if( pLK == NULL )
		return( -1 );	// memory allocation failed

	memset( pLK, 0, sizeof( LogicalKbdStt ) );
	pLK->pThread = pThread;	// save thread handle

	pObj->pPtr	 = pLK;
	pObj->pDev	 = pVDev;
	pObj->nMinor = nMinor;

	return( 0 );
}
static int logical_kbd_close( void *pCharDevObj )
{
	CharDevObjStt *pObj;

	pObj = pCharDevObj;

	if( pObj == NULL || pObj->pPtr == NULL )
		return( -1 );

	FREE( pObj->pPtr );
	
	return( 0 );
}
static int logical_kbd_read( void *pCharDevObj )
{
	int				nR;
	BKeyStt			key;
	LogicalKbdStt	*pLK;
	CharDevObjStt	*pObj;

	pObj = pCharDevObj;
	pLK  = pObj->pPtr;
	if( pLK->pThread == NULL )
		return( -1 );

	// get key from thread key buffer
 	nR = sub_key_from_thread( pLK->pThread, &key );
	if( nR == 1 )
		return( (int)key.byCode );

	return( -1 );		// no kbd q
}
static int logical_kbd_write( void *pCharDevObj, UCHAR *pB, int nSize )
{
	return( 0 );
}
static int logical_kbd_ioctl( void *pCharDevObj, void *pV )
{
	return( 0 );
}

int init_logical_kbd_driver ( int nMajor )
{
	int			nR;
	CharDevStt	k;

	memset( &k, 0, sizeof( k ) );

	strcpy( k.szName, "KEYBOARD" );
	k.nMajor	= nMajor;
	k.op.open	= logical_kbd_open ;
	k.op.close	= logical_kbd_close;
	k.op.read	= logical_kbd_read ;
	k.op.write	= logical_kbd_write;
	k.op.ioctl	= logical_kbd_ioctl;

	// register
	nR = register_chardev( &k );
	
	return( nR );
}

