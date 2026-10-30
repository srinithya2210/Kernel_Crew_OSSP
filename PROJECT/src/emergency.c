#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include "common.h"

int main(void) {
    printf("[EMERGENCY] Emergency handler started; waiting for alerts.\n"); fflush(stdout);
    while (1) {
        int fd = open(EMERGENCY_REQ_FIFO, O_RDONLY);
        if (fd < 0) { if (errno == EINTR) continue; perror("emergency request open"); return 1; }
        char message[200];
        ssize_t n = read(fd, message, sizeof(message) - 1);
        close(fd);
        if (n <= 0) continue;
        message[n] = '\0';
        message[strcspn(message, "\r\n")] = '\0';
        if (!strcmp(message, "EXIT")) break;
        int out = open(EMERGENCY_FIFO, O_WRONLY);
        if (out < 0) { perror("emergency alert FIFO"); continue; }
        (void)write(out, message, strlen(message) + 1);
        close(out);
        printf("[EMERGENCY] Alert forwarded to controller: %s\n", message); fflush(stdout);
    }
    printf("[EMERGENCY] Emergency handler stopped.\n");
    return 0;
}
