#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

typedef struct {
    char flight[20];
    char time[20];
} FlightRequest;

typedef struct {
    char flight[20];
    char status[30];
    char gate[10];
} FlightResponse;

ssize_t write_full(int fd, const void *buf, size_t size) {
    size_t total = 0;

    while (total < size) {
        ssize_t n = write(fd,
                          (char *)buf + total,
                          size - total);

        if (n < 0) {
            if (errno == EINTR)
                continue;

            return -1;
        }

        total += n;
    }

    return total;
}

ssize_t read_full(int fd, void *buf, size_t size) {
    size_t total = 0;

    while (total < size) {

        ssize_t n = read(fd,
                         (char *)buf + total,
                         size - total);

        if (n < 0) {
            if (errno == EINTR)
                continue;

            return -1;
        }

        if (n == 0)
            return 0;

        total += n;
    }

    return total;
}

int main(int argc, char *argv[]) {

    if (argc != 3) {
        printf("Invalid pipe arguments.\n");
        return 1;
    }

    int read_fd = atoi(argv[1]);
    int write_fd = atoi(argv[2]);

    FlightRequest flights[4] = {
        {"AI-502", "15:30"},
        {"6E-214", "15:45"},
        {"UK-831", "16:00"},
        {"SG-901", "16:15"}
    };

    printf("\nFLIGHT PROCESS STARTED\n");

    for (int i = 0; i < 4; i++) {

        while (1) {

            printf("[FLIGHT] Requesting arrival for %s at %s\n",
                   flights[i].flight,
                   flights[i].time);

            if (write_full(write_fd,
                           &flights[i],
                           sizeof(flights[i])) <= 0) {

                perror("Flight write");
                return 1;
            }

            FlightResponse response;

            if (read_full(read_fd,
                          &response,
                          sizeof(response)) <= 0) {

                perror("Flight read");
                return 1;
            }

            printf("[FLIGHT] Controller response: %s | Status: %s | Gate: %s\n",
                   response.flight,
                   response.status,
                   response.gate);

            if (strcmp(response.status, "WAITLISTED") == 0) {

                printf("[FLIGHT] Waiting for a gate...\n");

                sleep(3);
            }
            else {

                printf("[FLIGHT] %s approved for %s\n",
                       response.flight,
                       response.gate);

                break;
            }
        }

        sleep(1);
    }

    printf("\n[FLIGHT] All flight requests completed.\n");

    close(read_fd);
    close(write_fd);

    return 0;
}
