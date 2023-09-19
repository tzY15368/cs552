#include <linux/module.h>
#include <linux/init.h>

MODULE_LICENSE("GPL");

static int initialization_routine(void) {

  printk ("Hello, world!\n");

  return 0;
}

static void cleanup_routine(void) {

  printk ("Unloading module!\n");
}

module_init(initialization_routine);
module_exit(cleanup_routine);