#define GDT_H 1

#ifndef TYPES_H
#include "types.h"
#endif

#ifndef UTILS_H
#include "utils.h"
#endif

#define N_GDT_ENTRIES 3

// http://www.jamesmolloy.co.uk/tutorial_html/4.-The%20GDT%20and%20IDT.html

// Lets us access our ASM functions from our C code.
extern void gdt_flush(uint32_t);

// This structure contains the value of one GDT entry.
// We use the attribute 'packed' to tell GCC not to change
// any of the alignment in the structure.
struct gdt_entry_struct
{
  uint16_t limit_low;           // The lower 16 bits of the limit.
  uint16_t base_low;            // The lower 16 bits of the base.
  uint8_t  base_middle;         // The next 8 bits of the base.
  uint8_t  access;              // Access flags, determine what ring this segment can be used in.
  uint8_t  granularity;
  uint8_t  base_high;           // The last 8 bits of the base.
} __attribute__((packed));
typedef struct gdt_entry_struct gdt_entry_t;

struct gdt_ptr_struct
{
   uint16_t limit;               // The upper 16 bits of all selector limits.
   uint32_t base;                // The address of the first gdt_entry_t struct.
} __attribute__((packed));
typedef struct gdt_ptr_struct gdt_ptr_t;

gdt_entry_t gdt_entries[N_GDT_ENTRIES];
gdt_ptr_t   gdt_ptr;


static void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran)
{
   gdt_entries[num].base_low    = (base & 0xFFFF);
   gdt_entries[num].base_middle = (base >> 16) & 0xFF;
   gdt_entries[num].base_high   = (base >> 24) & 0xFF;

   gdt_entries[num].limit_low   = (limit & 0xFFFF);
   gdt_entries[num].granularity = (limit >> 16) & 0x0F;

   gdt_entries[num].granularity |= gran & 0xF0;
   gdt_entries[num].access      = access;
}

static void init_gdt()
{
   gdt_ptr.limit = (sizeof(gdt_entry_t) * N_GDT_ENTRIES) - 1;
   gdt_ptr.base  = (uint32_t)&gdt_entries;

   gdt_set_gate(0, 0, 0, 0, 0);                // Null segment
   gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // Code segment
   gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // Data segment

   gdt_flush((uint32_t)&gdt_ptr);
}

static void init_descriptor_tables()
{
   // Initialise the global descriptor table.
   init_gdt();
}

