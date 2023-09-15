/*
 *  ioctl test module -- Rich West.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h> /* error codes */
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/tty.h>
#include <linux/sched.h>

MODULE_LICENSE("GPL");


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

static struct file_operations pseudo_dev_proc_operations;

static struct proc_dir_entry *proc_entry;

static int __init mod_init(void) {
  printk("<1> Loading module\n");

  pseudo_dev_proc_operations.ioctl = pseudo_device_ioctl;

  /* Start create proc entry */
  proc_entry = create_proc_entry("kbdev_test", 0444, NULL);
  if(!proc_entry)
  {
    printk("<1> Error creating /proc entry.\n");
    return 1;
  }

  //proc_entry->owner = THIS_MODULE; <-- This is now deprecated
  proc_entry->proc_fops = &pseudo_dev_proc_operations;

  return 0;
}



static void mod_cleanup(void) {

  printk("<1> Dumping module\n");
  remove_proc_entry("ioctl_test", NULL);

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


module_init(mod_init); 
module_exit(mod_cleanup); 