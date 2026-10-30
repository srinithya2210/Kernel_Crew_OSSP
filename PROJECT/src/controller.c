#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include "common.h"

#define MAX_QUEUE 128
#define SERVICE_SECONDS 25

typedef struct {
    char flight[FLIGHT_ID_LEN];
    char time[FLIGHT_TIME_LEN];
    int queued;
} FlightEntry;

typedef struct {
    int occupied;
    char flight[FLIGHT_ID_LEN];
    time_t release_at;
} GateState;

static GateState gates[3];
static const char *gate_names[] = {"A", "B", "C"};
static FlightEntry waiting[MAX_QUEUE];
static int waiting_count = 0;
static int next_id = 1;
static volatile sig_atomic_t stop_requested = 0;

static void handle_signal(int sig) { (void)sig; stop_requested = 1; }

static void create_fifo(const char *path) {
    if (mkfifo(path, 0666) == -1 && errno != EEXIST) { perror(path); exit(1); }
}

static void send_fifo(const char *path, const char *message) {
    int fd = open(path, O_WRONLY | O_NONBLOCK);
    if (fd < 0) {
        /* These are local IPC workers; retry briefly if a reader is starting. */
        for (int i = 0; i < 20 && fd < 0; i++) {
            struct timespec pause = {0, 50000000};
            nanosleep(&pause, NULL);
            fd = open(path, O_WRONLY | O_NONBLOCK);
        }
    }
    if (fd < 0) { fprintf(stderr, "[CONTROLLER] Could not write FIFO %s: %s\n", path, strerror(errno)); return; }
    (void)write(fd, message, strlen(message) + 1);
    close(fd);
}

static void json_escape(const char *source, char *target, size_t capacity) {
    size_t used = 0;
    if (!source) source = "";
    for (size_t i = 0; source[i] && used + 2 < capacity; i++) {
        unsigned char ch = (unsigned char)source[i];
        if (ch == '\"' || ch == '\\') {
            target[used++] = '\\';
            target[used++] = (char)ch;
        } else if (ch == '\n' || ch == '\r' || ch == '\t') {
            target[used++] = ' ';
        } else if (ch >= 32) {
            target[used++] = (char)ch;
        }
    }
    target[used] = '\0';
}

static void emit_event(const char *type, const char *flight, const char *status,
                       const char *gate, const char *message) {
    char etype[64], eflight[96], estatus[64], egate[48], emessage[384];
    char json[1024];
    json_escape(type, etype, sizeof(etype));
    json_escape(flight, eflight, sizeof(eflight));
    json_escape(status, estatus, sizeof(estatus));
    json_escape(gate, egate, sizeof(egate));
    json_escape(message, emessage, sizeof(emessage));
    snprintf(json, sizeof(json),
        "{\"type\":\"%s\",\"flight\":\"%s\",\"status\":\"%s\",\"gate\":\"%s\",\"message\":\"%s\",\"timestamp\":%lld}\n",
        etype, eflight, estatus, egate, emessage, (long long)time(NULL));
    printf("[EVENT] %s", json); fflush(stdout);
    int fd = open(EVENTS_FIFO, O_WRONLY | O_NONBLOCK);
    if (fd >= 0) { (void)write(fd, json, strlen(json)); close(fd); }
}

static int available_gate(void) {
    for (int i = 0; i < 3; i++) if (!gates[i].occupied) return i;
    return -1;
}

static void send_flight_response(int fd, const char *flight, const char *status, const char *gate) {
    FlightResponse response = {0};
    snprintf(response.flight, sizeof(response.flight), "%s", flight);
    snprintf(response.status, sizeof(response.status), "%s", status);
    snprintf(response.gate, sizeof(response.gate), "%s", gate);
    (void)write(fd, &response, sizeof(response));
}

static void queue_flight(const char *flight, const char *time_label) {
    if (waiting_count >= MAX_QUEUE) {
        emit_event("flight", flight, "REJECTED", "NONE", "Waiting queue is full");
        return;
    }
    snprintf(waiting[waiting_count].flight, sizeof(waiting[waiting_count].flight), "%s", flight);
    snprintf(waiting[waiting_count].time, sizeof(waiting[waiting_count].time), "%s", time_label);
    waiting[waiting_count].queued = 1;
    waiting_count++;
    printf("[CONTROLLER] %s added to waiting queue. Queue size: %d\n", flight, waiting_count);
    emit_event("flight", flight, "WAITLISTED", "NONE", "No gate available; added to waiting queue");
}

static void assign_flight(const char *flight, const char *time_label, int response_fd) {
    int gate = available_gate();
    if (gate < 0) {
        if (response_fd >= 0) send_flight_response(response_fd, flight, "WAITLISTED", "NONE");
        queue_flight(flight, time_label);
        return;
    }
    gates[gate].occupied = 1;
    snprintf(gates[gate].flight, sizeof(gates[gate].flight), "%s", flight);
    gates[gate].release_at = time(NULL) + SERVICE_SECONDS;

    char command[160];
    snprintf(command, sizeof(command), "ASSIGN|%s|%s", flight, gate_names[gate]);
    send_fifo(GATE_CMD_FIFO, command);
    snprintf(command, sizeof(command), "%s arrived at Gate %s; baggage handling started", flight, gate_names[gate]);
    send_fifo(BAGGAGE_FIFO, command);
    snprintf(command, sizeof(command), "%s is arriving at Gate %s", flight, gate_names[gate]);
    send_fifo(ANNOUNCE_FIFO, command);

    if (response_fd >= 0) send_flight_response(response_fd, flight, "APPROVED", gate_names[gate]);
    snprintf(command, sizeof(command), "Gate %s assigned; simulated service lasts %d seconds", gate_names[gate], SERVICE_SECONDS);
    emit_event("flight", flight, "APPROVED", gate_names[gate], command);
    emit_event("gate", flight, "OCCUPIED", gate_names[gate], "Gate assigned to flight");
    emit_event("baggage", flight, "PROCESSING", gate_names[gate], "Baggage handling started");
    emit_event("announcement", flight, "PUBLISHED", gate_names[gate], "Arrival announcement sent");
    printf("[CONTROLLER] Assigned %s to Gate %s.\n", flight, gate_names[gate]);
}

static void process_waiting_queue(void) {
    while (waiting_count > 0 && available_gate() >= 0) {
        FlightEntry entry = waiting[0];
        for (int i = 1; i < waiting_count; i++) waiting[i - 1] = waiting[i];
        waiting_count--;
        assign_flight(entry.flight, entry.time, -1);
        emit_event("queue", entry.flight, "PROMOTED", "", "Waiting flight assigned to newly available gate");
    }
}

static void check_gate_releases(void) {
    time_t now = time(NULL);
    for (int i = 0; i < 3; i++) {
        if (gates[i].occupied && now >= gates[i].release_at) {
            char flight[FLIGHT_ID_LEN];
            snprintf(flight, sizeof(flight), "%s", gates[i].flight);
            gates[i].occupied = 0;
            gates[i].flight[0] = '\0';
            char command[80];
            snprintf(command, sizeof(command), "RELEASE|%s", gate_names[i]);
            send_fifo(GATE_CMD_FIFO, command);
            snprintf(command, sizeof(command), "Ground service finished; Gate %s is available", gate_names[i]);
            emit_event("flight", flight, "COMPLETED", gate_names[i], "Simulated ground service completed");
            emit_event("gate", flight, "AVAILABLE", gate_names[i], command);
            printf("[CONTROLLER] %s completed service; Gate %s released.\n", flight, gate_names[i]);
        }
    }
    process_waiting_queue();
}

static void handle_emergency(const char *message) {
    printf("\n[CONTROLLER] EMERGENCY: %s\n", message);
    emit_event("emergency", "", "ACTIVE", "B", message);
    send_fifo(ANNOUNCE_FIFO, "EMERGENCY ALERT: Please follow airport staff instructions and keep emergency routes clear.");
    if (!gates[1].occupied) {
        send_fifo(GATE_CMD_FIFO, "EMERGENCY|CLEAR_B");
        emit_event("gate", "", "EMERGENCY_PRIORITY", "B", "Gate B is free for emergency access");
    } else {
        char detail[160];
        snprintf(detail, sizeof(detail), "Gate B is occupied by %s; it will not be force-released", gates[1].flight);
        emit_event("gate", gates[1].flight, "OCCUPIED", "B", detail);
    }
}

static pid_t spawn_worker(const char *program) {
    pid_t pid = fork();
    if (pid == 0) { execl(program, program, (char *)NULL); perror(program); _exit(127); }
    if (pid < 0) perror("fork worker");
    return pid;
}

static void process_frontend_command(char *line, int response_fd) {
    line[strcspn(line, "\r\n")] = '\0';
    if (!*line) return;
    if (!strncmp(line, "ADD|", 4)) {
        char flight[FLIGHT_ID_LEN] = {0}, time_label[FLIGHT_TIME_LEN] = {0};
        if (sscanf(line + 4, "%31[^|]|%31[^|]", flight, time_label) >= 1 && flight[0]) {
            if (!time_label[0]) snprintf(time_label, sizeof(time_label), "MANUAL-%03d", next_id++);
            assign_flight(flight, time_label, -1);
        } else {
            emit_event("system", "", "ERROR", "", "Invalid ADD command. Use ADD|FLIGHT-ID|TIME");
        }
    } else if (!strncmp(line, "EMERGENCY|", 10)) {
        const char *message = line + 10;
        send_fifo(EMERGENCY_REQ_FIFO, *message ? message : "Emergency vehicle approaching Gate B");
        emit_event("system", "", "RECEIVED", "", "Emergency command forwarded to emergency handler");
    } else if (!strcmp(line, "STATUS")) {
        send_fifo(GATE_CMD_FIFO, "STATUS");
        for (int i = 0; i < 3; i++) {
            emit_event("gate", gates[i].flight, gates[i].occupied ? "OCCUPIED" : "AVAILABLE", gate_names[i], gates[i].occupied ? "Gate currently assigned" : "Gate available");
        }
        emit_event("system", "", "OK", "", "Status snapshot sent");
    } else if (!strcmp(line, "STOP")) {
        stop_requested = 1;
    } else {
        emit_event("system", "", "ERROR", "", "Unknown command. Use ADD, EMERGENCY, STATUS or STOP");
    }
    (void)response_fd;
}

int main(void) {
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    signal(SIGPIPE, SIG_IGN);
    memset(gates, 0, sizeof(gates));

    const char *fifos[] = {GATE_CMD_FIFO, GATE_STATUS_FIFO, BAGGAGE_FIFO,
        ANNOUNCE_FIFO, EMERGENCY_FIFO, EMERGENCY_REQ_FIFO,
        FRONTEND_CMD_FIFO, EVENTS_FIFO};
    for (size_t i = 0; i < sizeof(fifos) / sizeof(fifos[0]); i++) create_fifo(fifos[i]);

    int flight_to_controller[2], controller_to_flight[2];
    if (pipe(flight_to_controller) < 0 || pipe(controller_to_flight) < 0) { perror("pipe"); return 1; }

    pid_t flight_pid = fork();
    if (flight_pid == 0) {
        close(flight_to_controller[0]); close(controller_to_flight[1]);
        char r[20], w[20];
        snprintf(r, sizeof(r), "%d", controller_to_flight[0]);
        snprintf(w, sizeof(w), "%d", flight_to_controller[1]);
        execl("./flight", "flight", r, w, (char *)NULL); perror("./flight"); _exit(127);
    }
    if (flight_pid < 0) { perror("fork flight"); return 1; }
    close(flight_to_controller[1]); close(controller_to_flight[0]);

    pid_t gate_pid = spawn_worker("./gate");
    pid_t baggage_pid = spawn_worker("./baggage");
    pid_t announcement_pid = spawn_worker("./announcement");
    pid_t emergency_pid = spawn_worker("./emergency");

    int frontend_fd = open(FRONTEND_CMD_FIFO, O_RDWR | O_NONBLOCK);
    int emergency_fd = open(EMERGENCY_FIFO, O_RDWR | O_NONBLOCK);
    if (frontend_fd < 0 || emergency_fd < 0) { perror("open command FIFO"); stop_requested = 1; }

    printf("\n=== AIRPORT GROUND OPERATIONS: IPC CONTROLLER ===\n");
    printf("Unnamed pipes: Flight <-> Controller\nNamed FIFOs: Gate, Baggage, Announcement, Emergency, Frontend commands, Events\n");
    printf("Automatic arrival interval: 8 seconds | Gate service: %d seconds\n", SERVICE_SECONDS);
    printf("Frontend command examples: ADD|AI-777|17:30, EMERGENCY|Medical vehicle approaching, STATUS, STOP\n\n");
    fflush(stdout);
    emit_event("system", "", "RUNNING", "", "Airport IPC controller started");

    char command_buffer[2048];
    size_t command_used = 0;
    while (!stop_requested) {
        fd_set set;
        FD_ZERO(&set);
        FD_SET(flight_to_controller[0], &set);
        if (frontend_fd >= 0) FD_SET(frontend_fd, &set);
        if (emergency_fd >= 0) FD_SET(emergency_fd, &set);
        int maxfd = flight_to_controller[0];
        if (frontend_fd > maxfd) maxfd = frontend_fd;
        if (emergency_fd > maxfd) maxfd = emergency_fd;
        struct timeval timeout = {1, 0};
        int ready = select(maxfd + 1, &set, NULL, NULL, &timeout);
        if (ready < 0) { if (errno == EINTR) continue; perror("select"); break; }

        if (FD_ISSET(flight_to_controller[0], &set)) {
            FlightMessage request;
            ssize_t n = read(flight_to_controller[0], &request, sizeof(request));
            if (n == (ssize_t)sizeof(request) && request.kind == 1) {
                printf("[CONTROLLER] Automatic request received: %s (%s)\n", request.flight, request.time);
                assign_flight(request.flight, request.time, controller_to_flight[1]);
            }
        }
        if (frontend_fd >= 0 && FD_ISSET(frontend_fd, &set)) {
            char chunk[512];
            ssize_t n = read(frontend_fd, chunk, sizeof(chunk));
            if (n > 0) {
                for (ssize_t i = 0; i < n; i++) {
                    if (chunk[i] == '\n') {
                        command_buffer[command_used] = '\0';
                        process_frontend_command(command_buffer, controller_to_flight[1]);
                        command_used = 0;
                    } else if (command_used + 1 < sizeof(command_buffer)) {
                        command_buffer[command_used++] = chunk[i];
                    } else command_used = 0;
                }
            }
        }
        if (emergency_fd >= 0 && FD_ISSET(emergency_fd, &set)) {
            char message[256];
            ssize_t n = read(emergency_fd, message, sizeof(message) - 1);
            if (n > 0) { message[n] = '\0'; message[strcspn(message, "\r\n")] = '\0'; handle_emergency(message); }
        }
        check_gate_releases();
    }

    emit_event("system", "", "STOPPING", "", "Controller shutting down");
    send_fifo(GATE_CMD_FIFO, "EXIT");
    send_fifo(BAGGAGE_FIFO, "EXIT");
    send_fifo(ANNOUNCE_FIFO, "EXIT");
    send_fifo(EMERGENCY_REQ_FIFO, "EXIT");
    close(flight_to_controller[0]); close(controller_to_flight[1]);
    if (frontend_fd >= 0) close(frontend_fd);
    if (emergency_fd >= 0) close(emergency_fd);
    if (flight_pid > 0) kill(flight_pid, SIGTERM);
    pid_t workers[] = {flight_pid, gate_pid, baggage_pid, announcement_pid, emergency_pid};
    for (size_t i = 0; i < sizeof(workers) / sizeof(workers[0]); i++) if (workers[i] > 0) waitpid(workers[i], NULL, 0);
    for (size_t i = 0; i < sizeof(fifos) / sizeof(fifos[0]); i++) unlink(fifos[i]);
    printf("[CONTROLLER] Shutdown complete; FIFOs cleaned up.\n");
    return 0;
}
