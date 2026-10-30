# Airport Ground Operations — Linux IPC Upgrade

This version keeps the project focused on real Linux IPC while allowing both automatic arrivals and manual frontend flight requests.

## Processes

- `controller.c`: central coordinator; creates the FIFOs, launches worker processes, assigns gates, maintains a waiting queue, releases gates after simulated ground service, and emits JSON-line events.
- `flight.c`: automatic arrival generator. It submits a new arrival every 8 seconds over unnamed pipes.
- `gate.c`: receives gate commands through a named FIFO and reports gate state through a second FIFO.
- `baggage.c`: receives baggage-handling messages through a named FIFO.
- `announcement.c`: receives announcement messages through a named FIFO.
- `emergency.c`: receives emergency requests through a request FIFO and forwards alerts to the controller through the emergency FIFO.
- `bridge/server.js`: optional Node.js HTTP/SSE bridge for the React frontend. It uses Node's built-in modules only.

## 1. Compile in Ubuntu/WSL

Open Ubuntu/WSL and `cd` to the folder containing these files:

```bash
gcc -std=gnu11 -Wall -Wextra -o flight flight.c
gcc -std=gnu11 -Wall -Wextra -o gate gate.c
gcc -std=gnu11 -Wall -Wextra -o baggage baggage.c
gcc -std=gnu11 -Wall -Wextra -o announcement announcement.c
gcc -std=gnu11 -Wall -Wextra -o emergency emergency.c
gcc -std=gnu11 -Wall -Wextra -o controller controller.c
```

The controller launches the five child processes itself. **Do not start `flight`, `gate`, `baggage`, `announcement`, or `emergency` separately.**

## 2. Run the C IPC system

```bash
./controller
```

The controller prints events to the terminal. Automatic flights arrive every 8 seconds. Each assigned flight uses its gate for 25 seconds in this demo, after which the gate is released and the oldest waiting flight is assigned next.

To stop, enter `Ctrl+C` in the controller terminal. The controller signals shutdown, stops workers, and removes the FIFOs.

## 3. Run the Node bridge

Use a second Ubuntu/WSL terminal, in this project folder:

```bash
node bridge/server.js
```

No `npm install` is required for this bridge. Keep the bridge and C controller running in the same WSL environment because they share `/tmp` FIFOs.

### HTTP API

- `GET http://localhost:3001/api/events` — Server-Sent Events stream of events emitted by the real C controller.
- `POST http://localhost:3001/api/flights` with JSON `{"flight":"AI-777","time":"17:30"}` — adds a manual flight request.
- `POST http://localhost:3001/api/emergency` with JSON `{"message":"Medical vehicle approaching Gate B"}` — sends an emergency request.
- `POST http://localhost:3001/api/status` — requests a gate status snapshot.

Example manual request from a third terminal:

```bash
curl -X POST http://localhost:3001/api/flights \
  -H 'Content-Type: application/json' \
  -d '{"flight":"AI-777","time":"17:30"}'

curl -X POST http://localhost:3001/api/emergency \
  -H 'Content-Type: application/json' \
  -d '{"message":"Medical vehicle approaching Gate B"}'
```

## FIFO protocol

The controller creates these named FIFOs:

- `/tmp/airport_gate_cmd_fifo`
- `/tmp/airport_gate_status_fifo`
- `/tmp/airport_baggage_fifo`
- `/tmp/airport_announce_fifo`
- `/tmp/airport_emergency_fifo`
- `/tmp/airport_emergency_request_fifo`
- `/tmp/airport_frontend_cmd_fifo`
- `/tmp/airport_events_fifo`

Frontend commands are newline-delimited text written to `/tmp/airport_frontend_cmd_fifo`:

- `ADD|FLIGHT-ID|TIME`
- `EMERGENCY|message`
- `STATUS`
- `STOP`

Events on `/tmp/airport_events_fifo` are newline-delimited JSON objects with `type`, `flight`, `status`, `gate`, `message`, and Unix `timestamp` fields. These events originate from the C controller; the bridge relays them rather than inventing operational state.

## Demo limitations / next improvements

This is a teaching simulation, not a real airport system. Ground service duration is simulated. Gate release occurs after a fixed interval; real systems would use explicit service-complete signals. Emergency handling does not forcibly release an occupied gate. The automatic generator cycles through a small list of demo flight IDs; it is not a live airline feed.
