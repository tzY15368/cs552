#define IDT_H 1

#ifndef TYPES_H
#include <types.h>
#endif

#ifndef UTILS_H
#include <utils.h>
#endif

#ifndef PIC_H
#include "pic.h"
#endif

#define IDT_MAX_DESCRIPTORS 256

extern void* isr_wrapper();
void interrupt_handler() {
    tprintf(".");
    // __asm__ volatile ("cli; hlt"); // Completely hangs the computer
    PIC_sendEOI();
}

void timer_handler(){
    tprintf("Timer handler called\n");
    // __asm__ volatile ("cli; hlt"); // Completely hangs the computer
    PIC_sendEOI();
}

__attribute__((aligned(0x10))) 
static idt_entry_t idt[256]; // Create an array of IDT entries; aligned for performance

typedef struct {
	uint16_t	limit;
	uint32_t	base;
} __attribute__((packed)) idtr_t;

static idtr_t idtr;

void idt_set_descriptor(uint8_t vector, void* isr, uint8_t flags) {
    idt_entry_t* descriptor = &idt[vector];
 
    descriptor->isr_low        = (uint32_t)isr & 0xFFFF;
    descriptor->kernel_cs      = 0x08; // this value can be whatever offset your kernel code selector is in your GDT
    descriptor->attributes     = flags;
    descriptor->isr_high       = (uint32_t)isr >> 16;
    descriptor->reserved       = 0;
}

static void* isr_stub_table[48];
 
void idt_init() {
    idtr.base = (uint32_t)&idt[0];
    idtr.limit = (uint16_t)sizeof(idt_entry_t) * IDT_MAX_DESCRIPTORS - 1;
 
    for(int i=0;i<48;i++){
        isr_stub_table[i] = isr_wrapper;
    }
    isr_stub_table[0x20] = timer_handler;

    for (uint8_t vector = 0; vector < 48; vector++) {
        idt_set_descriptor(vector, isr_stub_table[vector], 0x8E);
        // vectors[vector] = true;
    }
 
    __asm__ volatile ("lidt %0" : : "m"(idtr)); // load the new IDT
    __asm__ volatile ("sti"); // set the interrupt flag
}