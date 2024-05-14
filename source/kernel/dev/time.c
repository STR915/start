#include "dev/time.h"
#include "cpu/irq.h"
#include "comm/cpu_instr.h"
#include "os_cfg.h"
#include "core/task.h"

static uint32_t sys_tick;						// ϵͳ�������tick����

/**
 * ��ʱ���жϴ�����
 */
void do_handler_timer (exception_frame_t *frame) {
    sys_tick++;

    // �ȷ�EOI�������Ƿ������
    // ����󽫴��������л���ȥ֮�󣬳����������л��������ܼ�����Ӧ
    pic_send_eoi(IRQ0_TIMER);

    task_time_tick();
}

/**
 * ��ʼ��Ӳ����ʱ��
 */
static void init_pit (void) {
    uint32_t reload_count = PIT_OSC_FREQ * OS_TICK_MS/1000;

    outb(PIT_COMMAND_MODE_PORT, PIT_CHANNLE0 | PIT_LOAD_LOHI | PIT_MODE3);
    outb(PIT_CHANNEL0_DATA_PORT, reload_count & 0xFF);   // ���ص�8λ
    outb(PIT_CHANNEL0_DATA_PORT, (reload_count >> 8) & 0xFF); // �ټ��ظ�8λ

    irq_install(IRQ0_TIMER, (irq_handler_t)exception_handler_timer);
    irq_enable(IRQ0_TIMER);
}

/**
 * ��ʱ����ʼ��
 */
void time_init (void) {
    sys_tick = 0;

    init_pit();
}


