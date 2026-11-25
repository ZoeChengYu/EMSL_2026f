/*
 * demo.c - Lab7 項目一：TX2 嵌入式平台 4 顆 LED 字元裝置驅動程式
 *
 * 透過 /dev/demo 控制 GPIO 連接的四顆 LED (LED1~LED4)。
 * 支援 write() 寫入控制命令 (如 "LED1 on", "LED1 off", "LED1")，
 * 以及 read() 讀回指定 LED 狀態。
 *
 * GPIO 對應：
 *   LED1 -> GPIO 396
 *   LED2 -> GPIO 255
 *   LED3 -> GPIO 296
 *   LED4 -> GPIO 298
 *
 * 環境：Jetson TX2 / Linux 4.9 L4T
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/delay.h>

/* GPIO sysfs path templates */
#define GPIO_EXPORT_PATH     "/sys/class/gpio/export"
#define GPIO_UNEXPORT_PATH   "/sys/class/gpio/unexport"
#define GPIO_DIR_FMT         "/sys/class/gpio/gpio%d/direction"
#define GPIO_VAL_FMT         "/sys/class/gpio/gpio%d/value"

#define DEVICE_NAME  "demo"
#define NUM_LEDS     4

/* Per-open private data: which LED the user selected for read-back */
struct demo_private {
    int selected_led;   /* 0..3, or -1 if none selected */
    int read_done;      /* flag: reply already consumed */
};

/* LED GPIO numbers as used on TX2 40-pin header */
static const int led_gpios[NUM_LEDS] = { 396, 255, 296, 298 };
static const char *led_names[NUM_LEDS] = { "LED1", "LED2", "LED3", "LED4" };

static int major;                       /* dynamic major number          */
static struct file *gpio_val_fp[NUM_LEDS];  /* open file ptrs for value  */
static int gpio_owned[NUM_LEDS];        /* 1 if we successfully exported */

/* ------------------------------------------------------------------ */
/*  Kernel-space file I/O helpers (Linux 4.9 set_fs / vfs_*)          */
/* ------------------------------------------------------------------ */

/* Write a small string to a sysfs attribute */
static int sysfs_write_str(const char *path, const char *val, size_t len)
{
    struct file *fp;
    loff_t pos = 0;
    mm_segment_t old_fs;
    ssize_t ret;

    fp = filp_open(path, O_WRONLY, 0);
    if (IS_ERR(fp))
        return PTR_ERR(fp);

    old_fs = get_fs();
    set_fs(KERNEL_DS);
    ret = vfs_write(fp, (const char __user *)val, len, &pos);
    set_fs(old_fs);

    filp_close(fp, NULL);
    return (ret == (ssize_t)len) ? 0 : (ret < 0 ? (int)ret : -EIO);
}

/* Write a single ASCII char ('0' or '1') to an already-opened value file */
static int gpio_write_value(int led_idx, int value)
{
    char ch = value ? '1' : '0';
    loff_t pos = 0;
    mm_segment_t old_fs;
    ssize_t ret;

    if (!gpio_val_fp[led_idx])
        return -ENODEV;

    vfs_llseek(gpio_val_fp[led_idx], 0, SEEK_SET);

    old_fs = get_fs();
    set_fs(KERNEL_DS);
    ret = vfs_write(gpio_val_fp[led_idx],
                    (const char __user *)&ch, 1, &pos);
    set_fs(old_fs);

    return (ret == 1) ? 0 : (ret < 0 ? (int)ret : -EIO);
}

/* Read a single ASCII char from an already-opened value file */
static int gpio_read_value(int led_idx)
{
    char ch = '0';
    loff_t pos = 0;
    mm_segment_t old_fs;
    ssize_t ret;

    if (!gpio_val_fp[led_idx])
        return -ENODEV;

    vfs_llseek(gpio_val_fp[led_idx], 0, SEEK_SET);

    old_fs = get_fs();
    set_fs(KERNEL_DS);
    ret = vfs_read(gpio_val_fp[led_idx],
                   (char __user *)&ch, 1, &pos);
    set_fs(old_fs);

    if (ret < 0)
        return (int)ret;
    if (ret != 1)
        return -EIO;
    if (ch == '0') return 0;
    if (ch == '1') return 1;
    return -EIO;
}

/* ------------------------------------------------------------------ */
/*  GPIO export / unexport / direction helpers                        */
/* ------------------------------------------------------------------ */

static int export_gpio(int gpio)
{
    char num[8];
    int len = scnprintf(num, sizeof(num), "%d", gpio);
    return sysfs_write_str(GPIO_EXPORT_PATH, num, len);
}

static void unexport_gpio(int gpio)
{
    char num[8];
    int len = scnprintf(num, sizeof(num), "%d", gpio);
    sysfs_write_str(GPIO_UNEXPORT_PATH, num, len);
}

static int set_gpio_direction(int gpio, const char *dir)
{
    char path[64];
    scnprintf(path, sizeof(path), GPIO_DIR_FMT, gpio);
    return sysfs_write_str(path, dir, strlen(dir));
}

static struct file *open_gpio_value(int gpio)
{
    char path[64];
    scnprintf(path, sizeof(path), GPIO_VAL_FMT, gpio);
    return filp_open(path, O_RDWR, 0);
}

/* ------------------------------------------------------------------ */
/*  Lookup helper: find LED index from name string (LED1..LED4)       */
/* ------------------------------------------------------------------ */

static int find_led(const char *name)
{
    int i;
    for (i = 0; i < NUM_LEDS; i++) {
        if (strncasecmp(name, led_names[i], 4) == 0)
            return i;
    }
    return -1;
}

/* ------------------------------------------------------------------ */
/*  file_operations callbacks                                         */
/* ------------------------------------------------------------------ */

static int drv_open(struct inode *inode, struct file *file)
{
    struct demo_private *priv;

    pr_info("Enter Open function\n");

    priv = kzalloc(sizeof(*priv), GFP_KERNEL);
    if (!priv)
        return -ENOMEM;
    priv->selected_led = -1;
    priv->read_done = 0;
    file->private_data = priv;
    return 0;
}

static int drv_release(struct inode *inode, struct file *file)
{
    pr_info("Enter Release function\n");
    kfree(file->private_data);
    return 0;
}

/*
 * drv_write:
 *   "LED1 on"   -> turn on LED1
 *   "LED1 off"  -> turn off LED1
 *   "LED1"      -> select LED1 for subsequent read (query mode)
 */
static ssize_t drv_write(struct file *file, const char __user *buf,
                         size_t count, loff_t *pos)
{
    struct demo_private *priv = file->private_data;
    char command[32];
    char led_str[8];
    char action[8];
    int idx, ret, n_parsed;

    pr_info("Enter Write function\n");

    if (!count)
        return 0;
    if (count >= sizeof(command))
        return -EINVAL;
    if (copy_from_user(command, buf, count))
        return -EFAULT;
    command[count] = '\0';

    /* Try to parse "LEDx on/off" or just "LEDx" */
    n_parsed = sscanf(command, "%7s %7s", led_str, action);
    if (n_parsed < 1)
        return -EINVAL;

    idx = find_led(led_str);
    if (idx < 0)
        return -EINVAL;

    if (n_parsed == 2) {
        /* Control mode: on or off */
        int value;
        if (strncasecmp(action, "on", 2) == 0)
            value = 1;
        else if (strncasecmp(action, "off", 3) == 0)
            value = 0;
        else
            return -EINVAL;

        ret = gpio_write_value(idx, value);
        if (ret)
            return ret;
    }

    /* Select this LED for subsequent read-back */
    priv->selected_led = idx;
    priv->read_done = 0;

    return count;
}

/*
 * drv_read:
 *   Returns "LEDx Status: 0\n" or "LEDx Status: 1\n"
 *   for the LED selected by a previous write.
 */
static ssize_t drv_read(struct file *file, char __user *buf,
                        size_t count, loff_t *pos)
{
    struct demo_private *priv = file->private_data;
    char reply[32];
    int len, val, idx;

    pr_info("Enter Read function\n");

    idx = priv->selected_led;
    if (idx < 0 || idx >= NUM_LEDS)
        return -EINVAL;

    if (priv->read_done)
        return 0;

    val = gpio_read_value(idx);
    if (val < 0)
        return val;

    len = scnprintf(reply, sizeof(reply),
                    "%s Status: %d\n", led_names[idx], val);

    if (count < (size_t)len)
        len = count;

    if (copy_to_user(buf, reply, len))
        return -EFAULT;

    priv->read_done = 1;
    return len;
}

static long drv_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    pr_info("Enter I/O Control function\n");
    return cmd == 0 ? 0 : -ENOTTY;
}

static const struct file_operations drv_fops = {
    .owner          = THIS_MODULE,
    .open           = drv_open,
    .read           = drv_read,
    .write          = drv_write,
    .unlocked_ioctl = drv_ioctl,
    .release        = drv_release,
};

/* ------------------------------------------------------------------ */
/*  Module init / exit                                                */
/* ------------------------------------------------------------------ */

static int __init demo_init(void)
{
    int i, ret;

    memset(gpio_val_fp, 0, sizeof(gpio_val_fp));
    memset(gpio_owned, 0, sizeof(gpio_owned));

    /* Export GPIOs and set as output (default low = off) */
    for (i = 0; i < NUM_LEDS; i++) {
        ret = export_gpio(led_gpios[i]);
        if (ret && ret != -EBUSY) {
            pr_err("demo: cannot export GPIO %d (err=%d)\n",
                   led_gpios[i], ret);
            goto release_owned_gpio;
        }
        gpio_owned[i] = 1;
        msleep(20); /* wait for sysfs nodes to settle */

        ret = set_gpio_direction(led_gpios[i], "low");
        if (ret) {
            pr_err("demo: cannot set direction for GPIO %d\n",
                   led_gpios[i]);
            goto release_owned_gpio;
        }

        gpio_val_fp[i] = open_gpio_value(led_gpios[i]);
        if (IS_ERR(gpio_val_fp[i])) {
            ret = PTR_ERR(gpio_val_fp[i]);
            gpio_val_fp[i] = NULL;
            pr_err("demo: cannot open value for GPIO %d\n",
                   led_gpios[i]);
            goto release_owned_gpio;
        }
    }

    /* Register char device */
    major = register_chrdev(0, DEVICE_NAME, &drv_fops);
    if (major < 0) {
        ret = major;
        pr_err("demo: register_chrdev failed (%d)\n", ret);
        goto release_owned_gpio;
    }

    pr_info("demo: module loaded, major=%d\n", major);
    return 0;

release_owned_gpio:
    for (i = 0; i < NUM_LEDS; i++) {
        if (gpio_val_fp[i]) {
            filp_close(gpio_val_fp[i], NULL);
            gpio_val_fp[i] = NULL;
        }
        if (gpio_owned[i]) {
            unexport_gpio(led_gpios[i]);
            gpio_owned[i] = 0;
        }
    }
    return ret;
}

static void __exit demo_exit(void)
{
    int i;

    unregister_chrdev(major, DEVICE_NAME);

    for (i = 0; i < NUM_LEDS; i++) {
        /* Turn off LED before releasing */
        if (gpio_val_fp[i]) {
            gpio_write_value(i, 0);
            filp_close(gpio_val_fp[i], NULL);
            gpio_val_fp[i] = NULL;
        }
        if (gpio_owned[i]) {
            unexport_gpio(led_gpios[i]);
            gpio_owned[i] = 0;
        }
    }

    pr_info("demo: module removed\n");
}

module_init(demo_init);
module_exit(demo_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Lab7 LED Character Device Driver for Jetson TX2");
