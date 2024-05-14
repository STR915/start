/**
 * �ڴ����
 *
 * ���ߣ�����ͭ
 * ��ϵ����: 527676163@qq.com
 */
#ifndef MEMORY_H
#define MEMORY_H

#include "tools/bitmap.h"
#include "comm/boot_info.h"
#include "ipc/mutex.h"

#define MEM_EBDA_START              0x00080000
#define MEM_EXT_START               (1024*1024)
#define MEM_EXT_END                 (128*1024*1024 - 1)
#define MEM_PAGE_SIZE               4096        // ��ҳ���Сһ��

#define MEMORY_TASK_BASE            (0x80000000)        // ������ʼ��ַ�ռ�
#define MEM_TASK_STACK_TOP          (0xE0000000)        // ��ʼջ��λ��  
#define MEM_TASK_STACK_SIZE         (MEM_PAGE_SIZE * 500)   // ��ʼ500KBջ
#define MEM_TASK_ARG_SIZE           (MEM_PAGE_SIZE * 4)     // �����ͻ�������ռ�õĴ�С

/**
 * @brief ��ַ����ṹ
 */
typedef struct _addr_alloc_t {
    mutex_t mutex;              // ��ַ���以���ź���
    bitmap_t bitmap;            // ���������õ�λͼ

    uint32_t page_size;         // ҳ��С
    uint32_t start;             // ��ʼ��ַ
    uint32_t size;              // ��ַ��С
}addr_alloc_t;

/**
 * @brief �����ַ�������ַ֮���ӳ���ϵ��
 */
typedef struct _memory_map_t {
    void * vstart;     // �����ַ
    void * vend;
    void * pstart;       // �����ַ
    uint32_t perm;      // ����Ȩ��
}memory_map_t;

void memory_init (boot_info_t * boot_info);
uint32_t memory_create_uvm (void);
uint32_t memory_alloc_for_page_dir (uint32_t page_dir, uint32_t vaddr, uint32_t size, int perm);
int memory_alloc_page_for (uint32_t addr, uint32_t size, int perm);
uint32_t memory_alloc_page (void);
void memory_free_page (uint32_t addr);
void memory_destroy_uvm (uint32_t page_dir);
uint32_t memory_copy_uvm (uint32_t page_dir);
uint32_t memory_get_paddr (uint32_t page_dir, uint32_t vaddr);
int memory_copy_uvm_data(uint32_t to, uint32_t page_dir, uint32_t from, uint32_t size);
char * sys_sbrk(int incr);

#endif // MEMORY_H