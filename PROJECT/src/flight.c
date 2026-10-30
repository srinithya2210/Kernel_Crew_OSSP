
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include "common.h"

static int write_full(int fd, const void *buffer, size_t size) {
    const char *p = buffer;
    size_t done = 0;

    while (done < size) {
        ssize_t n = write(fd, p + done, size - done);

        if (n < 0 && errno == EINTR)
            continue;

        if (n <= 0)
            return -1;

        done += (size_t)n;
    }

    return 0;
}

static int read_full(int fd, void *buffer, size_t size) {
    char *p = buffer;
    size_t done = 0;

    while (done < size) {
        ssize_t n = read(fd, p + done, size - done);

        if (n < 0 && errno == EINTR)
            continue;

        if (n <= 0)
            return -1;

        done += (size_t)n;
    }

    return 0;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr,
                "Usage: flight <controller-read-fd> <controller-write-fd>\n");
        return 1;
    }

    int controller_read = atoi(argv[1]);
    int controller_write = atoi(argv[2]);

    const char *ids[] = {
        "AI-502",
        "6E-214",
        "UK-831",
        "SG-901",
        "IX-442",
        "QP-118"
    };

    int counter = 0;

    fprintf(stdout, "[FLIGHT] Automatic arrival generator started.\n");
    fflush(stdout);

    while (1) {
        sleep(15); /* One automatic arrival every 15 seconds. */

        FlightMessage request = {0};
        request.kind = 1;

        snprintf(request.flight, sizeof(request.flight), "%s",
                 ids[counter % (sizeof(ids) / sizeof(ids[0]))]);

        snprintf(request.time, sizeof(request.time), "AUTO-%03d",
                 counter + 1);

        counter++;

        printf("[FLIGHT] Automatic arrival request: %s (%s)\n",
               request.flight, request.time);
        fflush(stdout);

        if (write_full(controller_write, &request, sizeof(request)) != 0)
            break;

        FlightResponse response;

        if (read_full(controller_read, &response, sizeof(response)) != 0)
            break;

        printf("[FLIGHT] Controller response: %s | %s | %s\n",
               response.flight, response.status, response.gate);
        fflush(stdout);
    }

    close(controller_read);
    close(controller_write);

    return 0;
}


