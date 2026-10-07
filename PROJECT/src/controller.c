#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#define GATE_CMD_FIFO "/tmp/airport_gate_cmd_fifo"
#define GATE_STATUS_FIFO "/tmp/airport_gate_status_fifo"
#define BAGGAGE_FIFO "/tmp/airport_baggage_fifo"
#define ANNOUNCE_FIFO "/tmp/airport_announce_fifo"
#define EMERGENCY_FIFO "/tmp/airport_emergency_fifo"

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
        ssize_t n = write(fd, (char *)buf + total, size - total);

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
        ssize_t n = read(fd, (char *)buf + total, size - total);

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

void create_fifo(const char *path) {
    if (mkfifo(path, 0666) == -1 && errno != EEXIST) {
        perror("mkfifo");
        exit(1);
    }
}

void send_fifo(const char *path, const char *message) {
    int fd = open(path, O_WRONLY);

    if (fd == -1) {
        perror("FIFO open");
        return;
    }

    write(fd, message, strlen(message) + 1);
    close(fd);
}

int get_gate_status(int status_fd, int gate_available[3]) {
    char status[100];

    int n = read(status_fd, status, sizeof(status) - 1);

    if (n <= 0)
        return -1;

    status[n] = '\0';

    gate_available[0] =
        strstr(status, "A:AVAILABLE") != NULL;

    gate_available[1] =
        strstr(status, "B:AVAILABLE") != NULL;

    gate_available[2] =
        strstr(status, "C:AVAILABLE") != NULL;

    printf("[CONTROLLER] Current gate status:\n");
    printf("[CONTROLLER] Gate A: %s\n",
           gate_available[0] ? "AVAILABLE" : "NOT AVAILABLE");
    printf("[CONTROLLER] Gate B: %s\n",
           gate_available[1] ? "AVAILABLE" : "NOT AVAILABLE");
    printf("[CONTROLLER] Gate C: %s\n",
           gate_available[2] ? "AVAILABLE" : "NOT AVAILABLE");

    return 0;
}

int main() {
    int flight_to_controller[2];
    int controller_to_flight[2];

    pid_t flight_pid;
    pid_t emergency_pid;

    create_fifo(GATE_CMD_FIFO);
    create_fifo(GATE_STATUS_FIFO);
    create_fifo(BAGGAGE_FIFO);
    create_fifo(ANNOUNCE_FIFO);
    create_fifo(EMERGENCY_FIFO);

    printf("\nAIRPORT IPC CONTROL CENTER\n");
    printf("Linux Pipes and FIFOs Simulation\n\n");

    if (pipe(flight_to_controller) == -1) {
        perror("pipe");
        return 1;
    }

    if (pipe(controller_to_flight) == -1) {
        perror("pipe");
        return 1;
    }

    printf("[IPC] Two unnamed pipes created for Flight <-> Controller\n");
    printf("[IPC] FIFOs created for Gate, Baggage, Announcement and Emergency\n\n");

    flight_pid = fork();

    if (flight_pid == -1) {
        perror("fork");
        return 1;
    }

    if (flight_pid == 0) {
        char read_fd[20];
        char write_fd[20];

        close(flight_to_controller[0]);
        close(controller_to_flight[1]);

        sprintf(read_fd, "%d", controller_to_flight[0]);
        sprintf(write_fd, "%d", flight_to_controller[1]);

        execl("./flight", "flight", read_fd, write_fd, NULL);

        perror("execl flight");
        exit(1);
    }

    close(flight_to_controller[1]);
    close(controller_to_flight[0]);

    emergency_pid = fork();

    if (emergency_pid == -1) {
        perror("fork");
        return 1;
    }

    if (emergency_pid == 0) {
        execl("./emergency", "emergency", NULL);

        perror("execl emergency");
        exit(1);
    }

    int gate_status_fd = open(GATE_STATUS_FIFO, O_RDWR);

    if (gate_status_fd == -1) {
        perror("Gate status FIFO");
        return 1;
    }

    int emergency_fd = open(EMERGENCY_FIFO, O_RDWR | O_NONBLOCK);

    if (emergency_fd == -1) {
        perror("Emergency FIFO");
        return 1;
    }

    int gate_available[3] = {1, 1, 1};

    int flights_completed = 0;
   

    while (flights_completed < 4) {

        fd_set readfds;

        FD_ZERO(&readfds);

        FD_SET(flight_to_controller[0], &readfds);
        FD_SET(emergency_fd, &readfds);

        int max_fd = flight_to_controller[0];

        if (emergency_fd > max_fd)
            max_fd = emergency_fd;

        struct timeval timeout;

        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        int result = select(max_fd + 1,
                            &readfds,
                            NULL,
                            NULL,
                            &timeout);

        if (result < 0) {
            if (errno == EINTR)
                continue;

            perror("select");
            break;
        }

        if (FD_ISSET(emergency_fd, &readfds)) {

            char message[100];

            int n = read(emergency_fd,
                         message,
                         sizeof(message) - 1);

            if (n > 0) {

                message[n] = '\0';

                printf("\n[CONTROLLER] %s\n", message);
                printf("[CONTROLLER] Activating emergency priority...\n");

                send_fifo(GATE_CMD_FIFO,
                          "EMERGENCY: Clear Gate B");

                send_fifo(ANNOUNCE_FIFO,
                          "EMERGENCY VEHICLE APPROACHING. CLEAR GATE B ROUTE.");

                printf("[CONTROLLER] Emergency instruction sent to Gate.\n");

                
            }
        }

        if (FD_ISSET(flight_to_controller[0], &readfds)) {

            FlightRequest request;

            if (read_full(flight_to_controller[0],
                          &request,
                          sizeof(request)) <= 0) {
                break;
            }

            printf("\n[CONTROLLER] Flight request received: %s at %s\n",
                   request.flight,
                   request.time);

            printf("[CONTROLLER] Requesting current gate status...\n");

            send_fifo(GATE_CMD_FIFO, "STATUS");

            if (get_gate_status(gate_status_fd,
                                gate_available) == -1) {
                printf("[CONTROLLER] Could not read gate status.\n");
                continue;
            }

            int assigned_gate = -1;

            for (int i = 0; i < 3; i++) {

                if (gate_available[i]) {
                    assigned_gate = i;
                    break;
                }
            }

            FlightResponse response;

            strcpy(response.flight, request.flight);

            if (assigned_gate == -1) {

                strcpy(response.status, "WAITLISTED");
                strcpy(response.gate, "NONE");

                printf("[CONTROLLER] No gate is available.\n");
                printf("[CONTROLLER] %s placed in waiting list.\n",
                       request.flight);

                write_full(controller_to_flight[1],
                           &response,
                           sizeof(response));
            }
            else {

                char gate_name[10];

                if (assigned_gate == 0)
                    strcpy(gate_name, "Gate A");
                else if (assigned_gate == 1)
                    strcpy(gate_name, "Gate B");
                else
                    strcpy(gate_name, "Gate C");

                strcpy(response.status, "APPROVED");
                strcpy(response.gate, gate_name);

                printf("[CONTROLLER] Choosing %s for %s.\n",
                       gate_name,
                       request.flight);

                char message[100];

                snprintf(message,
                         sizeof(message),
                         "ASSIGN %s %s",
                         request.flight,
                         gate_name);

                send_fifo(GATE_CMD_FIFO, message);

                snprintf(message,
                         sizeof(message),
                         "%s arrived at %s. Baggage handling started.",
                         request.flight,
                         gate_name);

                send_fifo(BAGGAGE_FIFO, message);

                snprintf(message,
                         sizeof(message),
                         "%s is arriving at %s.",
                         request.flight,
                         gate_name);

                send_fifo(ANNOUNCE_FIFO, message);

                write_full(controller_to_flight[1],
                           &response,
                           sizeof(response));

                flights_completed++;
            }
        }
    }

    send_fifo(GATE_CMD_FIFO, "EXIT");
    send_fifo(BAGGAGE_FIFO, "EXIT");
    send_fifo(ANNOUNCE_FIFO, "EXIT");

    close(flight_to_controller[0]);
    close(controller_to_flight[1]);
    close(gate_status_fd);
    close(emergency_fd);

    waitpid(flight_pid, NULL, 0);
    waitpid(emergency_pid, NULL, 0);

    unlink(GATE_CMD_FIFO);
    unlink(GATE_STATUS_FIFO);
    unlink(BAGGAGE_FIFO);
    unlink(ANNOUNCE_FIFO);
    unlink(EMERGENCY_FIFO);

    printf("\n[CONTROLLER] Airport IPC simulation completed.\n");

    return 0;
}
