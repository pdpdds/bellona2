#ifndef BELLONA2_FDD_HEADER_oh
#define BELLONA2_FDD_HEADER_oh

#define FDD_DIGITAL_OUTPUT 0x3F2
#define FDD_STATUS_PORT    0x3F4
#define FDD_DATA_PORT      0x3F5

extern int  fdd_reset();
extern void fdd_handler();
extern int  fdd_motor_on();
extern int  fdd_motor_off();
extern int  fdd_read_test();
extern int  fdd_write_test();
extern int  fdd_set_ready( int nDrv );
extern int  fdd_read_track( int nDrv, int nTrack, int nSide, char *pBuff );
extern int  fdd_write_track( int nDrv, int nTrack, int nSide, char *pBuff );

#endif
