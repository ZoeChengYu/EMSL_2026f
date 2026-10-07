#include <stdio.h>      // printf, scanf
#include <fcntl.h>      // open
#include <unistd.h>     // write, close
#include <string.h>     // strlen
#include <stdlib.h>     // atoi

int gpio[6] = {466, 392, 397, 255, 296, 481};

int setup_gpio(void)
{
    for (int i = 0; i < 6; i++) {
        int fp;
        char path[128];

        // 1. Export GPIO
        fp = open("/sys/class/gpio/export", O_WRONLY);

        if (fp >= 0) {
            snprintf(path, sizeof(path), "%d", gpio[i]);
            write(fp, path, strlen(path));
            close(fp);
        } else {
            perror("Failed to open GPIO export");
            return -1;
        }

        // 等待 GPIO 建立
        usleep(100000);

        // 2. 設定成輸出
        snprintf(path, sizeof(path),
                 "/sys/class/gpio/gpio%d/direction",
                 gpio[i]);

        fp = open(path, O_WRONLY);

        if (fp >= 0) {
            write(fp, "out", 3);
            close(fp);
        } else {
            perror("Failed to set GPIO direction");
            return -1;
        }
    }

    return 0;
}


int gpio_control(int pin, int value)
{
    if (pin < 0 || pin >= 6) {
        printf("Invalid pin number. Please enter a pin between 0 and 5.\n");
        return -1;
    }

    if (value != 0 && value != 1) {
        printf("Invalid value. Please enter 0 or 1.\n");
        return -1;
    }

    int fp;
    char path[128];

    snprintf(path, sizeof(path),
             "/sys/class/gpio/gpio%d/value",
             gpio[pin]);

    fp = open(path, O_WRONLY);

    if (fp >= 0) {
        if (value == 0) {
            write(fp, "0", 1);
        } else {
            write(fp, "1", 1);
        }

        close(fp);
    } else {
        printf("Failed to open GPIO value file for pin %d\n", pin);
        return -1;
    }

    return 0;
}


int blink_mode(int time)
{
    for (int i = 0; i < time; i++) {
        gpio_control(0, 1);
        gpio_control(1, 1);
        gpio_control(2, 0);
        gpio_control(3, 0);

        usleep(500000); // 0.5 seconds

        gpio_control(0, 0);
        gpio_control(1, 0);
        gpio_control(2, 1);
        gpio_control(3, 1);

        usleep(500000); // 0.5 seconds
    }

    gpio_control(0, 0);
    gpio_control(1, 0);
    gpio_control(2, 0);
    gpio_control(3, 0);

    return 0;
}

int main(int argc, char *argv[])
{
    if (setup_gpio() != 0) {
        printf("GPIO setup failed.\n");
        return 1;
    }

    if (argc < 2) {
        printf("Usage:\n");
        printf("%s 0 <pin> <value>\n", argv[0]);
        printf("%s 1 <time>\n", argv[0]);
        return 1;
    }

    int mode = atoi(argv[1]);

    if (mode == 0) {

        if (argc != 4) {
            printf("Usage: %s 0 <pin> <value>\n", argv[0]);
            return 1;
        }

        int pin = atoi(argv[2]);
        int value = atoi(argv[3]);

        gpio_control(pin, value);
    }
    else if (mode == 1) {

        if (argc != 3) {
            printf("Usage: %s 1 <time>\n", argv[0]);
            return 1;
        }

        int time = atoi(argv[2]);

        blink_mode(time);
    }
    else {
        printf("Invalid mode. Use 0 or 1.\n");
        return 1;
    }

    return 0;
}
