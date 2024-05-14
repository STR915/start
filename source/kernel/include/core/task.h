#ifndef TASK_H
#define TASK_H

#include "comm/types.h"
#include "cpu/cpu.h"
#include "tools/list.h"

#define TASK_NAME_SIZE				32			// �������ֳ���
#define TASK_TIME_SLICE_DEFAULT		10			// ʱ��Ƭ����
#define TASK_OFILE_NR				128			// ���֧�ִ򿪵��ļ�����

#define TASK_FLAG_SYSTEM       	(1 << 0)		// ϵͳ����

typedef struct _task_args_t {
	uint32_t ret_addr;		// ���ص�ַ������
	uint32_t argc;
	char **argv;
}task_args_t;

/**
 * @brief ������ƿ�ṹ
 */
typedef struct _task_t {
    enum {
		TASK_CREATED,
		TASK_RUNNING,
		TASK_SLEEP,
		TASK_READY,
		TASK_WAITING,
		TASK_ZOMBIE,
	}state;

    char name[TASK_NAME_SIZE];		// ��������

    int pid;				// ���̵�pid
    struct _task_t * parent;		// ������
	uint32_t heap_start;		// �ѵĶ����ַ
	uint32_t heap_end;			// �ѽ�����ַ
    int status;				// ����ִ�н��

    int sleep_ticks;		// ˯��ʱ��
    int time_ticks;			// ʱ��Ƭ
	int slice_ticks;		// �ݼ�ʱ��Ƭ����

    // file_t * file_table[TASK_OFILE_NR];	// �������򿪵��ļ�����
    //uint32_t* stack;
	tss_t tss;				// �����TSS��
	uint16_t tss_sel;		// tssѡ����
	
	list_node_t run_node;		// ������ؽ��
	list_node_t wait_node;		// �ȴ�����
	list_node_t all_node;		// ���ж��н��
}task_t;

int task_init (task_t *task,const char* name, int flag, uint32_t entry, uint32_t esp);
void task_switch_from_to (task_t * from, task_t * to);
void task_set_ready(task_t *task);
void task_set_block (task_t *task);
void task_set_sleep(task_t *task, uint32_t ticks);
void task_set_wakeup (task_t *task);
int sys_sched_yield (void);

void task_dispatch (void);
task_t * task_current (void);
void task_time_tick (void);
void sys_msleep (uint32_t ms);
// file_t * task_file (int fd);
// int task_alloc_fd (file_t * file);
// void task_remove_fd (int fd);

typedef struct _task_manager_t {
    task_t * curr_task;         // ��ǰ���е�����

	list_t ready_list;			// ��������
	list_t task_list;			// �����Ѵ�������Ķ���
	list_t sleep_list;          // ��ʱ����

	task_t first_task;			// �ں�����
	task_t idle_task;			// ��������

	int app_code_sel;			// ��������ѡ����
	int app_data_sel;			// Ӧ����������ݶ�ѡ����
}task_manager_t;

void task_manager_init (void);
void task_first_init (void);
task_t * task_first_task (void);

int sys_getpid (void);
int sys_fork (void);
int sys_execve(char *name, char **argv, char **env);
void sys_exit(int status);
int sys_wait(int* status);
void task_start(task_t * task);
#endif

