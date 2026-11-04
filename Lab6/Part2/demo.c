
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define MAJOR_NUM 123
#define DEVICE_NAME "hello"

static int iCount = 0;

/* 開啟設備 */
static int demo_open(struct inode *inode, struct file *file)
{
    pr_info("demo: open\n");
    return 0;
}

/* 關閉設備 */
static int demo_release(struct inode *inode, struct file *file)
{
    pr_info("demo: close\n");
    return 0;
}

/* 寫入資料 */
static ssize_t demo_write(struct file *file,
                          const char __user *buf,
                          size_t count, loff_t *pos)
{
    char data[100];
    size_t n = count;

    if (n >= sizeof(data))
        n = sizeof(data) - 1;

    if (copy_from_user(data, buf, n))
        return -EFAULT;

    data[n] = '\0';

    pr_info("demo: write = %s\n", data);
    pr_info("demo: iCount = %d\n", iCount);

    iCount++;
    return n;
}

/* 讀取資料 */
static ssize_t demo_read(struct file *file,
                         char __user *buf,
                         size_t count, loff_t *pos)
{
    char data[64];
    int len;

    len = scnprintf(data, sizeof(data),
                    "iCount = %d\n", iCount);

    pr_info("demo: read\n");

    return simple_read_from_buffer(buf, count, pos,
                                   data, len);
}

/* 對應設備操作 */
static const struct file_operations demo_fops = {
    .owner   = THIS_MODULE,
    .open    = demo_open,
    .release = demo_release,
    .read    = demo_read,
    .write   = demo_write,
};

/* 載入模組 */
static int __init demo_init(void)
{
    int ret;

    ret = register_chrdev(MAJOR_NUM, DEVICE_NAME,
                          &demo_fops);

    if (ret < 0) {
        pr_err("demo: register failed\n");
        return ret;
    }

    pr_info("demo: module loaded\n");
    return 0;
}

/* 卸載模組 */
static void __exit demo_exit(void)
{
    unregister_chrdev(MAJOR_NUM, DEVICE_NAME);
    pr_info("demo: module removed\n");
}

module_init(demo_init);
module_exit(demo_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Lab6 Character Device Driver");
