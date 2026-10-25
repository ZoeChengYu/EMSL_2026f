#include <stdio.h>      // printf, scanf
#include <fcntl.h>      // open
#include <unistd.h>     // write, close
#include <string.h>     // strlen
#include <stdlib.h>     // atoi

int gpio[6] = {466, 392, 397, 255, 296, 481};

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

// Usage: ./gpio <LED 1-4> <POWER 0|1>   (called by server.js)
int main(int argc, char** argv)
{
    int led = atoi(argv[1]);
    int power = atoi(argv[2]);
    if (led < 1 || led > 4) {
        fprintf(stderr, "LED must be 1-4\n");
        return 1;
    }
    return gpio_control(led - 1, power) == 0 ? 0 : 1;
}