/**
 * Ӳ����ʱ������
 *
 * ����ʱ�䣺2022��6��5��
 * ���ߣ�����ͭ
 * ��ϵ����: 527676163@qq.com
 */
#ifndef TIMER_H
#define TIMER_H

#include "comm/types.h"

#define PIT_OSC_FREQ                1193182				// ��ʱ��ʱ��

// ��ʱ���ļĴ����͸���λ����
#define PIT_CHANNEL0_DATA_PORT       0x40
#define PIT_COMMAND_MODE_PORT        0x43

#define PIT_CHANNLE0                (0 << 6)
#define PIT_LOAD_LOHI               (3 << 4)
#define PIT_MODE3                   (3 << 1)

void time_init (void);
void exception_handler_timer (void);

#endif //OS_TIMER_H