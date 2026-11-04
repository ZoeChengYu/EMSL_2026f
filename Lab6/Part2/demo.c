#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/printk.h>
#include <linux/uaccess.h>

#include "demo_ioctl.h"

#define MAJOR_NUM   123
#define DEVICE_NAME "hello"
#define BUFFER_SIZE 100

static int iCount;
static DEFINE_MUTEX(count_lock);

static int demo_open(struct inode *inode, struct file *file)
{
    (void)inode;
    (void)file;
    pr_info("hello: device opened\n");
    return 0;
}

static int demo_release(struct inode *inode, struct file *file)
{
    (void)inode;
    (void)file;
    pr_info("hello: device closed\n");
    return 0;
}

static ssize_t demo_write(struct file *file, const char __user *buf,
                          size_t count, loff_t *ppos)
{
    char data[BUFFER_SIZE];
    size_t n;
    int current_count;

    (void)file;
    (void)ppos;

    if (count == 0)
        return 0;

    /* Reserve one byte for the terminating NUL. A short write is legal. */
    n = min_t(size_t, count, sizeof(data) - 1);
    if (copy_from_user(data, buf, n) != 0)
        return -EFAULT;
    data[n] = '\0';

    mutex_lock(&count_lock);
    iCount++;
    current_count = iCount;
    mutex_unlock(&count_lock);

    pr_info("hello: write #%d, received %zu bytes: %s\n",
            current_count, n, data);
    return n;
}

static ssize_t demo_read(struct file *file, char __user *buf,
                         size_t count, loff_t *ppos)
{
    char text[64];
    int current_count;
    int len;

    (void)file;

    mutex_lock(&count_lock);
    current_count = iCount;
    mutex_unlock(&count_lock);

    len = scnprintf(text, sizeof(text), "iCount=%d\n", current_count);
    return simple_read_from_buffer(buf, count, ppos, text, len);
}

static long demo_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int current_count;

    (void)file;

    switch (cmd) {
    case HELLO_IOC_GET_COUNT:
        mutex_lock(&count_lock);
        current_count = iCount;
        mutex_unlock(&count_lock);
        if (copy_to_user((int __user *)arg,
                         &current_count, sizeof(current_count)) != 0)
            return -EFAULT;
        return 0;

    case HELLO_IOC_RESET_COUNT:
        mutex_lock(&count_lock);
        iCount = 0;
        mutex_unlock(&count_lock);
        pr_info("hello: write counter reset\n");
        return 0;

    default:
        return -ENOTTY;
    }
}

static const struct file_operations demo_fops = {
    .owner          = THIS_MODULE,
    .open           = demo_open,
    .release        = demo_release,
    .read           = demo_read,
    .write          = demo_write,
    .unlocked_ioctl = demo_ioctl,
};

static int __init demo_init(void)
{
    int ret;

    ret = register_chrdev(MAJOR_NUM, DEVICE_NAME, &demo_fops);
    if (ret < 0) {
        pr_err("hello: failed to register major %d (%d)\n", MAJOR_NUM, ret);
        return ret;
    }

    pr_info("hello: registered char device, major=%d\n", MAJOR_NUM);
    return 0;
}

static void __exit demo_exit(void)
{
    unregister_chrdev(MAJOR_NUM, DEVICE_NAME);
    pr_info("hello: device driver removed\n");
}

module_init(demo_init);
module_exit(demo_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Lab6 character device driver with read/write/ioctl");
