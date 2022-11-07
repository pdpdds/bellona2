#ifndef DEVICE_DRIVER_MAJOR_NUMBER
#define DEVICE_DRIVER_MAJOR_NUMBER

// BLOCK DEVICE
typedef enum {
	RAM_DISK_MAJOR	= 1,		// ram disk 
	IDE_HDD_MAJOR,				// ide hdd 
	FDD35_MAJOR,				// fdd 3.5 inch
	HDD_PART_MAJOR,				// hdd partition  
	FDD_FAT12_MAJOR,			// fat12 on fdd 
	FAT16_PART_MAJOR,			// fat16 on partition
	
	END_OF_MAJOR
} BLOCK_DEVICE_DRIVER_MAJOR_NUMBER;

// CHARACTER DEVICE
typedef enum {
	SERIAL_PORT_MAJOR	= 1,	// serial com port
	LOGICAL_KBD_MAJOR,			// logical keyboard
	TTY_MAJOR,					// tty major

	END_OF_CHAR_DEV
} CHAR_DEV_MAJOR;
	   


#endif
