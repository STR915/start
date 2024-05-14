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
    // 不能直接用Jmp far进入，因为当前特权级0，不能跳到低特权级的代码
    // 下面的iret后，还需要手动加载ds, fs, es等寄存器值，iret不会自动加载
    // 注意，运行下面的代码可能会产生异常：段保护异常或页保护异常。
    // 可根据产生的异常类型和错误码，并结合手册来找到问题所在
    task_t * curr = task_current();
    ASSERT(curr != 0);

    tss_t * tss = &(curr->tss);

    // 也可以使用类似boot跳loader中的函数指针跳转
    // 这里用jmp是因为后续需要使用内联汇编添加其它代码
    __asm__ __volatile__(
        // 模拟中断返回，切换入第1个可运行应用进程
        // 不过这里并不直接进入到进程的入口，而是先设置好段寄存器，再跳过去
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