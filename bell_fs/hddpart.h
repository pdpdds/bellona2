#ifndef HDD_PARTITION_HEADER_jj
#define HDD_PARTITION_HEADER_jj

typedef struct PartitionTag {
    UCHAR   byActive;
    UCHAR   byStartingHead;
	unsigned short  wStartingCylinderSector;
    UCHAR   byType;
    UCHAR   byEndingHead;
    unsigned short  wEndingCylinderSector;
    DWORD   dwStartingSector;
    DWORD   dwTotalSector;
};
typedef struct PartitionTag PartitionStt;

typedef struct HddPartTag {
	PartitionStt	p;
	void			*pBaseDevObj;		// partition�� �����ϴ� ���� ��� ����̽� ����̹�
};
typedef struct HddPartTag HddPartStt;

extern int init_hdd_part_driver();

extern int close_hdd_part_driver();

#endif
