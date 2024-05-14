/**
 * �ڴ����
 *
 * ���ߣ�����ͭ
 * ��ϵ����: 527676163@qq.com
 */
#include "tools/klib.h"
#include "tools/log.h"
#include "core/memory.h"
#include "tools/klib.h"
#include "cpu/mmu.h"
#include "dev/console.h"

static addr_alloc_t paddr_alloc;        // ������ַ����ṹ
static pde_t kernel_page_dir[PDE_CNT] __attribute__((aligned(MEM_PAGE_SIZE))); // �ں�ҳĿ¼��

/**
 * @brief ��ȡ��ǰҳ����ַ
 */
static pde_t * current_page_dir (void) {
    return (pde_t *)task_current()->tss.cr3;
}

/**
 * @brief ��ʼ����ַ����ṹ
 * ���²����start��size��ҳ�߽磬���ϲ�����߼��
 */
static void addr_alloc_init (addr_alloc_t * alloc, uint8_t * bits,
                    uint32_t start, uint32_t size, uint32_t page_size) {
    mutex_init(&alloc->mutex);
    alloc->start = start;
    alloc->size = size;
    alloc->page_size = page_size;
    bitmap_init(&alloc->bitmap, bits, alloc->size / page_size, 0);
}

/**
 * @brief �����ҳ�ڴ�
 */
static uint32_t addr_alloc_page (addr_alloc_t * alloc, int page_count) {
    uint32_t addr = 0;

    mutex_lock(&alloc->mutex);

    int page_index = bitmap_alloc_nbits(&alloc->bitmap, 0, page_count);
    if (page_index >= 0) {
        addr = alloc->start + page_index * alloc->page_size;
    }

    mutex_unlock(&alloc->mutex);
    return addr;
}

/**
 * @brief �ͷŶ�ҳ�ڴ�
 */
static void addr_free_page (addr_alloc_t * alloc, uint32_t addr, int page_count) {
    mutex_lock(&alloc->mutex);

    uint32_t pg_idx = (addr - alloc->start) / alloc->page_size;
    bitmap_set_bit(&alloc->bitmap, pg_idx, page_count, 0);

    mutex_unlock(&alloc->mutex);
}

static void show_mem_info (boot_info_t * boot_info) {
    log_printf("mem region:");
    for (int i = 0; i < boot_info->ram_region_count; i++) {
        log_printf("[%d]: 0x%x - 0x%x", i,
                    boot_info->ram_region_cfg[i].start,
                    boot_info->ram_region_cfg[i].size);
    }
    log_printf("\n");
}

/**
 * @brief ��ȡ���õ������ڴ��С
 */
static uint32_t total_mem_size(boot_info_t * boot_info) {
    int mem_size = 0;

    // ��������ݲ������м��пն������
    for (int i = 0; i < boot_info->ram_region_count; i++) {
        mem_size += boot_info->ram_region_cfg[i].size;
    }
    return mem_size;
}

pte_t * find_pte (pde_t * page_dir, uint32_t vaddr, int alloc) {
    pte_t * page_table;

    pde_t *pde = page_dir + pde_index(vaddr);
    if (pde->present) {
        page_table = (pte_t *)pde_paddr(pde);
    } else {
        // ��������ڣ����Ƿ���һ��
        if (alloc == 0) {
            return (pte_t *)0;
        }

        // ����һ������ҳ��
        uint32_t pg_paddr = addr_alloc_page(&paddr_alloc, 1);
        if (pg_paddr == 0) {
            return (pte_t *)0;
        }

        // ����Ϊ�û��ɶ�д������pte������������
        pde->v = pg_paddr | PTE_P | PTE_W | PDE_U;

        // Ϊ����ҳ���������ַ��ӳ�䣬��������Ϳ��Լ���������ַ��
        //kernel_pg_last[pde_index(vaddr)].v = pg_paddr | PTE_P | PTE_W;

        // ���ҳ������ֹ�����쳣
        // ���������ַ��������ַһһӳ�䣬����ֱ��д��
        page_table = (pte_t *)(pg_paddr);
        kernel_memset(page_table, 0, MEM_PAGE_SIZE);
    }
  
    return page_table + pte_index(vaddr);
}

/**
 * @brief ��ָ���ĵ�ַ�ռ����һҳ��ӳ��
 */
int memory_create_map (pde_t * page_dir, uint32_t vaddr, uint32_t paddr, int count, uint32_t perm) {
    for (int i = 0; i < count; i++) {
        // log_printf("create map: v-0x%x p-0x%x, perm: 0x%x", vaddr, paddr, perm);

        pte_t * pte = find_pte(page_dir, vaddr, 1);
        if (pte == (pte_t *)0) {
            // log_printf("create pte failed. pte == 0");
            return -1;
        }

        // ����ӳ���ʱ������pteӦ���ǲ����ڵġ�
        // ������ڣ�˵������������
        // log_printf("\tpte addr: 0x%x", (uint32_t)pte);

        ASSERT(pte->present == 0);

        pte->v = paddr | perm | PTE_P;

        vaddr += MEM_PAGE_SIZE;
        paddr += MEM_PAGE_SIZE;
    }

    return 0;
}

/**
 * @brief �����ڴ�ӳ����������ں�ҳ��
 */
void create_kernel_table (void) {
    extern uint8_t s_text[], e_text[], s_data[], e_data[];
    extern uint8_t kernel_base[];

    // ��ַӳ���, ���ڽ����ں˼��ĵ�ַӳ��
    // ��ַ���䣬��������������
    static memory_map_t kernel_map[] = {
        {kernel_base,   s_text,         0,              PTE_W},         // �ں�ջ��
        {s_text,        e_text,         s_text,         0},         // �ں˴�����
        {s_data,        (void *)(MEM_EBDA_START - 1),   s_data,        PTE_W},      // �ں�������
        {(void *)CONSOLE_DISP_ADDR, (void *)(CONSOLE_DISP_END - 1), (void *)CONSOLE_VIDEO_BASE, PTE_W},

        // ��չ�洢�ռ�һһӳ�䣬����ֱ�Ӳ���
        {(void *)MEM_EXT_START, (void *)MEM_EXT_END,     (void *)MEM_EXT_START, PTE_W},
    };

    // ���ҳĿ¼��
    kernel_memset(kernel_page_dir, 0, sizeof(kernel_page_dir));

    // ��պ�Ȼ�����θ���ӳ���ϵ����ӳ���
    for (int i = 0; i < sizeof(kernel_map) / sizeof(memory_map_t); i++) {
        memory_map_t * map = kernel_map + i;

        // �����ж��ҳ���������ҳ������
        // �������������4M�����
        uint32_t vstart = down2((uint32_t)map->vstart, MEM_PAGE_SIZE);
        uint32_t vend = up2((uint32_t)map->vend, MEM_PAGE_SIZE);
        uint32_t paddr = down2((uint32_t)map->pstart, MEM_PAGE_SIZE);
        int page_count = (vend - vstart) / MEM_PAGE_SIZE;

        memory_create_map(kernel_page_dir, vstart, (uint32_t)paddr, page_count, map->perm);
    }
}

/**
 * @brief �������̵ĳ�ʼҳ��
 * ��Ҫ�Ĺ�������ҳĿ¼����Ȼ����ں�ҳ���и���һ����
 */
uint32_t memory_create_uvm (void) {
    pde_t * page_dir = (pde_t *)addr_alloc_page(&paddr_alloc, 1);
    if (page_dir == 0) {
        return 0;
    }
    kernel_memset((void *)page_dir, 0, MEM_PAGE_SIZE);

    // ���������ں˿ռ��ҳĿ¼��Ա����������̹����ں˿ռ�
    // �û��ռ���ڴ�ӳ���ݲ��������ȼ��س���ʱ����
    uint32_t user_pde_start = pde_index(MEMORY_TASK_BASE);
    for (int i = 0; i < user_pde_start; i++) {
        page_dir[i].v = kernel_page_dir[i].v;
    }

    return (uint32_t)page_dir;
}

/**
 * @brief �����û��ռ��ڴ�
 */
void memory_destroy_uvm (uint32_t page_dir) {
    uint32_t user_pde_start = pde_index(MEMORY_TASK_BASE);
    pde_t * pde = (pde_t *)page_dir + user_pde_start;

    ASSERT(page_dir != 0);

    // �ͷ�ҳ���ж�Ӧ�ĸ��������ӳ����ں�ҳ��
    for (int i = user_pde_start; i < PDE_CNT; i++, pde++) {
        if (!pde->present) {
            continue;
        }

        // �ͷ�ҳ����Ӧ������ҳ + ҳ��
        pte_t * pte = (pte_t *)pde_paddr(pde);
        for (int j = 0; j < PTE_CNT; j++, pte++) {
            if (!pte->present) {
                continue;
            }

            addr_free_page(&paddr_alloc, pte_paddr(pte), 1);
        }

        addr_free_page(&paddr_alloc, (uint32_t)pde_paddr(pde), 1);
    }

    // ҳĿ¼��
    addr_free_page(&paddr_alloc, page_dir, 1);
}

/**
 * @brief ����ҳ���������е��ڴ�ռ�
 */
uint32_t memory_copy_uvm (uint32_t page_dir) {
    // ���ƻ���ҳ��
    uint32_t to_page_dir = memory_create_uvm();
    if (to_page_dir == 0) {
        goto copy_uvm_failed;
    }

    // �ٸ����û��ռ�ĸ���
    uint32_t user_pde_start = pde_index(MEMORY_TASK_BASE);
    pde_t * pde = (pde_t *)page_dir + user_pde_start;

    // �����û��ռ�ҳĿ¼��
    for (int i = user_pde_start; i < PDE_CNT; i++, pde++) {
        if (!pde->present) {
            continue;
        }

        // ����ҳ��
        pte_t * pte = (pte_t *)pde_paddr(pde);
        for (int j = 0; j < PTE_CNT; j++, pte++) {
            if (!pte->present) {
                continue;
            }

            // ���������ڴ�
            uint32_t page = addr_alloc_page(&paddr_alloc, 1);
            if (page == 0) {
                goto copy_uvm_failed;
            }

            // ����ӳ���ϵ
            uint32_t vaddr = (i << 22) | (j << 12);
            int err = memory_create_map((pde_t *)to_page_dir, vaddr, page, 1, get_pte_perm(pte));
            if (err < 0) {
                goto copy_uvm_failed;
            }

            // �������ݡ�
            kernel_memcpy((void *)page, (void *)vaddr, MEM_PAGE_SIZE);
        }
    }
    return to_page_dir;

copy_uvm_failed:
    if (to_page_dir) {
        memory_destroy_uvm(to_page_dir);
    }
    return -1;
}

/**
 * @brief ��ȡָ�������ַ��������ַ
 * ���ת��ʧ�ܣ�����0��
 */
uint32_t memory_get_paddr (uint32_t page_dir, uint32_t vaddr) {
    pte_t * pte = find_pte((pde_t *)page_dir, vaddr, 0);
    if (pte == (pte_t *)0) {
        return 0;
    }

    return pte_paddr(pte) + (vaddr & (MEM_PAGE_SIZE - 1));
}

/**
 * @brief �ڲ�ͬ�Ľ��̿ռ��п����ַ���
 * page_dirΪĿ��ҳ������ǰ��Ϊ��ҳ��
 */
int memory_copy_uvm_data(uint32_t to, uint32_t page_dir, uint32_t from, uint32_t size) {
    char *buf, *pa0;

    while(size > 0){
        // ��ȡĿ���������ַ, Ҳ������һ�������ַ
        uint32_t to_paddr = memory_get_paddr(page_dir, to);
        if (to_paddr == 0) {
            return -1;
        }

        // ���㵱ǰ�ɿ����Ĵ�С
        uint32_t offset_in_page = to_paddr & (MEM_PAGE_SIZE - 1);
        uint32_t curr_size = MEM_PAGE_SIZE - offset_in_page;
        if (curr_size > size) {
            curr_size = size;       // ����Ƚϴ󣬳���ҳ�߽磬��ֻ������ҳ�ڵ�
        }

        kernel_memcpy((void *)to_paddr, (void *)from, curr_size);

        size -= curr_size;
        to += curr_size;
        from += curr_size;
  }

  return 0;
}

uint32_t memory_alloc_for_page_dir (uint32_t page_dir, uint32_t vaddr, uint32_t size, int perm) {
    uint32_t curr_vaddr = vaddr;
    int page_count = up2(size, MEM_PAGE_SIZE) / MEM_PAGE_SIZE;
    vaddr = down2(vaddr, MEM_PAGE_SIZE);

    // ��ҳ�����ڴ棬Ȼ����ӳ���ϵ
    for (int i = 0; i < page_count; i++) {
        // ������Ҫ���ڴ�
        uint32_t paddr = addr_alloc_page(&paddr_alloc, 1);
        if (paddr == 0) {
            log_printf("mem alloc failed. no memory");
            return 0;
        }

        // ����������ڴ���ָ����ַ�Ĺ���
        int err = memory_create_map((pde_t *)page_dir, curr_vaddr, paddr, 1, perm);
        if (err < 0) {
            log_printf("create memory map failed. err = %d", err);
            addr_free_page(&paddr_alloc, vaddr, i + 1);
            return -1;
        }

        curr_vaddr += MEM_PAGE_SIZE;
    }

    return 0;
}

/**
 * @brief Ϊָ���������ַ�ռ�����ҳ�ڴ�
 */
int memory_alloc_page_for (uint32_t addr, uint32_t size, int perm) {
    return memory_alloc_for_page_dir(task_current()->tss.cr3, addr, size, perm);
}


/**
 * @brief ����һҳ�ڴ�
 * ��Ҫ�����ں˿ռ��ڴ�ķ��䣬�����ڽ����ڴ�ռ�
 */
uint32_t memory_alloc_page (void) {
    // �ں˿ռ������ַ��������ַ��ͬ
    return addr_alloc_page(&paddr_alloc, 1);
}

/**
 * @brief �ͷ�һҳ�ڴ�
 */
void memory_free_page (uint32_t addr) {
    if (addr < MEMORY_TASK_BASE) {
        // �ں˿ռ䣬ֱ���ͷ�
        addr_free_page(&paddr_alloc, addr, 1);
    } else {
        // ���̿ռ䣬��Ҫ�ͷ�ҳ��
        pte_t * pte = find_pte(current_page_dir(), addr, 0);
        ASSERT((pte == (pte_t *)0) && pte->present);

        // �ͷ��ڴ�ҳ
        addr_free_page(&paddr_alloc, pte_paddr(pte), 1);

        // �ͷ�ҳ��
        pte->v = 0;
    }
}

/**
 * @brief ��ʼ���ڴ����ϵͳ
 * �ú�������Ҫ����
 * 1����ʼ�������ڴ�������������������ڴ��������. ��1MB�ڴ��з�������λͼ
 * 2�����´����ں�ҳ����ԭloader�д�����ҳ���Ѿ����ٺ���
 */
void memory_init (boot_info_t * boot_info) {
    // 1MB�ڴ�ռ���ʼ�������ӽű��ж���
    extern uint8_t * mem_free_start;

    log_printf("mem init.");
    show_mem_info(boot_info);

    // ���ں����ݺ��������ҳλͼ
    uint8_t * mem_free = (uint8_t *)&mem_free_start;   // 2022��-10-1 ��ͬѧ���������������е�bug��������

    // ����1MB���Ͽռ�Ŀ����ڴ��������������ҳ�߽�
    uint32_t mem_up1MB_free = total_mem_size(boot_info) - MEM_EXT_START;
    mem_up1MB_free = down2(mem_up1MB_free, MEM_PAGE_SIZE);   // ���뵽4KBҳ
    log_printf("Free memory: 0x%x, size: 0x%x", MEM_EXT_START, mem_up1MB_free);

    // 4GB��С��Ҫ�ܹ�4*1024*1024*1024/4096/8=128KB��λͼ, ʹ�õ�1MB��RAM�ռ����㹻
    // �ò��ֵ��ڴ������mem_free_start��ʼ����
    addr_alloc_init(&paddr_alloc, mem_free, MEM_EXT_START, mem_up1MB_free, MEM_PAGE_SIZE);
    mem_free += bitmap_byte_count(paddr_alloc.size / MEM_PAGE_SIZE);

    // �����mem_freeӦ�ñ�EBDA��ַҪС
    ASSERT(mem_free < (uint8_t *)MEM_EBDA_START);

    // �����ں�ҳ�����л���ȥ
    create_kernel_table();

    // ���л�����ǰҳ��
    mmu_set_page_dir((uint32_t)kernel_page_dir);
}

/**
 * @brief �����ѵ��ڴ���䣬���ض�֮ǰ��ָ��
 */
char * sys_sbrk(int incr) {
    task_t * task = task_current();
    char * pre_heap_end = (char * )task->heap_end;
    int pre_incr = incr;

    ASSERT(incr >= 0);

    // �����ַΪ0���򷵻���Ч��heap����Ķ���
    if (incr == 0) {
        log_printf("sbrk(0): end = 0x%x", pre_heap_end);
        return pre_heap_end;
    } 
    
    uint32_t start = task->heap_end;
    uint32_t end = start + incr;

    // ��ʼƫ�Ʒ�0
    int start_offset = start % MEM_PAGE_SIZE;
    if (start_offset) {
        // ������1ҳ��ֻ����
        if (start_offset + incr <= MEM_PAGE_SIZE) {
            task->heap_end = end;
            return pre_heap_end;
        } else {
            // ����1ҳ����ֻ����ҳ��
            uint32_t curr_size = MEM_PAGE_SIZE - start_offset;
            start += curr_size;
            incr -= curr_size;
        }
    }

    // ��������ģ���ʼ�����ҳ�߽��
    if (incr) {
        uint32_t curr_size = end - start;
        int err = memory_alloc_page_for(start, curr_size, PTE_P | PTE_U | PTE_W);
        if (err < 0) {
            log_printf("sbrk: alloc mem failed.");
            return (char *)-1;
        }
    }

    //log_printf("sbrk(%d): end = 0x%x", pre_incr, end);
    task->heap_end = end;
    return (char * )pre_heap_end;        
}