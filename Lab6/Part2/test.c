
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main(void)
{
    int fd;
    char msg[] = "Hello Kernel";
    char buf[100];
    ssize_t n;

    /* 開啟設備 */
    fd = open("/dev/hello", O_RDWR);

    if (fd < 0) {
        perror("open");
        return 1;
    }

    /* 寫入資料 */
    n = write(fd, msg, strlen(msg));

    if (n < 0) {
        perror("write");
        close(fd);
        return 1;
    }

    printf("Write: %s\n", msg);

    /* 讀取資料 */
    n = read(fd, buf, sizeof(buf) - 1);

    if (n < 0) {
        perror("read");
        close(fd);
        return 1;
    }

    buf[n] = '\0';
    printf("Read: %s", buf);

    /* 關閉設備 */
    close(fd);

    return 0;
}
