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

static inline unsigned char inb( unsigned short usPort ) {
    unsigned char uch;
    __asm__ volatile ( "inb %1,%0" : "=a" (uch) : "Nd" (usPort) );
    return uch;
}

static inline void outb( unsigned char uch, unsigned short usPort ) {
    __asm__ volatile ( "outb %0,%1" : : "a" (uch), "Nd" (usPort) );
}

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

void interrupt_handler(unsigned long long interrupt_number) {
	terminal_writestring("Interrupt: ");
	terminal_writeint(interrupt_number);
	terminal_writestring("\n");
	// if(interrupt_number < 8) 
	// 	outb(0x20, 0x20);
	// else if(interrupt_number < 16)
	// 	outb(0x20, 0xA0);
	__asm__ volatile ("iret");
}

void idt_init() {
	__asm__ volatile ("cli");
	idtr.limit = sizeof(idt) - 1;
	idtr.base = (unsigned long long)idt;
	for(int i = 0; i < 256; i++) {
		idt_set_gate(i, (unsigned long long)interrupt_handler, 0x08, 0x8E);
	}
	__asm__ volatile ("lidt %0" : : "m" (idtr));
	__asm__ volatile ("sti");
}
