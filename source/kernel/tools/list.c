#include "tools/list.h"

/**
 * ��ʼ������
 * @param list ����ʼ��������
 */
void list_init(list_t *list) {
    list->first = list->last = (list_node_t *)0;
    list->count = 0;
}

/**
 * ��ָ��������뵽ָ�������ͷ��
 * @param list �����������
 * @param node ������Ľ��
 */
void list_insert_first(list_t *list, list_node_t *node) {
    // ���úô��������ǰ��ǰ��Ϊ��
    node->next = list->first;
    node->pre = (list_node_t *)0;

    // ���Ϊ�գ���Ҫͬʱ����first��lastָ���Լ�
    if (list_is_empty(list)) {
        list->last = list->first = node;
    } else {
        // �������ú�ԭ����һ������pre
        list->first->pre = node;

        // ����firstָ��
        list->first = node;
    }

    list->count++;
}


/**
 * ��ָ��������뵽ָ�������β��
 * @param list ����������
 * @param node ������Ľ��
 */
void list_insert_last(list_t *list, list_node_t *node) {
    // ���úý�㱾��
    node->pre = list->last;
    node->next = (list_node_t*)0;

    // ��գ���first/last��ָ��Ψһ��node
    if (list_is_empty(list)) {
        list->first = list->last = node;
    } else {
        // ���򣬵���last������һָ��Ϊnode
        list->last->next = node;

        // node������µĺ�̽��
        list->last = node;
    }

    list->count++;
}

/**
 * �Ƴ�ָ�������ͷ��
 * @param list ����������
 * @return ����ĵ�һ�����
 */
list_node_t* list_remove_first(list_t *list) {
    // ����Ϊ�գ����ؿ�
    if (list_is_empty(list)) {
        return (list_node_t*)0;
    }

    // ȡ��һ�����
    list_node_t * remove_node = list->first;

    // ��first����β��1���������ղ��ƹ����Ǹ������û�к�̣���first=0
    list->first = remove_node->next;
    if (list->first == (list_node_t *)0) {
        // nodeΪ���һ�����
        list->last = (list_node_t*)0;
    } else {
        // �����һ��㣬����̵�ǰ����0
        remove_node->next->pre = (list_node_t *)0;
    }

    // ����node�Լ�����0����Ϊû�к�̽��
    remove_node->next = remove_node->pre = (list_node_t*)0;

    // ͬʱ��������ֵ
    list->count--;
    return remove_node;
}

/**
 * �Ƴ�ָ��������еı���
 * �����node�Ƿ��ڽ����
 */
list_node_t * list_remove(list_t *list, list_node_t *remove_node) {
    // �����ͷ��ͷ��ǰ��
    if (remove_node == list->first) {
        list->first = remove_node->next;
    }

    // �����β����β������
    if (remove_node == list->last) {
        list->last = remove_node->pre;
    }

    // �����ǰ�������ǰ�ĺ��
    if (remove_node->pre) {
        remove_node->pre->next = remove_node->next;
    }

    // ����к����������ǰ��
    if (remove_node->next) {
        remove_node->next->pre = remove_node->pre;
    }

    // ���nodeָ��
    remove_node->pre = remove_node->next = (list_node_t*)0;
    --list->count;
    return remove_node;
}
