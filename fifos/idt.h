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

#ifndef THREAD_H
#include "thread.h"
#endif

#define IDT_MAX_DESCRIPTORS 256

extern void* isr_wrapper();
extern void* timer_wrapper();

void interrupt_handler() {
    tprintf(".");
    PIC_sendEOI(0);
}

void timer_handler(){
    // tprintf("[%d]", read_pit_count());
    
    PIC_sendEOI(0);
    // IRQ_set_mask(0);
    tprintf("`");
    thread_ctl_blk_t* tcb = get_current_tcb(FALSE);
    if(tcb != NULL){
        thread_preempt();
    }
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
    // For example, IRQ 0 is typically associated with interrupt vector 32.
    isr_stub_table[0x20] = timer_wrapper;

    for (uint8_t vector = 0; vector < 48; vector++) {
        idt_set_descriptor(vector, isr_stub_table[vector], 0x8E);
        // vectors[vector] = true;
    }
 
    __asm__ volatile ("lidt %0" : : "m"(idtr)); // load the new IDT
    __asm__ volatile ("sti"); // set the interrupt flag
}