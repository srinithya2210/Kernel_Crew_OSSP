#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>

#define ANNOUNCE_FIFO "/tmp/airport_announce_fifo"

int main() {

    mkfifo(ANNOUNCE_FIFO, 0666);

    printf("[ANNOUNCEMENT PROCESS] Started.\n");

    while (1) {

        int fd = open(ANNOUNCE_FIFO, O_RDONLY);

        if (fd == -1) {
            perror("Announcement FIFO");
            return 1;
        }

        char message[200];

        int n = read(fd,
                     message,
                     sizeof(message) - 1);

        close(fd);

        if (n <= 0)
            continue;

        message[n] = '\0';

        if (strcmp(message, "EXIT") == 0)
            break;

        printf("[ANNOUNCEMENT] %s\n", message);
    }

    printf("[ANNOUNCEMENT PROCESS] Stopped.\n");

    return 0;
}
