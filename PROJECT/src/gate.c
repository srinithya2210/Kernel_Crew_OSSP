#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>

#define GATE_CMD_FIFO "/tmp/airport_gate_cmd_fifo"
#define GATE_STATUS_FIFO "/tmp/airport_gate_status_fifo"

void send_status(int a, int b, int c) {
    int fd = open(GATE_STATUS_FIFO, O_WRONLY);

    if (fd == -1) {
        perror("Gate status FIFO");
        return;
    }

    char status[100];

    snprintf(status, sizeof(status),
             "A:%s B:%s C:%s",
             a ? "AVAILABLE" : "NOT_AVAILABLE",
             b ? "AVAILABLE" : "NOT_AVAILABLE",
             c ? "AVAILABLE" : "NOT_AVAILABLE");

    write(fd, status, strlen(status) + 1);
    close(fd);
}

void print_status(int a, int b, int c) {
    printf("[GATE] Gate A: %s\n", a ? "AVAILABLE" : "NOT AVAILABLE");
    printf("[GATE] Gate B: %s\n", b ? "AVAILABLE" : "NOT AVAILABLE");
    printf("[GATE] Gate C: %s\n", c ? "AVAILABLE" : "NOT AVAILABLE");
}

int main() {
    mkfifo(GATE_CMD_FIFO, 0666);
    mkfifo(GATE_STATUS_FIFO, 0666);

    int gate_a = 1;
    int gate_b = 1;
    int gate_c = 1;

    printf("[GATE PROCESS] Started.\n");

    print_status(gate_a, gate_b, gate_c);

    while (1) {
        int fd = open(GATE_CMD_FIFO, O_RDONLY);

        if (fd == -1) {
            perror("Gate command FIFO");
            return 1;
        }

        char message[200];

        int n = read(fd, message, sizeof(message) - 1);

        close(fd);

        if (n <= 0)
            continue;

        message[n] = '\0';

        if (strcmp(message, "EXIT") == 0)
            break;

        if (strcmp(message, "STATUS") == 0) {
            print_status(gate_a, gate_b, gate_c);
            send_status(gate_a, gate_b, gate_c);
        }

        else if (strncmp(message, "ASSIGN", 6) == 0) {
            printf("[GATE] %s\n", message);

            if (strstr(message, "Gate A") != NULL) {
                gate_a = 0;
            }
            else if (strstr(message, "Gate B") != NULL) {
                gate_b = 0;
            }
            else if (strstr(message, "Gate C") != NULL) {
                gate_c = 0;
            }

            print_status(gate_a, gate_b, gate_c);
        }

        else if (strcmp(message, "EMERGENCY: Clear Gate B") == 0) {
            printf("[GATE] Emergency received. Clearing Gate B.\n");

            gate_b = 1;

            print_status(gate_a, gate_b, gate_c);
        }
    }

    printf("[GATE PROCESS] Stopped.\n");

    return 0;
}
