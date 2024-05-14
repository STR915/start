/**
 * ��x86����ϵ�ṹ��صĽӿڼ�����
 *
 * ���ߣ�����ͭ
 * ��ϵ����: 527676163@qq.com
 */
#ifndef CPU_H
#define CPU_H

#include "comm/types.h"

#define SEG_G				(1 << 15)		// ���öν��޵ĵ�λ��1-4KB��0-�ֽ�
#define SEG_D				(1 << 14)		// �����Ƿ���32λ��16λ�Ĵ�������ݶ�
#define SEG_P_PRESENT	    (1 << 7)		// ���Ƿ����

#define SEG_DPL0			(0 << 5)		// ��Ȩ��0�������Ȩ��
#define SEG_DPL3			(3 << 5)		// ��Ȩ��3�����Ȩ��

#define SEG_S_SYSTEM		(0 << 4)		// �Ƿ���ϵͳ�Σ�������Ż����ж�
#define SEG_S_NORMAL		(1 << 4)		// ��ͨ�Ĵ���λ����ݶ�

#define SEG_TYPE_CODE		(1 << 3)		// ָ����Ϊ�����
#define SEG_TYPE_DATA		(0 << 3)		// ���ݶ�

#define SEG_TYPE_RW			(1 << 1)		// �Ƿ��д�ɶ���������Ϊֻ��
#define SEG_TYPE_TSS      	(9 << 0)		// 32λTSS


#define GATE_TYPE_IDT		(0xE << 8)		// �ж�32λ��������
#define GATE_TYPE_SYSCALL	(0xC << 8)		// ������
#define GATE_P_PRESENT		(1 << 15)		// �Ƿ����
#define GATE_DPL0			(0 << 13)		// ��Ȩ��0�������Ȩ��
#define GATE_DPL3			(3 << 13)		// ��Ȩ��3�����Ȩ��

#define SEG_CPL0                (0 << 0)
#define SEG_CPL3                (3 << 0)

#define EFLAGS_IF           (1 << 9)
#define EFLAGS_DEFAULT      (1 << 1)

#pragma pack(1)

/**
 * GDT������
 */
typedef struct _segment_desc_t {
	uint16_t limit15_0;
	uint16_t base15_0;
	uint8_t base23_16;
	uint16_t attr;
	uint8_t base31_24;
}segment_desc_t;

/*
 * ������������
 */
typedef struct _gate_desc_t {
	uint16_t offset15_0;
	uint16_t selector;
	uint16_t attr;
	uint16_t offset31_16;
}gate_desc_t;

/**
 * tss������
 */
typedef struct _tss_t {
    uint32_t pre_link;//ǰһ����������
    uint32_t esp0, ss0, esp1, ss1, esp2, ss2;//ջ�й� ss��Ȩ��
    uint32_t cr3;//�����ҳ��
    uint32_t eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;//cpu�Ĵ��������Ϣ
    uint32_t es, cs, ss, ds, fs, gs;//�μĴ���
    uint32_t ldt;//
    uint32_t iomap;//
}tss_t;
#pragma pack()

void cpu_init (void);
void segment_desc_set(int selector, uint32_t base, uint32_t limit, uint16_t attr);
void gate_desc_set(gate_desc_t * desc, uint16_t selector, uint32_t offset, uint16_t attr);
int  gdt_alloc_desc (void);
void gdt_free_sel (int sel);

void switch_to_tss (uint32_t tss_sel);

#endif

