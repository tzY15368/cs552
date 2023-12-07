#define UTILS_H 1

#include "multiboot.h"

#ifndef TYPES_H
#include "types.h"
#endif

#include <stdarg.h>



#define print_esp()\
  unsigned int esp;\
  __asm__("movl %%esp, %0;" : "=r" (esp));\
  tprintf("esp: %d\n", esp);\

#define print_eip()\
  unsigned int eip;\
  __asm__("call 1f\n1: pop %0" : "=r" (eip));\
  tprintf("eip: %d\n", eip);\


void sleep(int ms){
  // just consume cpu cycles
  int v = 0;
  for(int i=0; i<ms*100000; i++){
    v += i;
  };
}


bool strcmp(char* a, char* b, int max_iter){
  int i = 0;

  while(a[i] != '\0' && b[i] != '\0' && i < max_iter){
    if(a[i] != b[i]){
      return FALSE;
    }
    i++;
  }
  return a[i] == b[i];
}

void strcpy(char* src, char* dst, int max_iter){
  int i = 0;
  while(src[i] != '\0' && i < max_iter){
    dst[i] = src[i];
    i++;
  }
  dst[i] = '\0';
}

/* Hardware text mode color constants. */
enum vga_color
{
  COLOR_BLACK = 0,
  COLOR_BLUE = 1,
  COLOR_GREEN = 2,
  COLOR_CYAN = 3,
  COLOR_RED = 4,
  COLOR_MAGENTA = 5,
  COLOR_BROWN = 6,
  COLOR_LIGHT_GREY = 7,
  COLOR_DARK_GREY = 8,
  COLOR_LIGHT_BLUE = 9,
  COLOR_LIGHT_GREEN = 10,
  COLOR_LIGHT_CYAN = 11,
  COLOR_LIGHT_RED = 12,
  COLOR_LIGHT_MAGENTA = 13,
  COLOR_LIGHT_BROWN = 14,
  COLOR_WHITE = 15,
};

uint8_t make_color(enum vga_color fg, enum vga_color bg)
{
  return fg | bg << 4;
}
 
uint16_t make_vgaentry(char c, uint8_t color)
{
  uint16_t c16 = c;
  uint16_t color16 = color;
  return c16 | color16 << 8;
}

int min(int a, int b){
  return a < b ? a : b;
}

int max(int a, int b){
  return a > b ? a : b;
}

size_t strlen(const char* str)
{
  size_t ret = 0;
  while ( str[ret] != 0 )
    ret++;
  return ret;
}
 
static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 24;
 
size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer;
 
/* Convert the integer D to a string and save the string in BUF. If
   BASE is equal to 'd', interpret that D is decimal, and if BASE is
   equal to 'x', interpret that D is hexadecimal. */
void itoa (char *buf, int base, int d)
{
  char *p = buf;
  char *p1, *p2;
  unsigned long ud = d;
  int divisor = 10;
     
  /* If %d is specified and D is minus, put `-' in the head. */
  if (base == 'd' && d < 0)
    {
      *p++ = '-';
      buf++;
      ud = -d;
    }
  else if (base == 'x')
    divisor = 16;
     
  /* Divide UD by DIVISOR until UD == 0. */
  do
    {
      int remainder = ud % divisor;
     
      *p++ = (remainder < 10) ? remainder + '0' : remainder + 'a' - 10;
    }
  while (ud /= divisor);
     
  /* Terminate BUF. */
  *p = 0;
     
  /* Reverse BUF. */
  p1 = buf;
  p2 = p - 1;
  while (p1 < p2)
    {
      char tmp = *p1;
      *p1 = *p2;
      *p2 = tmp;
      p1++;
      p2--;
    }
}
void terminal_initialize()
{
  terminal_row = 0;
  terminal_column = 0;
  terminal_color = make_color(COLOR_LIGHT_GREY, COLOR_BLACK);
  terminal_buffer = (uint16_t*) 0xB8000;
  for ( size_t y = 0; y < VGA_HEIGHT; y++ )
    {
      for ( size_t x = 0; x < VGA_WIDTH; x++ )
	{
	  const size_t index = y * VGA_WIDTH + x;
	  terminal_buffer[index] = make_vgaentry(' ', terminal_color);
	}
    }
}
 
void terminal_setcolor(uint8_t color)
{
  terminal_color = color;
}
 
void terminal_putentryat(char c, uint8_t color, size_t x, size_t y)
{
  const size_t index = y * VGA_WIDTH + x;
  terminal_buffer[index] = make_vgaentry(c, color);
}

void terminal_putchar(char c)
{
  terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
  if ( ++terminal_column == VGA_WIDTH )
    {
      terminal_column = 0;
      if ( ++terminal_row == VGA_HEIGHT )
	{
	  terminal_row = 0;
	}
    }
  if(c == '\n'){
    terminal_column = 0;
    terminal_row++;
  }
  if(c == '\r'){
    terminal_column = 0;
  }
}
 
void terminal_writestring(const char* data)
{
  size_t datalen = strlen(data);
  for ( size_t i = 0; i < datalen; i++ )
    terminal_putchar(data[i]);
}

void terminal_writeln(const char* data)
{
  terminal_writestring(data);
  terminal_putchar('\n');
}

void terminal_writeint(int data)
{
  char buf[10];
  itoa(buf, 'd', data);
  terminal_writestring(buf);
}

static bool debug = TRUE;

void tprintf(const char* fmt, ...) {
    if(!debug){
      return;
    }
    const char* p = fmt;
    char c;

    va_list args;
    char buf[12];  // Maximum int length

    va_start(args, fmt);

    while ((c = *p++)) {
        if (c != '%') {
            terminal_putchar(c);
        } else {
            c = *p++;
            switch (c) {
                case 'c':
                    terminal_putchar(va_arg(args, int));
                    break;
                case 's':
                    terminal_writestring(va_arg(args, const char*));
                    break;
                case 'd':
                    itoa(buf, 'd', va_arg(args, int));
                    terminal_writestring(buf);
                    break;
                default:
                    terminal_putchar('%');
                    terminal_putchar(c);
                    break;
            }
        }
    }

    va_end(args);
}

void halt(){
  tprintf("halt\n");
  while(1);
}


void panic(const char* fmt, ...){
  va_list args;
  va_start(args, fmt);
  tprintf(fmt, args);
  va_end(args);
  halt();
}

static inline void outb(uint16_t port, uint8_t val)
{
    __asm__ volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
    /* There's an outb %al, $imm8  encoding, for compile-time constant port numbers that fit in 8b.  (N constraint).
     * Wider immediate constants would be truncated at assemble-time (e.g. "i" constraint).
     * The  outb  %al, %dx  encoding is the only option for all other cases.
     * %1 expands to %dx because  port  is a uint16_t.  %w1 could be used if we had the port number a wider C type */
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t ret;
    __asm__ volatile ( "inb %1, %0"
                   : "=a"(ret)
                   : "Nd"(port) );
    return ret;
}

static inline void io_wait(void)
{
     //Port 0x80 is used for 'checkpoints' during POST. 
     //The Linux kernel seems to think it is free for use :-/ 
    __asm__ volatile ( "outb %%al, $0x80" : : "a"(0) );
 //    %%al instead of %0 makes no difference.  TODO: does the register need to be zeroed? 
}