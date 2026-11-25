/*
 * spi_demo.c - Lab7 項目三：GPIO 軟體 SPI 實作 MCP3008 ADC 字元驅動
 *
 * 透過 GPIO 逐位操作實作軟體 SPI，讀取 MCP3008 光敏電阻 ADC 數值。
 * 註冊 /dev/spi_demo 字元裝置，提供 open(), read(), release()。
 *
 * GPIO 接線 (TX2 J21 header):
 *   CLK  -> GPIO 427 (pin 23)   -> MCP3008 CLK  (pin 13)
 *   MISO -> GPIO 428 (pin 21)   -> MCP3008 DOUT (pin 12)
 *   MOSI -> GPIO 429 (pin 19)   -> MCP3008 DIN  (pin 11)
 *   CS   -> GPIO 430 (pin 24)   -> MCP3008 CS   (pin 10)
 *
 * 載入範例：
 *   sudo insmod ./spi_demo.ko channel=0
 *
 * 環境：Jetson TX2 / Linux 4.9 L4T
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/ratelimit.h>

#define DEVICE_NAME  "spi_demo"

/* GPIO indices */
#define CLK   0
#define MISO  1
#define MOSI  2
#define CS    3
#define NUM_PINS 4

/* GPIO sysfs paths */
#define GPIO_EXPORT_PATH     "/sys/class/gpio/export"
#define GPIO_UNEXPORT_PATH   "/sys/class/gpio/unexport"
#define GPIO_DIR_FMT         "/sys/class/gpio/gpio%d/direction"
#define GPIO_VAL_FMT         "/sys/class/gpio/gpio%d/value"
#define GPIO_ACTIVE_LOW_FMT  "/sys/class/gpio/gpio%d/active_low"

/* SPI timing */
#define HALF_US 2   /* udelay between edges */

/* ADC timing guard */
#define MAX_FRAME_US 1000  /* max time for one SPI frame */
#define MAX_RETRIES  3

/* Module parameter */
static int channel = 0;
module_param(channel, int, 0444);
MODULE_PARM_DESC(channel, "MCP3008 ADC channel (0-7, default 0)");

/* Pin configuration */
static const int pin_gpios[NUM_PINS] = { 427, 428, 429, 430 };
static const char *pin_names[NUM_PINS] = { "CLK", "MISO", "MOSI", "CS" };

static int major;
static struct file *pin_value_fp[NUM_PINS];
static int pin_owned[NUM_PINS];
static DEFINE_MUTEX(spi_mutex);

/* Per-open data: reply buffer and read offset */
struct spi_private {
    char reply[16];
    int length;
    int offset;
};

/* ------------------------------------------------------------------ */
/*  Kernel-space file I/O helpers (Linux 4.9)                         */
/* ------------------------------------------------------------------ */

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

/* file_transfer: read or write one char to an already-opened file */
static ssize_t file_transfer(struct file *fp, char *ch, size_t len,
                             bool write_mode)
{
    loff_t pos = 0;
    mm_segment_t old_fs;
    ssize_t ret;

    if (!fp)
        return -ENODEV;

    vfs_llseek(fp, 0, SEEK_SET);

    old_fs = get_fs();
    set_fs(KERNEL_DS);
    if (write_mode)
        ret = vfs_write(fp, (const char __user *)ch, len, &pos);
    else
        ret = vfs_read(fp, (char __user *)ch, len, &pos);
    set_fs(old_fs);

    return ret;
}

/* Write a GPIO pin value (0 or 1) */
static int pin_write(int index, int value)
{
    char ch = value ? '1' : '0';
    ssize_t ret = file_transfer(pin_value_fp[index], &ch, 1, true);
    return ret < 0 ? (int)ret : (ret == 1 ? 0 : -EIO);
}

/* Read a GPIO pin value, returns 0 or 1, or negative error */
static int pin_read(int index)
{
    char ch;
    ssize_t ret = file_transfer(pin_value_fp[index], &ch, 1, false);
    if (ret < 0) return (int)ret;
    if (ret != 1 || (ch != '0' && ch != '1'))
        return -EIO;
    return ch - '0';
}

/* ------------------------------------------------------------------ */
/*  GPIO export / direction helpers                                   */
/* ------------------------------------------------------------------ */

static int export_gpio(int gpio_num)
{
    char num[8];
    int len = scnprintf(num, sizeof(num), "%d", gpio_num);
    return sysfs_write_str(GPIO_EXPORT_PATH, num, len);
}

static void unexport_gpio(int gpio_num)
{
    char num[8];
    int len = scnprintf(num, sizeof(num), "%d", gpio_num);
    sysfs_write_str(GPIO_UNEXPORT_PATH, num, len);
}

static int set_gpio_direction(int gpio_num, const char *dir)
{
    char path[64];
    scnprintf(path, sizeof(path), GPIO_DIR_FMT, gpio_num);
    return sysfs_write_str(path, dir, strlen(dir));
}

static int set_active_low(int gpio_num, int val)
{
    char path[64];
    char ch = val ? '1' : '0';
    scnprintf(path, sizeof(path), GPIO_ACTIVE_LOW_FMT, gpio_num);
    return sysfs_write_str(path, &ch, 1);
}

static struct file *open_gpio_value(int gpio_num)
{
    char path[64];
    scnprintf(path, sizeof(path), GPIO_VAL_FMT, gpio_num);
    return filp_open(path, O_RDWR, 0);
}

/* ------------------------------------------------------------------ */
/*  Software SPI: readadc                                             */
/* ------------------------------------------------------------------ */

static int readadc(int adcnum, unsigned int *value)
{
    int i, ret, bit;
    unsigned int commandout;
    unsigned int adcout = 0;
    ktime_t start;

    if (adcnum < 0 || adcnum > 7)
        return -EINVAL;

    start = ktime_get();

    /* CS low: start transaction */
    ret = pin_write(CS, 0);
    if (ret) return ret;

    /* Send 5 command bits: Start(1), SGL(1), D2, D1, D0 */
    commandout = (adcnum | 0x18) << 3;
    for (i = 0; i < 5; i++) {
        ret = pin_write(MOSI, !!(commandout & 0x80));
        if (ret) goto cs_high;
        commandout <<= 1;

        udelay(HALF_US);
        ret = pin_write(CLK, 1);
        if (ret) goto cs_high;
        udelay(HALF_US);
        ret = pin_write(CLK, 0);
        if (ret) goto cs_high;
    }

    /* Receive 12 clock cycles: null bit + 10-bit ADC + 1 extra */
    for (i = 0; i < 12; i++) {
        udelay(HALF_US);
        ret = pin_write(CLK, 1);
        if (ret) goto cs_high;
        udelay(HALF_US);

        bit = pin_read(MISO);
        if (bit < 0) {
            ret = bit;
            goto cs_high;
        }

        ret = pin_write(CLK, 0);
        if (ret) goto cs_high;

        adcout = (adcout << 1) | bit;
    }

    /* CS high: end transaction */
    ret = pin_write(CS, 1);
    if (ret) return ret;

    /* Check timing: frame too slow? */
    {
        s64 elapsed_us = ktime_us_delta(ktime_get(), start);
        if (elapsed_us > MAX_FRAME_US) {
            pr_warn_ratelimited("spi_demo: frame took %lld us "
                                "(limit %d)\n",
                                elapsed_us, MAX_FRAME_US);
            return -EAGAIN;
        }
    }

    *value = adcout & 0x3ff;  /* 10-bit result */
    return 0;

cs_high:
    pin_write(CS, 1);
    return ret;
}

/* ------------------------------------------------------------------ */
/*  file_operations                                                   */
/* ------------------------------------------------------------------ */

static int demo_open(struct inode *inode, struct file *file)
{
    struct spi_private *ctx;

    pr_info("spi_demo: Enter Open function\n");

    ctx = kzalloc(sizeof(*ctx), GFP_KERNEL);
    if (!ctx)
        return -ENOMEM;
    ctx->length = 0;
    ctx->offset = 0;
    file->private_data = ctx;
    return 0;
}

static int demo_release(struct inode *inode, struct file *file)
{
    pr_info("spi_demo: Enter Release function\n");
    kfree(file->private_data);
    return 0;
}

static ssize_t demo_read(struct file *file, char __user *buf,
                         size_t count, loff_t *pos)
{
    struct spi_private *ctx = file->private_data;
    int ret;
    size_t bytes;

    pr_info("spi_demo: Enter Read function\n");

    mutex_lock(&spi_mutex);

    /* If the previous snapshot has been fully read, do a new ADC read */
    if (ctx->offset >= ctx->length) {
        unsigned int value;
        int retries;

        for (retries = 0; retries < MAX_RETRIES; retries++) {
            ret = readadc(channel, &value);
            if (ret != -EAGAIN)
                break;
        }
        if (ret) {
            mutex_unlock(&spi_mutex);
            return ret;
        }

        ctx->length = scnprintf(ctx->reply, sizeof(ctx->reply),
                                "%u\n", value);
        ctx->offset = 0;
    }

    mutex_unlock(&spi_mutex);

    /* Copy remaining bytes to user */
    bytes = ctx->length - ctx->offset;
    if (bytes > count)
        bytes = count;
    if (bytes == 0)
        return 0;

    if (copy_to_user(buf, ctx->reply + ctx->offset, bytes))
        return -EFAULT;

    ctx->offset += bytes;
    return bytes;
}

static const struct file_operations demo_fops = {
    .owner   = THIS_MODULE,
    .open    = demo_open,
    .read    = demo_read,
    .release = demo_release,
};

/* ------------------------------------------------------------------ */
/*  Module init / exit                                                */
/* ------------------------------------------------------------------ */

static int __init spi_demo_init(void)
{
    int i, ret;
    /*
     * Pin directions:
     *   CLK  = output low (idle)
     *   MISO = input
     *   MOSI = output low
     *   CS   = output high (deselected)
     */
    static const char *pin_dirs[NUM_PINS] = {
        "low",   /* CLK  */
        "in",    /* MISO */
        "low",   /* MOSI */
        "high"   /* CS   */
    };

    if (channel < 0 || channel > 7) {
        pr_err("spi_demo: channel must be 0-7\n");
        return -EINVAL;
    }

    memset(pin_value_fp, 0, sizeof(pin_value_fp));
    memset(pin_owned, 0, sizeof(pin_owned));

    for (i = 0; i < NUM_PINS; i++) {
        ret = export_gpio(pin_gpios[i]);
        if (ret && ret != -EBUSY) {
            pr_err("spi_demo: cannot export GPIO %d (%s): %d\n",
                   pin_gpios[i], pin_names[i], ret);
            goto cleanup;
        }
        pin_owned[i] = 1;
        msleep(20); /* wait for sysfs nodes to settle */

        ret = set_active_low(pin_gpios[i], 0);
        if (ret) {
            pr_err("spi_demo: cannot set active_low for GPIO %d\n",
                   pin_gpios[i]);
            goto cleanup;
        }

        ret = set_gpio_direction(pin_gpios[i], pin_dirs[i]);
        if (ret) {
            pr_err("spi_demo: cannot set direction for GPIO %d\n",
                   pin_gpios[i]);
            goto cleanup;
        }

        pin_value_fp[i] = open_gpio_value(pin_gpios[i]);
        if (IS_ERR(pin_value_fp[i])) {
            ret = PTR_ERR(pin_value_fp[i]);
            pin_value_fp[i] = NULL;
            pr_err("spi_demo: cannot open value for GPIO %d\n",
                   pin_gpios[i]);
            goto cleanup;
        }
    }

    major = register_chrdev(0, DEVICE_NAME, &demo_fops);
    if (major < 0) {
        ret = major;
        pr_err("spi_demo: register_chrdev failed (%d)\n", ret);
        goto cleanup;
    }

    pr_info("spi_demo: loaded (major=%d, channel=%d)\n", major, channel);
    return 0;

cleanup:
    for (i = 0; i < NUM_PINS; i++) {
        if (pin_value_fp[i]) {
            filp_close(pin_value_fp[i], NULL);
            pin_value_fp[i] = NULL;
        }
        if (pin_owned[i]) {
            unexport_gpio(pin_gpios[i]);
            pin_owned[i] = 0;
        }
    }
    return ret;
}

static void __exit spi_demo_exit(void)
{
    int i;

    unregister_chrdev(major, DEVICE_NAME);

    for (i = 0; i < NUM_PINS; i++) {
        if (pin_value_fp[i]) {
            filp_close(pin_value_fp[i], NULL);
            pin_value_fp[i] = NULL;
        }
        if (pin_owned[i]) {
            unexport_gpio(pin_gpios[i]);
            pin_owned[i] = 0;
        }
    }

    pr_info("spi_demo: unloaded\n");
}

module_init(spi_demo_init);
module_exit(spi_demo_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Lab7 Software SPI ADC (MCP3008) Driver for Jetson TX2");
