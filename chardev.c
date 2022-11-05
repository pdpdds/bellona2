#include "bellona2.h"

static CharDeviceStt chardev;

CharDevObjStt *find_first_chardev_obj( int *pTotal )
{
	CharDevObjStt	*pObj;

	pObj	= chardev.pObjStart;
	*pTotal = chardev.nTotalObj;
	
	return( pObj );
}

int register_chardev( CharDevStt *pC )
{
	CharDevStt	*pT;

	if( pC->nMajor <= 0 || pC->nMajor >= MAX_CHAR_DEV )
	{	// invalid MAJOR NUMBER
		return( -1 );
	}

	if( chardev.ent[ pC->nMajor ] != NULL )
	{	// the MAJOR_NUMBER is already being used.
		return( -1 );
	}

	// allocate memory
	pT = (CharDevStt*)MALLOC( sizeof( CharDevStt ) );
	if( pT == NULL )
	{	// memory allocation failed.
		return( -1 );
	}

	memcpy( pT, pC, sizeof( BlkDevStt ) );

	chardev.ent[pC->nMajor] = pT;
	chardev.nTotal++;

	return( 0 );
}

int unregister_chardev( CharDevStt *pC )
{
	if( pC->nMajor <= 0 || pC->nMajor >= MAX_CHAR_DEV )
	{	// invalid MAJOR NUMBER
		return( -1 );
	}

	if( chardev.ent[ pC->nMajor ] == NULL )
	{	// the block device is not being used.
		return( -1 );
	}

	FREE( chardev.ent[pC->nMajor] );
	chardev.ent[pC->nMajor] = NULL;
	chardev.nTotal--;

	return( 0 );
}

int open_char_device( CharDevObjStt *pObj, int nMajor, int nMinor )
{
	CharDevStt	*pDev;
	int			nR;

	pDev = chardev.ent[nMajor];
	if( pDev == NULL )			// device is not registered.
		return( -1 );

	// dispatch open call
	nR = pDev->op.open( nMinor, pObj, pDev );
	if( nR == -1 )
		return( -1 );

	// insert the opened char device to object list
	if( chardev.pObjEnd == NULL )		
	{
		chardev.pObjStart  = chardev.pObjEnd = pObj;
		pObj->pPre = pObj->pNext = NULL;
	}
	else
	{
		pObj->pPre				= chardev.pObjEnd;
		pObj->pNext				= NULL;
		chardev.pObjEnd->pNext	= pObj;
		chardev.pObjEnd			= pObj;
	}

	chardev.nTotalObj++;

	return( 0 );
}

// read character device
int read_char_device( CharDevObjStt *pObj )
{
	int nR;

	if( pObj == NULL || pObj->pDev == NULL || pObj->pDev->op.read == NULL )
		return( -1 );

	nR = pObj->pDev->op.read( pObj );

	return( nR );
}

// write character device
int write_char_device( CharDevObjStt *pObj, UCHAR *pB, int nSize )
{
	int nR;

	if( pObj == NULL || pObj->pDev == NULL || pObj->pDev->op.write == NULL )
		return( -1 );

	nR = pObj->pDev->op.write( pObj, pB, nSize );

	return( nR );
}


// close character device 
int close_char_device( CharDevObjStt *pObj )
{
	CharDevObjStt	*pT;
	int				nR;

	nR = pObj->pDev->op.close( pObj );

	if( pObj->pPre != NULL )
	{
		pT = pObj->pPre;
		pT->pNext = pObj->pNext;
		if( pT->pNext == NULL )
			chardev.pObjEnd = pT;
	}

	if( pObj->pNext != NULL )
	{
		pT = pObj->pNext;
		pT->pPre = pObj->pPre;
		if( pT->pPre == NULL )
			chardev.pObjStart = pT;
	}

	chardev.nTotalObj--;
	
	return( nR );
}

static CharDevObjStt k;
// initialize character device structure and register default char device
int init_char_dev()
{
	memset( &chardev, 0, sizeof( CharDeviceStt ) );

	// register standard drivers
	init_serial_port_driver ( SERIAL_PORT_MAJOR );
	init_serial_port_driver ( LOGICAL_KBD_MAJOR	);
	init_serial_port_driver ( TTY_MAJOR			);

	// open Keyboard driver
	open_char_device( &k, LOGICAL_KBD_MAJOR, 0 );

	return( 0 );
}







