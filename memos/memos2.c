typedef struct multiboot_info {
    unsigned int flags;
    unsigned int mem_lower;
    unsigned int mem_upper;
    unsigned int boot_device;
    unsigned int cmdline;
    unsigned int mods_count;
    unsigned int mods_addr;
    unsigned int num;
    unsigned int size;
    unsigned int addr;
    unsigned int shndx;
    unsigned int mmap_length;
    unsigned int mmap_addr;
} multiboot_info_t;

typedef struct multiboot_memory_map {
    unsigned int size;
    unsigned int base_addr_low;
    unsigned int base_addr_high;
    unsigned int length_low;
    unsigned int length_high;
    unsigned int type;
} multiboot_memory_map_t;

char *video = (char *)0xB8000;

void itoa(unsigned int value, char *str) {
    char *start = str;
    do {
        *str++ = '0' + (value % 10);
        value /= 10;
    } while (value);
    *str-- = '\0';
    while (start < str) {
        char temp = *start;
        *start++ = *str;
        *str-- = temp;
    }
}

void itoa16(unsigned int value, char *str) {
    char *start = str;
    do {
        unsigned int digit = value % 16;
        if (digit < 10) {
            *str++ = '0' + digit;
        } else {
            *str++ = 'a' + digit - 10;
        }
        value /= 16;
    } while (value);
    *str-- = '\0';
    while (start < str) {
        char temp = *start;
        *start++ = *str;
        *str-- = temp;
    }
}

void print_memory_size(unsigned int size) {
    char *msg = "MemOS: Welcome *** System Memory is: ";
    while(*msg) {
        *video++ = *msg++;
        *video++ = 0x07;
    }
    char buffer[10];
    itoa(size, buffer);
    char *num = buffer;
    while(*num) {
        *video++ = *num++;
        *video++ = 0x07;
    }
    char *ending = "MB";
    while(*ending) {
        *video++ = *ending++;
        *video++ = 0x07;
    }
    while(video < (char *)0xB8140) {
        *video++ = ' ';
        *video++ = 0x07;
    }
}

void print_memory_map_entry(multiboot_memory_map_t *mmap) {
    char *msg = "Memory [0x";
    while(*msg) {
        *video++ = *msg++;
        *video++ = 0x07;
    }
    char buffer[10];
    itoa16(mmap->base_addr_high, buffer);
    char *num = buffer;
    while(*num) {
        *video++ = *num++;
        *video++ = 0x07;
    }
    itoa16(mmap->base_addr_low, buffer);
    num = buffer;
    while(*num) {
        *video++ = *num++;
        *video++ = 0x07;
    }
    char *msg2 = " - 0x";
    while(*msg2) {
        *video++ = *msg2++;
        *video++ = 0x07;
    }
    itoa16(mmap->base_addr_high + mmap->length_high, buffer);
    num = buffer;
    while(*num) {
        *video++ = *num++;
        *video++ = 0x07;
    }
    itoa16(mmap->base_addr_low + mmap->length_low, buffer);
    num = buffer;
    while(*num) {
        *video++ = *num++;
        *video++ = 0x07;
    }
    char *msg3 = "] status: ";
    while(*msg3) {
        *video++ = *msg3++;
        *video++ = 0x07;
    }
    itoa(mmap->type, buffer);
    num = buffer;
    while(*num) {
        *video++ = *num++;
        *video++ = 0x07;
    }
    while(((unsigned int)video - 0xB8000) % 160 != 0) {
        *video++ = ' ';
        *video++ = 0x07;
    }
}

void c_handle_boot(multiboot_info_t* mb_info) {
    if(mb_info->flags & 0x1) {
        unsigned int total_mem_kb = mb_info->mem_upper + 1024;
        print_memory_size(total_mem_kb / 1024);
        if (mb_info->flags & (1 << 6)) {
            multiboot_memory_map_t *mmap;
            for (mmap = (multiboot_memory_map_t *) mb_info->mmap_addr;
                (unsigned int) mmap < mb_info->mmap_addr + mb_info->mmap_length;
                mmap = (multiboot_memory_map_t *) ((unsigned int) mmap + mmap->size + sizeof(mmap->size))) {
                print_memory_map_entry(mmap);
            }
        }
    } else {
        char *msg = "Memory size information not available.  ";
        while(*msg) {
            *video++ = *msg++;
            *video++ = 0x07;
        }
    }
}

void __stack_chk_fail(void) {
    while (1);
}
