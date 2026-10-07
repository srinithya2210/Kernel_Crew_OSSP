#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>

#define EMERGENCY_FIFO "/tmp/airport_emergency_fifo"

int main() {

    mkfifo(EMERGENCY_FIFO, 0666);

    sleep(4);

    int fd = open(EMERGENCY_FIFO, O_WRONLY);

    if (fd == -1) {
        perror("Emergency FIFO");
        return 1;
    }

    char message[] = "EMERGENCY VEHICLE APPROACHING GATE B";

    write(fd,
          message,
          strlen(message) + 1);

    close(fd);

    printf("[EMERGENCY] Emergency vehicle alert sent to Controller.\n");

    return 0;
}
