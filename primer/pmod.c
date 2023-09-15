/*
 *  ioctl test module -- Rich West.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h> /* error codes */
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/tty.h>

#include <linux/uaccess.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <asm/irq.h>
#include <linux/sched.h>
#include <linux/wait.h> // Include the wait header


MODULE_LICENSE("GPL");
#define KBD_IRQLINE 3

static irqreturn_t kbd_irq_handler(int, void*);
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

static int shift_pressed = 0;
static int ctrl_pressed = 0;
static int caps_pressed = 0;

char my_getchar ( void ) {

  char c;
  char res;
  static char scancode[128] = "\0\e1234567890-=\b\tqwertyuiop[]\n\0asdfghjkl;'`\0\\zxcvbnm,./\0*\0 \0\0\0\0\0\0\0\0\0\0\0\0\000789-456+1230.\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
  int prev_keypress = shift_pressed;
  int prev_ctrl = ctrl_pressed;
  /* Poll keyboard status register at port 0x64 checking bit 0 to see if
   * output buffer is full. We continue to poll if the msb of port 0x60
   * (data port) is set, as this indicates out-of-band data or a release
   * keystroke
   */
  while( !(inb( 0x64 ) & 0x1) || ( ( c = inb( 0x60 ) ) & 0x80 ) );

  // return scancode[ (int)c ];
    
    printk("Got char %d %04x --> %c, shift press: %d  \n", c, c, scancode[(int)c], shift_pressed);
    if(c == 42){ // 2a
      shift_pressed = 1;
    } else {
      shift_pressed = 0;
    }

    if(c == 29){ // 1d
      ctrl_pressed = 1;
    } else {
      ctrl_pressed = 0;
    }
    
    if(c == 58){ // 3a
      caps_pressed = !caps_pressed;
    }

    res = scancode[(int)c];
    if (prev_keypress || caps_pressed) {
      if (res >= 'a' && res <= 'z') {
        res = res - 32;
        printk("got caps: %d %c --> %c\n", res, res, res+32);
        // printk("shift pressed: %d", res);
      }
    }
    return res;
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

static int __init mod_init(void) {
  int req_ret;
  printk("<1> Loading module\n");

  pseudo_dev_proc_operations.ioctl = pseudo_device_ioctl;
  pseudo_dev_proc_operations.read = pseudo_device_read;
  /* Start create proc entry */
  proc_entry = create_proc_entry("kbdev_test", 0444, NULL);
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

  return 0;
}



static void mod_cleanup(void) {

  printk("<1> Dumping module\n");
  remove_proc_entry("ioctl_test", NULL);
  free_irq(KBD_IRQLINE, (void*) proc_entry);
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

// Keyboard interrupt handler. Retrieves the character code (scancode)
// and keyboard status from the keyboard I/O ports. Awakes processes
// waiting for a keyboard event.
static irqreturn_t kbd_irq_handler(int irq, void* dev_id)
{
	unsigned char status, scancode;
	status = inb(0x64);
	scancode = inb(0x60);
	kbd_buffer = (unsigned short) ((status << 8) | (scancode & 0x00ff));
  my_printk("did trigger handler");
	wake_up_interruptible(&kbd_irq_waitq);
  return IRQ_HANDLED;
} // kbd_irq_handler()

static int pseudo_device_read(struct file *file, char __user *buf, size_t size, loff_t *off)
{
	my_printk ("in kernel module read function\n");
  interruptible_sleep_on(&kbd_irq_waitq);
	// copy_to_user (buf, "hello!!!\0", 9);
  
	copy_to_user((void*) buf, (void*) &kbd_buffer, sizeof(kbd_buffer));
  return(sizeof(kbd_buffer));
	// return 0;
}


module_init(mod_init); 
module_exit(mod_cleanup); 