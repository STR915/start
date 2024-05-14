/**
 * ������
 *
 * ���ߣ�����ͭ
 * ��ϵ����: 527676163@qq.com
 */
#ifndef MUTEX_H
#define MUTEX_H

#include "core/task.h"
#include "tools/list.h"

/**
 * ����ͬ���õļ����ź���
 */
typedef struct _mutex_t {
    task_t * owner;
    int locked_count;
    list_t wait_list;
}mutex_t;

void mutex_init (mutex_t * mutex);
void mutex_lock (mutex_t * mutex);
void mutex_unlock (mutex_t * mutex);
void test(void);
#endif //MUTEX_H
