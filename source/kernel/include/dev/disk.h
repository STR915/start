/**
 * ��������
 *
 * ���ߣ�����ͭ
 * ��ϵ����: 527676163@qq.com
 */
#ifndef DISK_H
#define DISK_H

#include "comm/types.h"
#include "ipc/mutex.h"
#include "ipc/sem.h"

#define PART_NAME_SIZE              32      // ��������
#define DISK_NAME_SIZE              32      // �������ƴ�С
#define DISK_CNT                    2       // ���̵�����
#define DISK_PRIMARY_PART_CNT       (4+1)       // ��������������4��
#define DISK_PER_CHANNEL            2       // ÿͨ����������

// https://wiki.osdev.org/ATA_PIO_Mode#IDENTIFY_command
// ֻ����֧�����ܽ�primary bus
#define IOBASE_PRIMARY              0x1F0
#define	DISK_DATA(disk)				(disk->port_base + 0)		// ���ݼĴ���
#define	DISK_ERROR(disk)			(disk->port_base + 1)		// ����Ĵ���
#define	DISK_SECTOR_COUNT(disk)		(disk->port_base + 2)		// ���������Ĵ���
#define	DISK_LBA_LO(disk)			(disk->port_base + 3)		// LBA�Ĵ���
#define	DISK_LBA_MID(disk)			(disk->port_base + 4)		// LBA�Ĵ���
#define	DISK_LBA_HI(disk)			(disk->port_base + 5)		// LBA�Ĵ���
#define	DISK_DRIVE(disk)			(disk->port_base + 6)		// ���̻��ͷ��
#define	DISK_STATUS(disk)			(disk->port_base + 7)		// ״̬�Ĵ���
#define	DISK_CMD(disk)				(disk->port_base + 7)		// ����Ĵ���

// ATA����
#define	DISK_CMD_IDENTIFY				0xEC	// IDENTIFY����
#define	DISK_CMD_READ					0x24	// ������
#define	DISK_CMD_WRITE					0x34	// д����

// ״̬�Ĵ���
#define DISK_STATUS_ERR          (1 << 0)    // �����˴���
#define DISK_STATUS_DRQ          (1 << 3)    // ׼���ý������ݻ����������
#define DISK_STATUS_DF           (1 << 5)    // ��������
#define DISK_STATUS_BUSY         (1 << 7)    // ��æ

#define	DISK_DRIVE_BASE		    0xE0		// �������Ż���ֵ:0xA0 + LBA

#pragma pack(1)

/**
 * MBR�ķ�����������
 */
typedef struct _part_item_t {
    uint8_t boot_active;               // �����Ƿ�
	uint8_t start_header;              // ��ʼheader
	uint16_t start_sector : 6;         // ��ʼ����
	uint16_t start_cylinder : 10;	    // ��ʼ�ŵ�
	uint8_t system_id;	                // �ļ�ϵͳ����
	uint8_t end_header;                // ����header
	uint16_t end_sector : 6;           // ��������
	uint16_t end_cylinder : 10;        // �����ŵ�
	uint32_t relative_sectors;	        // ����ڸ���������ʼ�����������
	uint32_t total_sectors;            // �ܵ�������
}part_item_t;

#define MBR_PRIMARY_PART_NR	    4   // 4��������

/**
 * MBR���������ṹ
 */
typedef  struct _mbr_t {
	uint8_t code[446];                 // ����������
    part_item_t part_item[MBR_PRIMARY_PART_NR];
	uint8_t boot_sig[2];               // ������־
}mbr_t;

#pragma pack()

struct _disk_t;

/**
 * @brief ��������
 */
typedef struct _partinfo_t {
    char name[PART_NAME_SIZE]; // ��������
    struct _disk_t * disk;      // �����Ĵ���

    // https://www.win.tue.nl/~aeb/partitions/partition_types-1.html
    enum {
        FS_INVALID = 0x00,      // ��Ч�ļ�ϵͳ����
        FS_FAT16_0 = 0x06,      // FAT16�ļ�ϵͳ����
        FS_FAT16_1 = 0x0E,
    }type;

	int start_sector;           // ��ʼ����
	int total_sector;           // ������
}partinfo_t;

/**
 * @brief ���̽ṹ
 */
typedef struct _disk_t {
    char name[DISK_NAME_SIZE];      // ��������

    enum {
        DISK_DISK_MASTER = (0 << 4),     // ���豸
        DISK_DISK_SLAVE = (1 << 4),      // ���豸
    }drive;

    uint16_t port_base;             // �˿���ʼ��ַ
    int sector_size;                // ���С
    int sector_count;               // ����������
	partinfo_t partinfo[DISK_PRIMARY_PART_CNT];	// ������, ����һ�������������̵ļٷ�����Ϣ
    mutex_t * mutex;              // ���ʸ�֪ͨ�Ļ����ź���
    sem_t * op_sem;               // ��д���������ͬ���ź���
}disk_t;

void disk_init (void);

void exception_handler_ide_primary (void);

#endif // DISK_H
