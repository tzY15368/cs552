/*
 *  ioctl test module -- Rich West.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h> /* error codes */
#include <linux/proc_fs.h>
#include <linux/tty.h>
#include <linux/uaccess.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/wait.h> // Include the wait header
#include <asm/desc.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/irq_vectors.h>


MODULE_LICENSE("GPL");
#define KBD_IRQLINE 1

#define PIC1_CMD 0x20        // PIC1 command port
#define PIC1_DATA 0x21       // PIC1 data port
#define PIC2_CMD 0xA0        // PIC2 command port
#define PIC2_DATA 0xA1       // PIC2 data port

static irqreturn_t kbd_irq_handler(int, void*);
static irqreturn_t dummy_irq_handler(int, void*);
static irqreturn_t (*kbd_default_handler_ptr)(int, void*) = NULL;

static int handle_char_input(unsigned char c);  
static char scancode[128] = "\0\e1234567890-=\b\tqwertyuiop[]\n\0asdfghjkl;'`\0\\zxcvbnm,./\0*\0 \0\0\0\0\0\0\0\0\0\0\0\0\000789-456+1230.\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";

static int lshiftState = 0;
static int lctrlState = 0;
static int recordingState = 0;

static char input_buffer[10];
static char rec_buffer[10];
static char rec_item_count = 0;
static unsigned short kbd_buffer = 0x0000; /* HByte=Status, LByte=Scancode */

static wait_queue_head_t kbd_irq_waitq;

/* 'printk' version that prints to active tty. */
void my_printk(char *string)
{
  struct tty_struct *my_tty;

  my_tty = current->signal->tty;

  if (my_tty != NULL) {
    (*my_tty->driver->ops->write)(my_tty, string, strlen(string));
    (*my_tty->driver->ops->write)(my_tty, "\015\012", 2);
  }
} 

static inline unsigned char inb( unsigned short usPort ) {

    unsigned char uch;
   
    asm volatile( "inb %1,%0" : "=a" (uch) : "Nd" (usPort) );
    return uch;
}

static inline void outb( unsigned char uch, unsigned short usPort ) {

    asm volatile( "outb %0,%1" : : "a" (uch), "Nd" (usPort) );
}


char my_getchar ( void ) {

  char c;
  /* Poll keyboard status register at port 0x64 checking bit 0 to see if
   * output buffer is full. We continue to poll if the msb of port 0x60
   * (data port) is set, as this indicates out-of-band data or a release
   * keystroke
   */
  while( !(inb( 0x64 ) & 0x1) || ( ( c = inb( 0x60 ) ) & 0x80 ) );
  // return handle_char_input(c);
  return scancode[(int)c];
}

/* attribute structures */
struct ioctl_test_t {
  int field1;
  char field2;
};

#define IOCTL_TEST _IOW(0, 6, struct ioctl_test_t)
#define IOCTL_GETCH _IOR('k', 7, char)

static int pseudo_device_ioctl(struct inode *inode, struct file *file,
			       unsigned int cmd, unsigned long arg);
static int pseudo_device_read(struct file *, char __user *, size_t, loff_t *);

static struct file_operations pseudo_dev_proc_operations;

static struct proc_dir_entry *proc_entry;


void IRQ_set_mask(uint8_t IRQline) {
    unsigned short port;
    unsigned char value;
 
    if(IRQline < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        IRQline -= 8;
    }
    value = inb(port) | (1 << IRQline);
    outb(port, value);   
    printk("kb disabled: %d\n", value);         
}
 
void IRQ_clear_mask(uint8_t IRQline) {
    unsigned short port;
    unsigned char value;
 
    if(IRQline < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        IRQline -= 8;
    }
    value = inb(port) & ~(1 << IRQline);
    outb(port, value);  
    printk("kb enabled: %d\n", value);  
}

static void disable_default_kb(){
  local_irq_disable();
  IRQ_set_mask(KBD_IRQLINE);

  struct desc_ptr idtr;
  store_idt(&idtr); 
  unsigned long handler_address;
  unsigned long idt_base = idtr.address;
  gate_desc *idt_entry = (gate_desc *)(idt_base + 1 * sizeof(gate_desc));
  printk("gate type: %d\n", idt_entry->type);

  // Extract the offset of the default handler    
  handler_address = (idt_entry->base0) |
                      (idt_entry->base1 << 16) |
                      (idt_entry->base2 << 24);
  kbd_default_handler_ptr = (irqreturn_t (*)(int, void*))handler_address;

  handler_address = (unsigned long)dummy_irq_handler;
  idt_entry->base0 = (unsigned short)(handler_address & 0xFFFF);
  idt_entry->base1 = (unsigned char)((handler_address >> 16) & 0xFF);
  idt_entry->base2 = (unsigned char)((handler_address >> 24) & 0xFF);

  local_irq_enable();
  load_idt(&idtr);
  printk("Reset Keyboard IRQ Handler Address: default: 0x%lx, dummy: 0x%lx\n", (unsigned long) kbd_default_handler_ptr, (unsigned long)dummy_irq_handler);
}

static void enable_default_kb(){
  local_irq_disable();
  IRQ_clear_mask(KBD_IRQLINE);
  
  struct desc_ptr idtr, idtr_new;
  unsigned long default_handler = (unsigned long)kbd_default_handler_ptr;

  // Access the IDT descriptor
  store_idt(&idtr);

  // Calculate the address of the keyboard interrupt entry (IRQ 1)
  unsigned long idt_base = idtr.address;
  gate_desc *idt_entry = (gate_desc *)(idt_base + 1 * sizeof(gate_desc));


  idt_entry->base0 = (unsigned short)(default_handler & 0xFFFF);
  idt_entry->base1 = (unsigned char)((default_handler >> 16) & 0xFF);
  idt_entry->base2 = (unsigned char)((default_handler >> 24) & 0xFF);
  
  // loading the same IDT should be fine?
  local_irq_enable();
  load_idt(&idtr);
  printk("Unset Keyboard IRQ Handler Address: back to: 0x%lx\n", default_handler);
}

static int __init mod_init(void) {
  int req_ret;
  remove_proc_entry("ioctl_test", NULL);

  printk("<1> Loading module\n");    
  // original_kbd_irq_handler = irq_get_handler(KBD_IRQLINE);

  // free_irq(KBD_IRQLINE, NULL);

  disable_default_kb();
  // printk("<1> Original handler: %p\n", kbd_default_handler_ptr);

  pseudo_dev_proc_operations.ioctl = pseudo_device_ioctl;
  pseudo_dev_proc_operations.read = pseudo_device_read;
  /* Start create proc entry */
  proc_entry = create_proc_entry("ioctl_test", 0444, NULL);
  if(!proc_entry)
  {
    printk("<1> Error creating /proc entry.\n");
    return 1;
  }

  //proc_entry->owner = THIS_MODULE; <-- This is now deprecated
  proc_entry->proc_fops = &pseudo_dev_proc_operations;

  req_ret = request_irq(KBD_IRQLINE, kbd_irq_handler, IRQF_SHARED , "kbd", proc_entry);
  if(req_ret!=0){
    printk("<1> Error creating IRQ: %d.\n", req_ret);
    my_printk("Error creating IRQ.\n");
    return 1;
  }
	init_waitqueue_head(&kbd_irq_waitq);
  
  // outb(inb(0x21) | (1 << KBD_IRQLINE), 0x21);

  return 0;
}



static void mod_cleanup(void) {

  printk("<1> Dumping module\n");
  remove_proc_entry("ioctl_test", NULL);
  free_irq(KBD_IRQLINE, (void*) proc_entry);
  
  enable_default_kb();
  // if(original_kbd_irq_handler){
  //   // irq_set_handler(KBD_IRQLINE, original_kbd_irq_handler);
  //   request_irq(KBD_IRQLINE, original_kbd_irq_handler, IRQF_SHARED , "kbd", NULL);
  // }
  return;
}


/***
 * ioctl() entry point...
 */
static int pseudo_device_ioctl(struct inode *inode, struct file *file,
				unsigned int cmd, unsigned long arg)
{
  struct ioctl_test_t ioc;
  char c = 'a';
  
  switch (cmd){

  case IOCTL_TEST:
    copy_from_user(&ioc, (struct ioctl_test_t *)arg, 
		   sizeof(struct ioctl_test_t));
    printk("<1> ioctl: call to IOCTL_TEST (%d,%c)!\n", 
	   ioc.field1, ioc.field2);

    my_printk ("Got msg in kernel\n");
    break;
  case IOCTL_GETCH:
    // char c = my_getchar();
    // my_printk(sprintf("Got char %c", c));
    
    // if(copy_to_user(&c, (char *)arg, sizeof(char)) != 0){
    //   return -EFAULT;
    // }
    c = my_getchar();
    copy_to_user((char *)arg, &c, sizeof(char));
    // my_printk ("sent a\n");
    break;
  default:
    return -EINVAL;
    break;
  }
  
  return 0;
}

static int handle_char_input(unsigned char c){
  // printk("got c %d\n", c);
  char res;
  char new_res;
  int i;
  int ret_val = 1;
  // toggle lshift and lctrl
  if(c == 42){
    lshiftState = 1;
  } else if(c == 170){
    lshiftState = 0;
  } else if(c == 29){
    lctrlState = 1;
  } else if(c == 157){
    lctrlState = 0;
  }

  // change kbd-buffer state
  if(c < 128){
    res = scancode[(int)c];
    if(lshiftState==1){
      if(res >= 'a' && res <= 'z'){
        res = res - 32;
        // printk("got caps: %d %c --> %c\n", res, res, res+32);
      }
    }
    if(lctrlState==0){
      switch (recordingState)
      {
        case 1:
          if(rec_item_count == 9){
            my_printk("kbd_irq: ctrl-r buffer full\n");
            recordingState = 0;
            break;
          }
          rec_buffer[rec_item_count] = res;
          rec_item_count++;
          break;
        case 2:
          if(rec_item_count == 9){
            my_printk("kbd_irq: ctrl-R buffer full\n");
            recordingState = 0;
            break;
          }
          if('a' <= res && res <= 'z'){
            new_res = res - 32;
          } else {
            new_res = res;
          }
          rec_buffer[rec_item_count] = new_res;
          rec_item_count++;
          break;
        case 0:
          kbd_buffer = (unsigned short) res;
          input_buffer[0] = res;
          input_buffer[1] = '\0';
          ret_val = 0;
          break;
      }
    } else {
      switch(res){
        case 'h':
          printk("kbd_irq: ctrl-r pressed\n");
          recordingState = 1;
          for(i = 0; i < 10; i++){
            rec_buffer[i] = '\0';
          }
          rec_item_count = 0;
          break;
        case 'H':
          recordingState = 2;
          for(i = 0; i < 10; i++){
            rec_buffer[i] = '\0';
          }
          rec_item_count = 0;
          printk("kbd_irq: ctrl-R pressed\n");
          break;
        case 'k':
          printk("kbd_irq: ctrl-p pressed\n");
          rec_buffer[9] = '\0';
          printk("ctrlp dump: rec_buffer: %s\n", rec_buffer);
          recordingState = 0;
          rec_item_count = 0;
          for(i = 0; i < 10; i++){
            input_buffer[i] = rec_buffer[i];
            rec_buffer[i] = '\0';
          }


          ret_val = 0;
          break;
      }
    }
    // printk("kbd_irq: char %d %04x --> %c (%c) , shift press: %d  \n", c, c, scancode[(int)c], res,  shift_pressed);
    if(32 <= res && res <= 126){
      printk("kbd_irq: char %d %c ls:%d lc:%d\n", res, res, lshiftState, lctrlState);
    } else {
      printk("kbd_irq: char %d ??? ls:%d lc:%d\n", res, lshiftState, lctrlState);
    }
  }
  return ret_val;
}

// Keyboard interrupt handler. Retrieves the character code (scancode)
// and keyboard status from the keyboard I/O ports. Awakes processes
// waiting for a keyboard event.
static irqreturn_t kbd_irq_handler(int irq, void* dev_id)
{
  // struct desc_ptr idtr;
  // store_idt(&idtr); 
  // local_irq_disable();
  // unsigned long handler_address;
  // unsigned long idt_base = idtr.address;
  // gate_desc *idt_entry = (gate_desc *)(idt_base + 1 * sizeof(gate_desc));

  // // Extract the offset of the default handler    
  // handler_address = (idt_entry->base0) |
  //                     (idt_entry->base1 << 16) |
  //                     (idt_entry->base2 << 24);
  //printk("handler: gate type: %d, handler addr: 0x%lx\n", idt_entry->type, handler_address);
	unsigned char status, c;
  unsigned char result;
  // checking status is not needed as we don't do polling
	status = inb(0x64);
	c = inb(0x60);

  int ret = handle_char_input(c);
  if(ret == 1){
    return IRQ_HANDLED;
  }
	wake_up_interruptible(&kbd_irq_waitq);
  return IRQ_HANDLED;
}

static irqreturn_t dummy_irq_handler(int irq, void* dev_id){
  printk("dummy irq handler\n");
  return IRQ_HANDLED;
}

static int pseudo_device_read(struct file *file, char __user *buf, size_t size, loff_t *off)
{
  char buffer[128]; // Adjust the buffer size as needed
	// my_printk ("in kernel module read function\n");
  interruptible_sleep_on(&kbd_irq_waitq);
    
    // Format a string into the buffer using snprintf
  snprintf(buffer, sizeof(buffer), "did wakeup -- kbd-buffer: %d\n", kbd_buffer);
  buffer[sizeof(buffer) - 1] = '\0'; // snprintf does not guarantee null termination
  printk("kbd_irq: read: %s\n", input_buffer);
  
  copy_to_user(buf, input_buffer, 10);
  return(sizeof(input_buffer));
	// return 0;
}


module_init(mod_init); 
module_exit(mod_cleanup); 