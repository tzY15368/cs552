#define IDT_H 1

#ifndef UTILS_H
#include "utils.h"
#endif

#define IDTBASE 0x00000000
#define IDTSIZE 0xFF

typedef struct {
    unsigned short offset_low;
    unsigned short selector;
    unsigned char zero;
    unsigned char flags;
    unsigned short offset_high;
} __attribute__((packed)) idt_entry_t;

typedef struct {
    unsigned short limit;
    unsigned long long base;
} __attribute__((packed)) idtr_t;

static idt_entry_t idt[256];
static idtr_t idtr;

void idt_set_gate(
        unsigned short interrupt_number, 
        unsigned long long offset, 
        unsigned short selector, 
        unsigned short type) {
    idt[interrupt_number].offset_low = offset & 0xFFFF;
    idt[interrupt_number].selector = selector;
    idt[interrupt_number].zero = 0;
    idt[interrupt_number].flags = type;
    idt[interrupt_number].offset_high = offset >> 16;
}

void exception_handler(void) {
    tprintf("Exception!");
}

void idt_init() {
    idtr.limit = sizeof(idt) - 1;
    idtr.base = (unsigned long long)idt;
    // Set all interrupt gates to point to a default exception handler.
    for(int i = 0; i < 256; i++) {
        idt_set_gate(i, (unsigned long long)exception_handler, 0x08, 0x8E);
    }
    __asm__ volatile ("lidt %0" : : "m" (idtr));
}
