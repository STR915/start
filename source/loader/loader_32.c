#include "loader.h"
#include "comm/elf.h"
static void read_disk(uint32_t sector, uint32_t sector_count,uint8_t *buf){
    outb(0x1F6, (uint8_t) (0xE0));

	outb(0x1F2, (uint8_t) (sector_count >> 8));
    outb(0x1F3, (uint8_t) (sector >> 24));		// LBA������24~31λ
    outb(0x1F4, (uint8_t) (0));					// LBA������32~39λ
    outb(0x1F5, (uint8_t) (0));					// LBA������40~47λ

    outb(0x1F2, (uint8_t) (sector_count));
	outb(0x1F3, (uint8_t) (sector));			// LBA������0~7λ
	outb(0x1F4, (uint8_t) (sector >> 8));		// LBA������8~15λ
	outb(0x1F5, (uint8_t) (sector >> 16));		// LBA������16~23λ

	outb(0x1F7, (uint8_t) 0x24);

	// ��ȡ����
	uint16_t *data_buf = (uint16_t*) buf;
	while (sector_count-- > 0) {
		// ÿ��������֮ǰ��Ҫ��飬�ȴ����ݾ���
		while ((inb(0x1F7) & 0x88) != 0x8) {}

		// ��ȡ��������д�뵽������
		for (int i = 0; i < SECTOR_SIZE / 2; i++) {
			*data_buf++ = inw(0x1F0);
		}
	}
}

static uint32_t reload_elf_file(uint8_t* file_buffer){
    Elf32_Ehdr* elf_hdr=(Elf32_Ehdr*)file_buffer;
    if((elf_hdr->e_ident[0] != 0x7F) || (elf_hdr->e_ident[1] != 'E')
       || (elf_hdr->e_ident[2] != 'L') || (elf_hdr->e_ident[3] != 'F')){
          return 0;  
    }

   // Ȼ����м��س���ͷ�������ݿ�������Ӧ��λ��
    for (int i = 0; i < elf_hdr->e_phnum; i++) {
        Elf32_Phdr * phdr = (Elf32_Phdr *)(file_buffer + elf_hdr->e_phoff) + i;
        if (phdr->p_type != PT_LOAD) {
            continue;
        }

		// ȫ��ʹ�������ַ����ʱ��ҳ���ƻ�δ��
        uint8_t * src = file_buffer + phdr->p_offset;
        uint8_t * dest = (uint8_t *)phdr->p_paddr;
        for (int j = 0; j < phdr->p_filesz; j++) {
            *dest++ = *src++;
        }

		// memsz��filesz��ͬʱ������Ҫ��0
		dest= (uint8_t *)phdr->p_paddr + phdr->p_filesz;
		for (int j = 0; j < phdr->p_memsz - phdr->p_filesz; j++) {
			*dest++ = 0;
		}
    }

    return elf_hdr->e_entry;
}

static void die(int code){
    for(;;){}
}

/**
 * @brief ������ҳ����
 * ��0-4M�ռ�ӳ�䵽0-4M��SYS_KERNEL_BASE_ADDR~+4MB�ռ�
 * 0-4MB��ӳ����Ҫ���ڱ���loader�Լ�������������
 * SYS_KERNEL_BASE_ADDR+4MB������Ϊ�ں��ṩ��ȷ�������ַ�ռ�
 */
void enable_page_mode (void) {
#define PDE_P			(1 << 0)
#define PDE_PS			(1 << 7)
#define PDE_W			(1 << 1)
#define CR4_PSE		    (1 << 4)
#define CR0_PG		    (1 << 31)

    // ʹ��4MBҳ�飬��������ҳ��ͼ򵥺ֻܶ࣬��Ҫ1�����ɡ�
    // ���±�Ϊ��ʱʹ�ã����ڰ����ں��������У����ں���������֮�󣬽���������
    static uint32_t page_dir[1024] __attribute__((aligned(4096))) = {
        [0] = PDE_P | PDE_PS | PDE_W,			// PDE_PS������4MB��ҳ
    };

    // ����PSE���Ա�����4M��ҳ��������4KB
    uint32_t cr4 = read_cr4();
    write_cr4(cr4 | CR4_PSE);

    // ����ҳ���ַ
    write_cr3((uint32_t)page_dir);

    // ������ҳ����
    write_cr0(read_cr0() | CR0_PG);
}

void load_kernel(void){
    read_disk(100, 500, (uint8_t*)SYS_KERNEL_LOAD_ADDR);

    uint32_t kernel_entry= reload_elf_file((uint8_t*)SYS_KERNEL_LOAD_ADDR);
    if(kernel_entry==0){
        die(-1);
    }
    enable_page_mode();
    ((void (*)(boot_info_t*))kernel_entry)(&boot_info);
    for(;;){}
}