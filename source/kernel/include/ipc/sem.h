/**
 * �����ź���
 *
 * ���ߣ�����ͭ
 * ��ϵ����: 527676163@qq.com
 */
#ifndef OS_SEM_H
#define OS_SEM_H

#include "tools/list.h"

/**
 * ����ͬ���õļ����ź���
 */
typedef struct _sem_t {
    int count;				// �ź�������
    list_t wait_list;		// �ȴ��Ľ����б�
}sem_t;

void sem_init (sem_t * sem, int init_count);
void sem_wait (sem_t * sem);
void sem_notify (sem_t * sem);
int sem_count (sem_t * sem);

#endif //OS_SEM_H
