#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include "common.h"

int main(void) {
    printf("[ANNOUNCEMENT] Announcement process started.\n"); fflush(stdout);
    while (1) {
        int fd = open(ANNOUNCE_FIFO, O_RDONLY);
        if (fd < 0) { if (errno == EINTR) continue; perror("announcement open"); return 1; }
        char message[256];
        ssize_t n = read(fd, message, sizeof(message) - 1);
        close(fd);
        if (n <= 0) continue;
        message[n] = '\0';
        message[strcspn(message, "\r\n")] = '\0';
        if (!strcmp(message, "EXIT")) break;
        printf("[ANNOUNCEMENT] %s\n", message); fflush(stdout);
    }
    printf("[ANNOUNCEMENT] Announcement process stopped.\n");
    return 0;
}
