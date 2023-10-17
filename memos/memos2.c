char *video = (char *)0xB8000;

void print_memory_size(unsigned int size) {
    char *msg = "RAM Size (KB): ";
    while (*msg) {
        *video++ = *msg++;
        *video++ = 0x07;
    }

    char buffer[10];
    itoa(size, buffer);

    char *num = buffer;
    while (*num) {
        *video++ = *num++;
        *video++ = 0x07;
    }
}

void __stack_chk_fail(void) {
    while (1);
}
