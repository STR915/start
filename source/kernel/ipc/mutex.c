/**
 * ������
 *
 * ���ߣ�����ͭ
 * ��ϵ����: 527676163@qq.com
 */
#include "cpu/irq.h"
#include "ipc/mutex.h"

/**
 * ����ʼ��
 */
void mutex_init (mutex_t * mutex) {
    mutex->locked_count = 0;
    mutex->owner = (task_t *)0;
    list_init(&mutex->wait_list);
}

/**
 * ������
 */
void mutex_lock (mutex_t * mutex) {
    irq_state_t  irq_state = irq_enter_protection();

    task_t * curr = task_current();
    if (mutex->locked_count == 0) {
        // û������ռ�ã�ռ��֮
        mutex->locked_count = 1;
        mutex->owner = curr;
    } else if (mutex->owner == curr) {
        // �Ѿ�Ϊ��ǰ�������У�ֻ���Ӽ���
        mutex->locked_count++;
    } else {
        // ����������ռ�ã��������еȴ�
        task_t * curr = task_current();
        task_set_block(curr);
        list_insert_last(&mutex->wait_list, &curr->wait_node);
        task_dispatch();
    }

    irq_leave_protection(irq_state);
}

/**
 * �ͷ���
 */
void mutex_unlock (mutex_t * mutex) {
    irq_state_t  irq_state = irq_enter_protection();

    // ֻ������ӵ���߲����ͷ���
    task_t * curr = task_current();
    if (mutex->owner == curr) {
        if (--mutex->locked_count == 0) {
            // ����0���ͷ���
            mutex->owner = (task_t *)0;

            // ���������������ȴ������������Ѳ�ռ����
            if (list_count(&mutex->wait_list)) {
                list_node_t * task_node = list_remove_first(&mutex->wait_list);
                task_t * task = list_node_parent(task_node, task_t, wait_node);
                task_set_ready(task);

                // ������ռ�ã��������������Ѻ�ռ�ã���Ϊ����������
                mutex->locked_count = 1;
                mutex->owner = task;

                task_dispatch();
            }
        }
    }

    irq_leave_protection(irq_state);
}
