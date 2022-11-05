#ifndef BELLONA2_KBD_CHAR_DRV_HEADER_jj
#define BELLONA2_KBD_CHAR_DRV_HEADER_jj

typedef struct LogicalKbdTag {
	struct ThreadTag *pThread;
};
typedef struct LogicalKbdTag LogicalKbdStt;

extern int init_logical_kbd_driver ( int nMajor );

#endif
