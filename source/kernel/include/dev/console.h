/**
 * �ն���ʾ����
 *
 * ���ߣ�����ͭ
 * ��ϵ����: 527676163@qq.com
 * 
 * ֻ֧��VGAģʽ
 */
#ifndef CONSOLE_H
#define CONSOLE_H

#include "comm/types.h"
#include "dev/tty.h"
#include "ipc/mutex.h"


#define CONSOLE_VIDEO_BASE			0xb8000		// ����̨�Դ���ʼ��ַ,��32KB
#define CONSOLE_DISP_ADDR           0xb8000
#define CONSOLE_DISP_END			(0xb8000 + 32*1024)	// �Դ�Ľ�����ַ
#define CONSOLE_ROW_MAX				25			// ����
#define CONSOLE_COL_MAX				80			// �������

#define ASCII_ESC                   0x1b        // ESC ascii��            

#define	ESC_PARAM_MAX				10			// ���֧�ֵ�ESC [ ��������

// ������ɫ
typedef enum _cclor_t {
    COLOR_Black			= 0,
    COLOR_Blue			= 1,
    COLOR_Green			= 2,
    COLOR_Cyan			= 3,
    COLOR_Red			= 4,
    COLOR_Magenta		= 5,
    COLOR_Brown			= 6,
    COLOR_Gray			= 7,
    COLOR_Dark_Gray 	= 8,
    COLOR_Light_Blue	= 9,
    COLOR_Light_Green	= 10,
    COLOR_Light_Cyan	= 11,
    COLOR_Light_Red		= 12,
    COLOR_Light_Magenta	= 13,
    COLOR_Yellow		= 14,
    COLOR_White			= 15
}cclor_t;

/**
 * @brief ��ʾ�ַ�
 */
typedef union {
	struct {
		char c;						// ��ʾ���ַ�
		char foreground : 4;		// ǰ��ɫ
		char background : 3;		// ����ɫ
	};

	uint16_t v;
}disp_char_t;

/**
 * �ն���ʾ����
 */
typedef struct _console_t {
	disp_char_t * disp_base;	// ��ʾ����ַ

    enum {
        CONSOLE_WRITE_NORMAL,			// ��ͨģʽ
        CONSOLE_WRITE_ESC,				// ESCת������
        CONSOLE_WRITE_SQUARE,           // ESC [����״̬
    }write_state;

    int cursor_row, cursor_col;		// ��ǰ�༭���к���
    int display_rows, display_cols;	// ��ʾ���������������
    int old_cursor_col, old_cursor_row;	// ����Ĺ��λ��
    cclor_t foreground, background;	// ǰ��ɫ

    int esc_param[ESC_PARAM_MAX];	// ESC [ ;;��������
    int curr_param_index;

    mutex_t mutex;                  // д������
}console_t;

int console_init (int idx);
int console_write (tty_t * tty);
void console_close (int dev);
void console_select(int idx);
void console_set_cursor(int idx, int visiable);
#endif /* SRC_UI_TTY_WIDGET_H_ */
