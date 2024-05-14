/**
 * �򵥵�����
 *
 * ���ߣ�����ͭ
 * ��ϵ����: 527676163@qq.com
 */
#ifndef LIST_H
#define LIST_H


// ��֪�ṹ���е�ĳ���ֶε�ָ�룬�����ڽṹ���ָ��
// ���磺
// struct aa{
//  .....
//  int node;
//  .....
// };
// struct aa a;
// 1.���������ڽṹ�е�ƫ��:����һ��ָ��0��ָ�룬��(struct aa *)&0->node�����ü�Ϊnode�ֶ��������ṹ���ƫ��
#define offset_in_parent(parent_type, node_name)    \
    ((uint32_t)&(((parent_type*)0)->node_name))

// 2.��node���ڵĽṹ����ַ��node�ĵ�ַ - node��ƫ��
// ����֪a->node�ĵ�ַ����a�ĵ�ַ
#define offset_to_parent(node, parent_type, node_name)   \
    ((uint32_t)node - offset_in_parent(parent_type, node_name))

// 3. ����ת��: (struct aa *)addr
// list_node_parent(node_addr, struct aa, node_name)
#define list_node_parent(node, parent_type, node_name)   \
        ((parent_type *)(node ? offset_to_parent((node), parent_type, node_name) : 0))

/**
 * ����������
 */
typedef struct _list_node_t {
    struct _list_node_t* pre;           // �����ǰһ���
    struct _list_node_t* next;         // ��̽��
}list_node_t;

/**
 * ͷ���ĳ�ʼ��
 * @param node ����ʼ���Ľ��
 */
static inline void list_node_init(list_node_t *node) {
    node->pre = node->next = (list_node_t *)0;
}

/**
 * ��ȡ����ǰһ���
 * @param node ��ѯ�Ľ��
 * @return ��̽��
 */
static inline list_node_t * list_node_pre(list_node_t *node) {
    return node->pre;
}

/**
 * ��ȡ����ǰһ���
 * @param node ��ѯ�Ľ��
 * @return ��̽��
 */
static inline list_node_t * list_node_next(list_node_t *node) {
    return node->next;
}

/**
 * ��ͷ����β���ĵ�����
 * ÿ�����ֻ��Ҫһ��ָ�룬���ڼ����ڴ�ʹ����
 */
typedef struct _list_t {
    list_node_t * first;            // ͷ���
    list_node_t * last;             // β���
    int count;                        // �������
}list_t;

void list_init(list_t *list);

/**
 * �ж������Ƿ�Ϊ��
 * @param list �жϵ�����
 * @return 1 - �գ�0 - �ǿ�
 */
static inline int list_is_empty(list_t *list) {
    return list->count == 0;
}

/**
 * ��ȡ����Ľ������
 * @param list ��ѯ������
 * @return ���������
 */
static inline int list_count(list_t *list) {
    return list->count;
}

/**
 * ��ȡָ������ĵ�һ������
 * @param list ��ѯ������
 * @return ��һ������
 */
static inline list_node_t* list_first(list_t *list) {
    return list->first;
}

/**
 * ��ȡָ�����ӵ����һ������
 * @param list ��ѯ������
 * @return ���һ������
 */
static inline list_node_t* list_last(list_t *list) {
    return list->last;
}

void list_insert_first(list_t *list, list_node_t *node);
void list_insert_last(list_t *list, list_node_t *node);
list_node_t* list_remove_first(list_t *list);
list_node_t* list_remove(list_t *list, list_node_t *node);

#endif /* LIST_H */
