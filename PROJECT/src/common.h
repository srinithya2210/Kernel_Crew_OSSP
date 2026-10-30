#ifndef AIRPORT_COMMON_H
#define AIRPORT_COMMON_H

#include <sys/types.h>

#define GATE_CMD_FIFO       "/tmp/airport_gate_cmd_fifo"
#define GATE_STATUS_FIFO    "/tmp/airport_gate_status_fifo"
#define BAGGAGE_FIFO        "/tmp/airport_baggage_fifo"
#define ANNOUNCE_FIFO       "/tmp/airport_announce_fifo"
#define EMERGENCY_FIFO      "/tmp/airport_emergency_fifo"
#define EMERGENCY_REQ_FIFO  "/tmp/airport_emergency_request_fifo"
#define FRONTEND_CMD_FIFO   "/tmp/airport_frontend_cmd_fifo"
#define EVENTS_FIFO         "/tmp/airport_events_fifo"

#define FLIGHT_ID_LEN 32
#define FLIGHT_TIME_LEN 32
#define STATUS_LEN 24
#define GATE_LEN 16

typedef struct {
    int kind; /* 1 = arrival request, 2 = flight finished */
    char flight[FLIGHT_ID_LEN];
    char time[FLIGHT_TIME_LEN];
    char gate[GATE_LEN];
} FlightMessage;

typedef struct {
    char flight[FLIGHT_ID_LEN];
    char status[STATUS_LEN];
    char gate[GATE_LEN];
} FlightResponse;

#endif
