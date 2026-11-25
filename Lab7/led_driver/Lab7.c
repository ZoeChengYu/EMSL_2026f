/*
 * Lab7.c - Lab7 項目一：使用者空間 LED 控制程式
 *
 * 用法：
 *   ./Lab7 LED1 on     -> 點亮 LED1
 *   ./Lab7 LED1 off    -> 熄滅 LED1
 *   ./Lab7 LED1        -> 查詢 LED1 狀態
 *
 * 透過 /dev/demo 字元裝置與核心模組通訊。
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>

#define DEVICE_PATH "/dev/demo"

static void usage(const char *prog)
{
    fprintf(stderr,
            "Usage:\n"
            "  %s LEDn on    - Turn on LEDn  (n=1..4)\n"
            "  %s LEDn off   - Turn off LEDn (n=1..4)\n"
            "  %s LEDn       - Query LEDn status\n",
            prog, prog, prog);
}

int main(int argc, char *argv[])
{
    int fd;
    char command[32];
    char reply[64];
    ssize_t n;

    if (argc < 2 || argc > 3) {
        usage(argv[0]);
        return 1;
    }

    /* Validate LED name: LED1..LED4 */
    if (strlen(argv[1]) != 4 ||
        strncasecmp(argv[1], "LED", 3) != 0 ||
        argv[1][3] < '1' || argv[1][3] > '4') {
        fprintf(stderr, "Error: Invalid LED name '%s'. "
                "Use LED1, LED2, LED3, or LED4.\n", argv[1]);
        return 1;
    }

    /* Validate action if given */
    if (argc == 3) {
        if (strcasecmp(argv[2], "on") != 0 &&
            strcasecmp(argv[2], "off") != 0) {
            fprintf(stderr, "Error: Invalid action '%s'. "
                    "Use 'on' or 'off'.\n", argv[2]);
            return 1;
        }
    }

    /* Build command string */
    if (argc == 3)
        snprintf(command, sizeof(command), "%s %s", argv[1], argv[2]);
    else
        snprintf(command, sizeof(command), "%s", argv[1]);

    /* Open device */
    fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("open " DEVICE_PATH);
        return 1;
    }

    /* Trigger ioctl to verify I/O Control function log */
    if (ioctl(fd, 0, 0) < 0) {
        perror("ioctl");
        /* Non-fatal: continue */
    }

    /* Write command */
    n = strlen(command);
    if (write(fd, command, n) != n) {
        perror("write");
        close(fd);
        return 1;
    }

    /* Read back status */
    n = read(fd, reply, sizeof(reply) - 1);
    if (n < 0) {
        perror("read");
        close(fd);
        return 1;
    }
    reply[n] = '\0';

    /* Print result to stdout (e.g. "LED1 Status: 1") */
    printf("%s", reply);

    close(fd);
    return 0;
}
