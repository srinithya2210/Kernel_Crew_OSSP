#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>

#define BAGGAGE_FIFO "/tmp/airport_baggage_fifo"

int main() {

    mkfifo(BAGGAGE_FIFO, 0666);

    printf("[BAGGAGE PROCESS] Started. Waiting for controller...\n");

    while (1) {

        int fd = open(BAGGAGE_FIFO, O_RDONLY);

        if (fd == -1) {
            perror("Baggage FIFO");
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

        printf("[BAGGAGE] %s\n", message);
    }

    printf("[BAGGAGE PROCESS] Stopped.\n");

    return 0;
}
