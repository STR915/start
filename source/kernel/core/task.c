#include "comm/cpu_instr.h"
#include "core/task.h"
#include "tools/klib.h"
#include "tools/log.h"
#include "os_cfg.h"
#include "cpu/irq.h"
#include "cpu/cpu.h"
#include "cpu/irq.h"
#include "core/memory.h"
#include "cpu/mmu.h"
#include "comm/elf.h"
static task_manager_t task_manager;     // ���������
static uint32_t idle_task_stack[IDLE_STACK_SIZE];	// ���������ջ
static task_t task_table[TASK_NR];      // �û����̱�

static int tss_init (task_t * task,int flag, uint32_t entry, uint32_t esp){
    int tss_sel = gdt_alloc_desc();
    if (tss_sel < 0) {
        log_printf("alloc tss failed.\n");
        return -1;
    }

    segment_desc_set(tss_sel, (uint32_t)&task->tss, sizeof(tss_t),
            SEG_P_PRESENT | SEG_DPL0 | SEG_TYPE_TSS);

    // tss�γ�ʼ��
    kernel_memset(&task->tss, 0, sizeof(tss_t));

    // �����ں�ջ���õ����������ַ
    uint32_t kernel_stack = memory_alloc_page();
    if (kernel_stack == 0) {
        goto tss_init_failed;
    }

    // ���ݲ�ͬ��Ȩ��ѡ��ͬ�ķ���ѡ����
    int code_sel, data_sel;
    if (flag & TASK_FLAG_SYSTEM) {
        code_sel = KERNEL_SELECTOR_CS;
        data_sel = KERNEL_SELECTOR_DS;
    } else {
        // ע�����RP3,��Ȼ�������α�������
        code_sel = task_manager.app_code_sel | SEG_CPL3;
        data_sel = task_manager.app_data_sel | SEG_CPL3;
    }

    task->tss.eip = entry;
    task->tss.esp = esp;  // δָ��ջ�����ں�ջ������������Ȩ��0�Ľ���
    task->tss.esp0 = kernel_stack + MEM_PAGE_SIZE;
    task->tss.ss0 = KERNEL_SELECTOR_DS;
    task->tss.eip = entry;
    task->tss.eflags = EFLAGS_DEFAULT| EFLAGS_IF;
    task->tss.es = task->tss.ss = task->tss.ds = task->tss.fs 
            = task->tss.gs = data_sel;   // ȫ������ͬһ���ݶ�
    task->tss.cs = code_sel; 
    task->tss.iomap = 0;

    // ҳ���ʼ��
    uint32_t page_dir = memory_create_uvm();
    if (page_dir == 0) {
        goto tss_init_failed;
    }
    task->tss.cr3 = page_dir;

    task->tss_sel = tss_sel;
    return 0;
tss_init_failed:
    gdt_free_sel(tss_sel);

    if (kernel_stack) {
        memory_free_page(kernel_stack);
    }
    return -1;
}
/**
 * @brief ��ȡ��һ��Ҫ���е�����
 */
static task_t * task_next_run (void) {
    // ���û�����������п�������
    if (list_count(&task_manager.ready_list) == 0) {
        return &task_manager.idle_task;
    }
    
    // ��ͨ����
    list_node_t * task_node = list_first(&task_manager.ready_list);
    return list_node_parent(task_node, task_t, run_node);
}

int task_init (task_t *task, const char* name,int flag, uint32_t entry, uint32_t esp){
    ASSERT(task != (task_t *)0);

    tss_init(task,flag, entry, esp);

    kernel_strncpy(task->name, name, TASK_NAME_SIZE);
    task->state = TASK_CREATED;
    task->sleep_ticks = 0;
    task->time_ticks = TASK_TIME_SLICE_DEFAULT;

    task->slice_ticks = task->time_ticks;

    list_node_init(&task->all_node);
    list_node_init(&task->run_node);
    list_node_init(&task->wait_node);
    irq_state_t state = irq_enter_protection();
    task_set_ready(task);
    list_insert_last(&task_manager.task_list, &task->all_node);
    irq_leave_protection(state);
    return 0;
}

void simple_switch (uint32_t **from, uint32_t *to);

/**
 * @brief �л���ָ������
 */
void task_switch_from_to (task_t * from, task_t * to) {
    switch_to_tss(to->tss_sel);
    //simple_switch(&from->stack, to->stack);
}

void task_first_init (void){
    void first_task_entry (void);

    // ���»�õ���bin�ļ����ڴ��е������ַ
    extern uint8_t s_first_task[], e_first_task[];

    // ����Ŀռ��ʵ�ʴ洢�Ŀռ�Ҫ��һЩ����������ڷ���ջ
    uint32_t copy_size = (uint32_t)(e_first_task - s_first_task);
    uint32_t alloc_size = 10 * MEM_PAGE_SIZE;
    ASSERT(copy_size < alloc_size);

    uint32_t first_start = (uint32_t)first_task_entry;

    // ��һ�����������СһЩ���ú�ջ����1��ҳ����
    // �����Ͳ�Ҫ�������ǻ�Ҫ��ջ����ռ������
    task_init(&task_manager.first_task, "first task", 0, first_start, first_start + alloc_size);
    task_manager.first_task.heap_start = (uint32_t)e_first_task;  // ���ﲻ��
    task_manager.first_task.heap_end = task_manager.first_task.heap_start;
    task_manager.curr_task = &task_manager.first_task;

    // ����ҳ���ַΪ�Լ���
    mmu_set_page_dir(task_manager.first_task.tss.cr3);

    // ����һҳ�ڴ湩������ʹ�ã�Ȼ�󽫴��븴�ƹ�ȥ
    memory_alloc_page_for(first_start,  alloc_size, PTE_P | PTE_W | PTE_U);
    kernel_memcpy((void *)first_start, (void *)&s_first_task, copy_size);

    // ��������
    task_start(&task_manager.first_task);

    // дTR�Ĵ�����ָʾ��ǰ���еĵ�һ������
    write_tr(task_manager.first_task.tss_sel);
}
/**
 * @brief ��������
 */
void task_start(task_t * task) {
    irq_state_t state = irq_enter_protection();
    task_set_ready(task);
    irq_leave_protection(state);
}

task_t * task_first_task (void){
    return &task_manager.first_task;
}

static void idle_task_entry (void){
    for(;;){
        hlt();
    }
}

void task_manager_init (void){
    kernel_memset(task_table, 0, sizeof(task_table));
    //mutex_init(&task_table_mutex);

    //���ݶκʹ���Σ�ʹ��DPL3������Ӧ�ù���ͬһ��
    //Ϊ���Է��㣬��ʱʹ��DPL0
    int sel = gdt_alloc_desc();
    segment_desc_set(sel, 0x00000000, 0xFFFFFFFF,
                     SEG_P_PRESENT | SEG_DPL3 | SEG_S_NORMAL |
                     SEG_TYPE_DATA | SEG_TYPE_RW | SEG_D);
    task_manager.app_data_sel = sel;

    sel = gdt_alloc_desc();
    segment_desc_set(sel, 0x00000000, 0xFFFFFFFF,
                     SEG_P_PRESENT | SEG_DPL3 | SEG_S_NORMAL |
                     SEG_TYPE_CODE | SEG_TYPE_RW | SEG_D);
    task_manager.app_code_sel = sel;

    // �����г�ʼ��
    list_init(&task_manager.ready_list);
    list_init(&task_manager.task_list);
    list_init(&task_manager.sleep_list);

    // ���������ʼ��
    task_init(&task_manager.idle_task,"idle_task",TASK_FLAG_SYSTEM,(uint32_t)idle_task_entry,(uint32_t)(idle_task_stack+IDLE_STACK_SIZE));
    task_manager.curr_task = (task_t *)0;
    task_start(&task_manager.idle_task);
}

/**
 * @brief ����������������
 */
void task_set_ready(task_t *task) {
    if (task != &task_manager.idle_task) {
        list_insert_last(&task_manager.ready_list, &task->run_node);
        task->state = TASK_READY;
    }
}

/**
 * @brief ������Ӿ��������Ƴ�
 */
void task_set_block (task_t *task) {
    if (task != &task_manager.idle_task) {
        list_remove(&task_manager.ready_list, &task->run_node);
    }
}

/**
 * @brief ��ȡ��ǰ�������е�����
 */
task_t * task_current (void) {
    return task_manager.curr_task;
}

/**
 * @brief ����һ���������
 */
void task_dispatch (void) {
    irq_state_t state = irq_enter_protection();
    task_t * to = task_next_run();

    if (to != task_manager.curr_task) {
        task_t * from = task_manager.curr_task;
        task_manager.curr_task = to;
        to->state = TASK_RUNNING;
        task_switch_from_to(from, to);
    }
    irq_leave_protection(state);
}

int sys_sched_yield (void){
    irq_state_t state = irq_enter_protection();

    if (list_count(&task_manager.ready_list) > 1) {
        task_t * curr_task = task_current();

        // ��������л������������򽫵�ǰ�������뵽����β��
        task_set_block(curr_task);
        task_set_ready(curr_task);

        // �л�����һ���������л����ǰҪ��������Ȼ������һ����
        // ����ĳЩԭ�����к�������ɾ�����ٻص������л�����������
        task_dispatch();
    }
    irq_leave_protection(state);

    return 0;
}

/**
 * @brief ʱ�䴦��
 * �ú������жϴ������е���
 */
void task_time_tick (void) {
    task_t * curr_task = task_current();

    // ʱ��Ƭ�Ĵ���
    irq_state_t state = irq_enter_protection();
    if (--curr_task->slice_ticks == 0) {
        // ʱ��Ƭ���꣬���¼���ʱ��Ƭ
        // ���ڿ������񣬴˴���δ��
        curr_task->slice_ticks = curr_task->time_ticks;

        // �������е�λ�õ�β��������ֱ�Ӳ�������
        task_set_block(curr_task);
        task_set_ready(curr_task);
        task_dispatch();
    }
    
    // ˯�ߴ���
    list_node_t * curr = list_first(&task_manager.sleep_list);
    while (curr) {
        list_node_t * next = list_node_next(curr);

        task_t * task = list_node_parent(curr, task_t, run_node);
        if (--task->sleep_ticks == 0) {
            // ��ʱʱ�䵽���˯�߶������Ƴ���������������
            task_set_wakeup(task);
            task_set_ready(task);
        }
        curr = next;
    }

    task_dispatch();
    irq_leave_protection(state);
}
/**
 * @brief ���������˯��״̬
 */
void task_set_sleep(task_t *task, uint32_t ticks) {
    if (ticks <= 0) {
        return;
    }

    task->sleep_ticks = ticks;
    task->state = TASK_SLEEP;
    list_insert_last(&task_manager.sleep_list, &task->run_node);
}

/**
 * @brief ���������ʱ�����Ƴ�
 * 
 * @param task 
 */
void task_set_wakeup (task_t *task) {
    list_remove(&task_manager.sleep_list, &task->run_node);
}

/**
 * @brief �������˯��״̬
 * 
 * @param ms 
 */
void sys_msleep (uint32_t ms) {
    // ������ʱ1��tick
    if (ms < OS_TICK_MS) {
        ms = OS_TICK_MS;
    }

    irq_state_t state = irq_enter_protection();

    // �Ӿ��������Ƴ�������˯�߶���
    task_set_block(task_manager.curr_task);
    task_set_sleep(task_manager.curr_task, (ms + (OS_TICK_MS - 1))/ OS_TICK_MS);
    
    // ����һ�ε���
    task_dispatch();

    irq_leave_protection(state);
}
