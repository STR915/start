/**
 * os����
 *
 * ���ߣ�����ͭ
 * ��ϵ����: 527676163@qq.com
 */
#ifndef OS_OS_CFG_H
#define OS_OS_CFG_H

#define GDT_TABLE_SIZE      	256		// GDT��������
#define KERNEL_SELECTOR_CS		(1 * 8)		// �ں˴����������
#define KERNEL_SELECTOR_DS		(2 * 8)		// �ں����ݶ�������
#define KERNEL_STACK_SIZE       (8*1024)    // �ں�ջ
#define SELECTOR_SYSCALL     	(3 * 8)	// �����ŵ�ѡ����

#define OS_TICK_MS              10       	// ÿ�����ʱ����

#define OS_VERSION              "0.0.1"     // OS�汾��

#define IDLE_STACK_SIZE       1024        // ��������ջ

#define TASK_NR             128            // ���̵�����

#define ROOT_DEV            DEV_DISK, 0xb1  // ��Ŀ¼���ڵ��豸

#endif //OS_OS_CFG_H
