/**
 * �Լ�����д����ϵͳ
 *
 * 16λ��������
 * �����������������Ӳ����⣬���뱣��ģʽ��Ȼ������ںˣ�����ת���ں�����
 *
 * ���ߣ�����ͭ
 * ��ϵ����: 527676163@qq.com
 */

// 16λ���룬������Ϸ��ڿ�ͷ���Ա���Щioָ������Ϊ32λ
__asm__(".code16gcc");

#include "loader.h"

boot_info_t boot_info;			// ����������Ϣ

/**
 * BIOS����ʾ�ַ���
 */
static void show_msg (const char * msg) {
    char c;

	// ʹ��biosд�Դ棬��������д
	while ((c = *msg++) != '\0') {
		__asm__ __volatile__(
				"mov $0xe, %%ah\n\t"
				"mov %[ch], %%al\n\t"
				"int $0x10"::[ch]"r"(c));
	}
}

// �ο���https://wiki.osdev.org/Memory_Map_(x86)
// 1MB���±Ƚϱ�׼, ��1M���ϻ��в��
// ��⣺https://wiki.osdev.org/Detecting_Memory_(x86)#BIOS_Function:_INT_0x15.2C_AH_.3D_0xC7
static void  detect_memory(void) {
	uint32_t contID = 0;
	SMAP_entry_t smap_entry;
	int signature, bytes;

    show_msg("try to detect memory:");

	// ���Σ�EDX=0x534D4150,EAX=0xE820,ECX=24,INT 0x15, EBX=0�����Σ�
	// ������EAX=0xE820,ECX=24,
	// �����жϣ�EBX=0
	boot_info.ram_region_count = 0;
	for (int i = 0; i < BOOT_RAM_REGION_MAX; i++) {
		SMAP_entry_t * entry = &smap_entry;

		__asm__ __volatile__("int  $0x15"
			: "=a"(signature), "=c"(bytes), "=b"(contID)
			: "a"(0xE820), "b"(contID), "c"(24), "d"(0x534D4150), "D"(entry));
		if (signature != 0x534D4150) {
            show_msg("failed.\r\n");
			return;
		}

		// todo: 20�ֽ�
		if (bytes > 20 && (entry->ACPI & 0x0001) == 0){
			continue;
		}

        // ����RAM��Ϣ��ֻȡ32λ���ռ��������迼�Ǹ������������
        if (entry->Type == 1) {
            boot_info.ram_region_cfg[boot_info.ram_region_count].start = entry->BaseL;
            boot_info.ram_region_cfg[boot_info.ram_region_count].size = entry->LengthL;
            boot_info.ram_region_count++;
        }

		if (contID == 0) {
			break;
		}
	}
    show_msg("ok.\r\n");
}

// GDT����ʱ�ã��������ݻ��滻���Լ���
uint16_t gdt_table[][4] = {
    {0, 0, 0, 0},
    {0xFFFF, 0x0000, 0x9A00, 0x00CF},
    {0xFFFF, 0x0000, 0x9200, 0x00CF},
};

/**
 * ���뱣��ģʽ
 */
static void  enter_protect_mode() {
    // ���ж�
    cli();

    // ����A20��ַ�ߣ�ʹ�ÿɷ���1M���Ͽռ�
    // ʹ�õ���Fast A20 Gate��ʽ����https://wiki.osdev.org/A20#Fast_A20_Gate
    uint8_t v = inb(0x92);
    outb(0x92, v | 0x2);

    // ����GDT�������ж��Ѿ��ص���IDT����Ҫ����
    lgdt((uint32_t)gdt_table, sizeof(gdt_table));

    // ��CR0�ı���ģʽλ�����뱣��ģʽ
    uint32_t cr0 = read_cr0();
    write_cr0(cr0 | (1 << 0));


    // ����ת���뵽����ģʽ
    // ʹ�ó���ת���Ա������ˮ�ߣ��������16λ��������
    far_jump(8, (uint32_t)protect_mode_entry);
}

void loader_entry(void) {
    show_msg("....loading.....\r\n");
	detect_memory();
    enter_protect_mode();
    for(;;) {}
}


