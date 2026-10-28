#include <linux/init.h>
#include <linux/module.h>
#include <linux/printk.h>

static int __init hellod_init(void)
{
    printk(KERN_INFO "Hello!\n");
    return 0;
}

static void __exit hellod_exit(void)
{
    printk(KERN_INFO "Goodbye!\n");
}

module_init(hellod_init);
module_exit(hellod_exit);

MODULE_LICENSE("MIT");
MODULE_DESCRIPTION("Lab6 simple Linux kernel module");
