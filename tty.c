#include "bellona2.h"

static CharDevObjStt tty;

static int tty_open( int nMinor, void *pCharDevObj, void *pDev )
{
	return( 0 );
}

static int tty_close( void *pCharDevObj )
{
	return( 0 );
}

static int tty_read( void *pCharDevObj )
{
	return( 0 );
}

static int tty_write( void *pCharDevObj, UCHAR *pB, int nSize )
{
	return( 0 );
}

static int tty_ioctl( void *pCharDevObj, void *pV )
{
	return( 0 );
}

int init_tty_driver ( int nMajor )
{
	int			nR;
	CharDevStt	tty;

	memset( &tty, 0, sizeof( tty ) );

	strcpy( tty.szName, "TTY" );
	tty.nMajor		= nMajor;
	tty.op.open		= tty_open ;
	tty.op.close	= tty_close;
	tty.op.read		= tty_read ;
	tty.op.write	= tty_write;
	tty.op.ioctl	= tty_ioctl;

	// register
	nR = register_chardev( &tty );
	
	return( nR );
}
