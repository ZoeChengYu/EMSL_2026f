/*
 * button_irq.c - Lab7 項目二：按鍵中斷驅動模組
 *
 * 將按鍵開關連接至 TX2 的 GPIO（預設 GPIO 481, J21 pin18）。
 * 設為輸入模式並綁定硬體中斷服務常式 (ISR)。
 * 按鍵按下時透過 dmesg 印出指定提示。
 *
 * 載入範例：
 *   sudo insmod ./button_irq.ko gpio=481 active_low=1 debounce_ms=30
 *
 * 環境：Jetson TX2 / Linux 4.9 L4T
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/jiffies.h>

/* Module parameters */
static int gpio = 481;
module_param(gpio, int, 0444);
MODULE_PARM_DESC(gpio, "GPIO number for the button (default 481)");

static int active_low = 1;
module_param(active_low, int, 0444);
MODULE_PARM_DESC(active_low, "1 = press is low (falling edge), "
                 "0 = press is high (rising edge)");

static int debounce_ms = 30;
module_param(debounce_ms, int, 0444);
MODULE_PARM_DESC(debounce_ms, "Debounce time window in ms (default 30)");

/* Per-device state */
struct button_device {
    int gpio;
    int irq;
    unsigned long press_count;
    bool have_last_press;
    unsigned long last_press;
    unsigned long debounce_ticks;
};

static struct button_device button;

/* ------------------------------------------------------------------ */
/*  ISR: Interrupt Service Routine                                    */
/* ------------------------------------------------------------------ */

static irqreturn_t button_isr(int irq, void *dev_id)
{
    struct button_device *b = dev_id;
    unsigned long now = jiffies;

    /* Debounce: ignore events within the time window */
    if (b->debounce_ticks && b->have_last_press &&
        time_before(now, b->last_press + b->debounce_ticks))
        return IRQ_HANDLED;

    /* Accept this event */
    b->last_press = now;
    b->have_last_press = true;
    b->press_count++;

    pr_info("Button Pressed! Interrupt Triggered. count=%lu\n",
            b->press_count);

    return IRQ_HANDLED;
}

/* ------------------------------------------------------------------ */
/*  Module init / exit                                                */
/* ------------------------------------------------------------------ */

static int __init button_irq_init(void)
{
    int ret;
    unsigned long flags;

    button.gpio = gpio;
    button.press_count = 0;
    button.have_last_press = false;
    button.debounce_ticks = msecs_to_jiffies(debounce_ms);

    /* Request and configure GPIO */
    ret = gpio_request(button.gpio, "lab_button_irq");
    if (ret) {
        pr_err("button_irq: gpio_request(%d) failed: %d\n",
               button.gpio, ret);
        return ret;
    }

    ret = gpio_direction_input(button.gpio);
    if (ret) {
        pr_err("button_irq: gpio_direction_input(%d) failed: %d\n",
               button.gpio, ret);
        goto free_gpio;
    }

    /* Map GPIO to IRQ */
    ret = gpio_to_irq(button.gpio);
    if (ret < 0) {
        pr_err("button_irq: gpio_to_irq(%d) failed: %d\n",
               button.gpio, ret);
        goto free_gpio;
    }
    button.irq = ret;

    /* Register ISR with appropriate trigger edge */
    flags = active_low ? IRQF_TRIGGER_FALLING : IRQF_TRIGGER_RISING;
    ret = request_irq(button.irq, button_isr, flags,
                      "lab_button_irq", &button);
    if (ret) {
        pr_err("button_irq: request_irq(%d) failed: %d\n",
               button.irq, ret);
        goto free_gpio;
    }

    pr_info("button_irq: loaded (gpio=%d, irq=%d, active_low=%d, "
            "debounce=%dms)\n",
            button.gpio, button.irq, active_low, debounce_ms);
    return 0;

free_gpio:
    gpio_free(button.gpio);
    return ret;
}

static void __exit button_irq_exit(void)
{
    free_irq(button.irq, &button);
    gpio_free(button.gpio);

    pr_info("button_irq: unloaded (total presses=%lu)\n",
            button.press_count);
}

module_init(button_irq_init);
module_exit(button_irq_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Lab7 Button Interrupt Driver for Jetson TX2");
