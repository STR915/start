#include "init.h"
#include "comm/boot_info.h"
#include "comm/cpu_instr.h"
#include "cpu/cpu.h"
#include "cpu/irq.h"
#include "dev/time.h"
#include "tools/log.h"
#include "tools/klib.h"
#include "os_cfg.h"
#include "core/task.h"
#include "core/memory.h"
void kernel_init(boot_info_t* boot_info){
    
    cpu_init();
    log_init();
    memory_init(boot_info);
    irq_init();
    time_init();
    task_manager_init();
}


void move_to_first_task (void){
    // ����ֱ����Jmp far���룬��Ϊ��ǰ��Ȩ��0��������������Ȩ���Ĵ���
    // �����iret�󣬻���Ҫ�ֶ�����ds, fs, es�ȼĴ���ֵ��iret�����Զ�����
    // ע�⣬��������Ĵ�����ܻ�����쳣���α����쳣��ҳ�����쳣��
    // �ɸ��ݲ������쳣���ͺʹ����룬������ֲ����ҵ���������
    task_t * curr = task_current();
    ASSERT(curr != 0);

    tss_t * tss = &(curr->tss);

    // Ҳ����ʹ������boot��loader�еĺ���ָ����ת
    // ������jmp����Ϊ������Ҫʹ��������������������
    __asm__ __volatile__(
        // ģ���жϷ��أ��л����1��������Ӧ�ý���
        // �������ﲢ��ֱ�ӽ��뵽���̵���ڣ����������úöμĴ�����������ȥ
        "push %[ss]\n\t"			// SS
        "push %[esp]\n\t"			// ESP
        "push %[eflags]\n\t"           // EFLAGS
        "push %[cs]\n\t"			// CS
        "push %[eip]\n\t"		    // ip
        "iret\n\t"::[ss]"r"(tss->ss),  [esp]"r"(tss->esp), [eflags]"r"(tss->eflags),
        [cs]"r"(tss->cs), [eip]"r"(tss->eip));
};

void init_main(void){
    log_printf("Kernel is running....");
    log_printf("Version: %s",OS_VERSION);

    task_first_init();
    move_to_first_task();
}