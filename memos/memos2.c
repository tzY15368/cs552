#include <multiboot.h>

#define VGA_TXT_BUFFER_ADDR 0xB8000

// big buffer for string
static char addr_buffer[30];
static char buffer[1000];
static char buf_range[800];
static int range_offset = 0;
static int buffer_offset = 0;

unsigned short *text_buffer = (unsigned short*)VGA_TXT_BUFFER_ADDR;

struct position {
	int x, y;
} cur_pos;

void pad30(char *str) {
    int i,j;
    int nz_cnt;
    char tmp_buf[30];
    // copy str first
    for(i=0;i<30;i++){
        tmp_buf[i] = str[i];
        if(str[i] != '\0'){
            nz_cnt = i+1;
        } else {
            break;
        }
    }
    for(i=0;i<15-nz_cnt;i++){
        str[i] = '0';
    }
    for(j=0;j<nz_cnt;j++){
        str[i++] = tmp_buf[j];
    }
    str[16] = '\0';
}
void clear_addr_buffer() {
    int i;
    for (i = 0; i < 30; i++) {
        addr_buffer[i] = '\0';
    }
}

void print(char *text) {

	int i;
    unsigned short *offset;
	for (i =0; text[i] != '\0'; i ++) {
        if(text[i] >= ' '){
            // assume 80 chars on terminal width
            offset = text_buffer + (cur_pos.y*80) + cur_pos.x ;
            *offset = 0x0F << 8 | text[i];
            cur_pos.x++;
        } else if(text[i] == '\n'){
            cur_pos.x = 0;
            cur_pos.y++;
        } else if(text[i] == '\r'){
            cur_pos.x = 0;
        }
	}
}
char *itoa( int value, char * str, int base )
{
    char * rc;
    char * ptr;
    char * low;
    // Check for supported base.
    if ( base < 2 || base > 36 )
    {
        *str = '\0';
        return str;
    }
    rc = ptr = str;
    // Set '-' for negative decimals.
    if ( value < 0 && base == 10 )
    {
        *ptr++ = '-';
    }
    // Remember where the numbers start.
    low = ptr;
    // The actual conversion.
    do
    {
        // Modulo is negative for negative value. This trick makes abs() unnecessary.
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz"[35 + value % base];
        value /= base;
    } while ( value );
    // Terminating the string.
    *ptr-- = '\0';
    // Invert the numbers.
    while ( low < ptr )
    {
        char tmp = *low;
        *low++ = *ptr;
        *ptr-- = tmp;
    }
    return rc;
}
void panic(char *str) {
    print(str);
    while (1) {
        // do nothing
    }
}

void append_str(char *str) {
    int i = 0;
    while (str[i] != '\0') {
        buffer[buffer_offset] = str[i];
        buffer_offset++;
        i++;
    }
}

void append_range(char *str){
    int i = 0;
    while (str[i] != '\0') {
        buf_range[range_offset] = str[i];
        range_offset++;
        i++;
    }
}

void append_range_with_padded_addr(unsigned long val){
    clear_addr_buffer();
    itoa(val, addr_buffer, 16);
    pad30(addr_buffer);
    append_range(addr_buffer);
}


void cmain(multiboot_info_t *mbi , unsigned int magic) {
    // custom_sleep(1);
    append_str("MemOS: Welcome *** System Memory is:\0");
    
    append_range("Memory Map:\n\r\0");
    clear_addr_buffer();
    itoa(mbi->mem_upper, addr_buffer, 10);
    append_range(addr_buffer);

    append_range(" KB UPPER*** \n\r\0");
    clear_addr_buffer();
    itoa(mbi->mem_lower, addr_buffer, 10);
    append_range(addr_buffer);

	unsigned long total_len = 0;
        /* Make sure the magic number matches for memory mapping*/
    if(magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        panic("invalid magic number!");
    }
 
    /* Check bit 6 to see if we have a valid memory map */
    if(!(mbi->flags >> 6 & 0x1)) {
        panic("invalid memory map given by GRUB bootloader");
    }
    int i;
    for(i = 0; i < mbi->mmap_length-(sizeof(multiboot_memory_map_t)); 
        i += sizeof(multiboot_memory_map_t)) 
    {
        multiboot_memory_map_t* mmmt = 
            (multiboot_memory_map_t*) (mbi->mmap_addr + i);

        append_range("Address Range: [");
        
        append_range_with_padded_addr(mmmt->addr);

        append_range(" : ");

        append_range_with_padded_addr(mmmt->addr - 1 + mmmt->len);

        append_range("] status: ");
        clear_addr_buffer();
        itoa(mmmt->type, addr_buffer, 16);
        append_range(addr_buffer);

        append_range("\n\r\0");
        total_len += mmmt->len;
    }
    
    char total_len_buf[20];
    total_len = ((((total_len) >> 20) &0xfff) + 1);
    itoa(total_len, total_len_buf, 10);
    append_str(total_len_buf);
    append_str(" MB *** \n\r\n\0");
    append_str(buf_range);
    print(buffer);
}