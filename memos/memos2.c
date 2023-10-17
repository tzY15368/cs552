char *video = (char *)0xB8000;

void print_memory_size(unsigned int size) {
    char *msg = "Hello There 2!";
    while (*msg) {
        *video++ = *msg++;
        *video++ = 0x07;
    }
}

void __stack_chk_fail(void) {
    while (1);
}
