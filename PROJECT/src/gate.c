#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include "common.h"

static void send_status(int a, int b, int c) {
    int fd = open(GATE_STATUS_FIFO, O_WRONLY | O_NONBLOCK);
    if (fd < 0) return;
    char status[160];
    snprintf(status, sizeof(status), "A:%s B:%s C:%s\n",
             a ? "AVAILABLE" : "OCCUPIED",
             b ? "AVAILABLE" : "OCCUPIED",
             c ? "AVAILABLE" : "OCCUPIED");
    (void)write(fd, status, strlen(status));
    close(fd);
}

int main(void) {
    int a = 1, b = 1, c = 1;
    int fd;
    printf("[GATE] Gate process started.\n"); fflush(stdout);
    while (1) {
        fd = open(GATE_CMD_FIFO, O_RDONLY);
        if (fd < 0) { if (errno == EINTR) continue; perror("gate open"); return 1; }
        char message[200];
        ssize_t n = read(fd, message, sizeof(message) - 1);
        close(fd);
        if (n <= 0) continue;
        message[n] = '\0';
        message[strcspn(message, "\r\n")] = '\0';
        if (!strcmp(message, "EXIT")) break;
        if (!strcmp(message, "STATUS")) {
            printf("[GATE] Status requested. A=%s B=%s C=%s\n", a ? "AVAILABLE" : "OCCUPIED", b ? "AVAILABLE" : "OCCUPIED", c ? "AVAILABLE" : "OCCUPIED");
            send_status(a, b, c);
        } else if (!strncmp(message, "ASSIGN|", 7)) {
            char flight[FLIGHT_ID_LEN], gate[GATE_LEN];
            if (sscanf(message + 7, "%31[^|]|%15s", flight, gate) == 2) {
                if (!strcmp(gate, "A")) a = 0;
                if (!strcmp(gate, "B")) b = 0;
                if (!strcmp(gate, "C")) c = 0;
                printf("[GATE] Assigned %s to Gate %s.\n", flight, gate);
                send_status(a, b, c);
            }
        } else if (!strncmp(message, "RELEASE|", 8)) {
            char gate[GATE_LEN] = {0};
            sscanf(message + 8, "%15s", gate);
            if (!strcmp(gate, "A")) a = 1;
            if (!strcmp(gate, "B")) b = 1;
            if (!strcmp(gate, "C")) c = 1;
            printf("[GATE] Gate %s released.\n", gate);
            send_status(a, b, c);
        } else if (!strcmp(message, "EMERGENCY|CLEAR_B")) {
            /* Controller only sends this when B is not assigned to a flight. */
            if (b) printf("[GATE] Emergency priority: Gate B route is clear.\n");
            else printf("[GATE] Emergency received; Gate B is occupied, controller must not force-release it.\n");
        }
        fflush(stdout);
    }
    printf("[GATE] Gate process stopped.\n");
    return 0;
}
