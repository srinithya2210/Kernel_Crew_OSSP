# ✈️ Airport Ground Operations Using Linux IPC

## 📌 Project Overview

This project demonstrates **Inter-Process Communication (IPC)** in Linux using **Unnamed Pipes** and **Named Pipes (FIFOs)** in a simulated airport ground operations environment.

An airport consists of multiple operations such as **flight management, gate allocation, baggage handling, and passenger announcements**. These operations can be represented as separate processes that need to exchange information and coordinate with each other.

This project simulates communication between these airport operations using Linux processes and IPC mechanisms.

A **React-based dashboard** is also used to visualize flight status, gate assignments, baggage status, passenger announcements, and IPC activity.

The core implementation focuses on **C programming, Linux/POSIX processes, Pipes, and FIFOs**.

---

## 🎯 Problem Statement

Airport ground operations involve multiple activities that need to coordinate with each other.

When a flight arrives, information such as the flight number, arrival status, gate assignment, baggage information, and passenger announcements need to be communicated between different airport operations.

The problem addressed by this project is:

> **How can separate airport ground-operation processes communicate and coordinate efficiently using Linux Inter-Process Communication mechanisms?**

This project provides a simulated solution using **Unnamed Pipes and Named FIFOs**.

---

## 💡 Proposed Solution

The project represents different airport ground operations as separate Linux processes.

The main communication flow is:

```text
                    ✈️ Flight Process
                           |
                           | Flight Information
                           ↓
                 🎛️ Airport Controller
                    /       |       \
                   /        |        \
                  ↓         ↓         ↓
             🚪 Gate     🧳 Baggage   📢 Announcement
             Process       Process       Process
```

The **Airport Controller** acts as the central coordination process.

The basic workflow is:

1. The Flight Process generates flight information.
2. The Flight Process sends the information to the Airport Controller.
3. The Airport Controller processes the flight information.
4. The controller assigns an available gate.
5. Gate information is sent to the Gate Process.
6. Flight information is sent to the Baggage Process.
7. Announcement information is sent to the Announcement Process.
8. The React dashboard displays the current airport status and IPC activity.

---

## 🎯 Objectives

- To understand the concept of **Inter-Process Communication (IPC)** in Linux.
- To demonstrate communication between related processes using **Unnamed Pipes**.
- To demonstrate communication between independent processes using **Named FIFOs**.
- To simulate coordination between different airport ground operations.
- To understand Linux process creation and management.
- To demonstrate data transfer using `read()` and `write()`.
- To understand the use of file descriptors in IPC.
- To visualize IPC communication using a React dashboard.
- To demonstrate a practical application of Linux/POSIX IPC.

---

## 🧠 Concepts Demonstrated

### 1. Inter-Process Communication (IPC)

IPC allows separate processes to exchange data and coordinate their activities.

This project demonstrates IPC using:

- Unnamed Pipes
- Named Pipes (FIFOs)

### 2. Unnamed Pipe

An unnamed pipe is a communication channel mainly used between related processes.

For example, the Flight Process and Airport Controller can communicate through an unnamed pipe.

```text
✈️ Flight Process
       |
       | Flight Information
       ↓
     PIPE
       |
       ↓
🎛️ Airport Controller
```

A pipe has two file descriptors:

```text
fd[0] → Read end
fd[1] → Write end
```

### 3. Named Pipe / FIFO

A FIFO is a named communication channel that can be used by independent processes.

For example, the Airport Controller can communicate with an independently running Announcement Process through a FIFO.

```text
🎛️ Airport Controller
          |
          | Announcement
          ↓
        FIFO
          |
          ↓
📢 Announcement Process
```

The FIFO can be created using:

```c
mkfifo("airport_fifo", 0666);
```

---

## 🛠️ Technologies Used

| Technology | Purpose |
|---|---|
| C | Core IPC implementation |
| Linux / Ubuntu | Operating system environment |
| POSIX IPC | Process communication |
| Pipes | Communication between related processes |
| FIFOs | Communication between independent processes |
| GCC | C compiler |
| React | Frontend dashboard |
| GitHub | Project repository and documentation |

---

## 📂 Project Structure

```text
AIRPORT_IPC/
│
├── backend/
│   ├── flight_process.c
│   ├── airport_controller.c
│   ├── gate_process.c
│   ├── baggage_process.c
│   ├── announcement_process.c
│   └── ipc_common.h
│
├── frontend/
│   └── React Dashboard
│
├── screenshots/
│
├── PPT/
│
└── README.md
```

### File Description

| File / Folder | Description |
|---|---|
| `flight_process.c` | Simulates flight arrival and sends flight information |
| `airport_controller.c` | Coordinates the different airport operations |
| `gate_process.c` | Handles gate-related information |
| `baggage_process.c` | Simulates baggage handling |
| `announcement_process.c` | Handles passenger announcement information |
| `ipc_common.h` | Contains common IPC definitions |
| `frontend/` | React-based visualization dashboard |
| `screenshots/` | Contains screenshots of project execution |
| `PPT/` | Contains project presentation |
| `README.md` | Project documentation |

---

## ⚙️ System Calls Used

### `fork()`

Creates a new child process.

```c
pid_t pid = fork();
```

### `pipe()`

Creates an unnamed pipe.

```c
int fd[2];
pipe(fd);
```

The two file descriptors are:

```text
fd[0] → Read end
fd[1] → Write end
```

### `mkfifo()`

Creates a named pipe (FIFO).

```c
mkfifo("airport_fifo", 0666);
```

### `open()`

Opens a FIFO for reading or writing.

```c
open("airport_fifo", O_WRONLY);
```

or

```c
open("airport_fifo", O_RDONLY);
```

### `read()`

Reads data from a pipe or FIFO.

```c
read(fd, buffer, size);
```

### `write()`

Writes data into a pipe or FIFO.

```c
write(fd, message, size);
```

### `close()`

Closes a file descriptor after communication is completed.

```c
close(fd);
```

---

## 🔄 Project Workflow

```text
Start Airport System
        ↓
Create / Start Processes
        ↓
Flight Process Generates Flight Information
        ↓
Send Information to Airport Controller
        ↓
Airport Controller Processes Information
        ↓
Assign Gate
        ↓
Send Information to Baggage Process
        ↓
Send Announcement Information
        ↓
Update Airport Status
        ↓
React Dashboard Displays Current Status
        ↓
Processes Continue / Terminate
```

---

## ✈️ Example Scenario

Suppose **Flight AI202** arrives at the airport.

The Flight Process generates:

```text
Flight Number: AI202
Status: Arrived
Passengers: 180
```

The information is sent to the Airport Controller.

The controller processes the information and assigns a gate:

```text
Flight: AI202
Assigned Gate: G12
Status: Arrived
```

The controller then communicates the information to the relevant processes.

```text
✈️ Flight Process
       |
       | AI202 Arrived
       ↓
🎛️ Airport Controller
       |
       +------→ 🚪 Gate Process
       |          Gate G12 Assigned
       |
       +------→ 🧳 Baggage Process
       |          Baggage Processing Started
       |
       +------→ 📢 Announcement Process
                  Flight Arrival Announced
```

The React dashboard can display:

```text
Flight       Status       Gate       Baggage
------------------------------------------------
AI202        Arrived      G12        Processing
```

---

## 🔀 Pipe vs FIFO

| Feature | Unnamed Pipe | FIFO |
|---|---|---|
| Name | No name | Has a name |
| Creation | `pipe()` | `mkfifo()` |
| Main Use | Related processes | Independent processes |
| Example | Flight → Controller | Controller → Announcement |
| File System Entry | No named file | Appears as a FIFO |
| Communication | Process-to-process | Process-to-process |

---

## 🏗️ System Architecture

```text
                         ✈️ AIRPORT IPC SYSTEM
                                  |
                    +-------------+-------------+
                    |                           |
                    ↓                           ↓
             ✈️ Flight Process          🎛️ Airport Controller
                                                |
                         +----------------------+----------------------+
                         |                      |                      |
                         ↓                      ↓                      ↓
                   🚪 Gate Process       🧳 Baggage Process     📢 Announcement
                                                                    Process
```

Communication mechanisms:

```text
Related Processes
       ↓
Unnamed Pipes

Independent Processes
       ↓
Named FIFOs
```

---

## 💻 Implementation

### Unnamed Pipe

The Flight Process and Airport Controller can communicate using an unnamed pipe.

```c
int flight_to_controller[2];
```

The communication flow is:

```text
Flight Process
      |
      | flight_to_controller
      ↓
Airport Controller
```

For two-way communication, another pipe can be used:

```c
int controller_to_flight[2];
```

```text
Flight Process
      |
      | Flight Information
      ↓
Airport Controller
      |
      | Response / Gate Information
      ↓
Flight Process
```

### FIFO

The FIFO implementation uses:

```c
#define FIFO_NAME "airport_fifo"
```

The FIFO is created using:

```c
mkfifo(FIFO_NAME, 0666);
```

The writer opens it for writing:

```c
open(FIFO_NAME, O_WRONLY);
```

The reader opens it for reading:

```c
open(FIFO_NAME, O_RDONLY);
```

Messages are transferred using:

```c
write()
read()
```

---

## 🖥️ React Dashboard

The React dashboard provides a visual representation of the airport simulation.

The dashboard can display:

- ✈️ Flight status
- 🚪 Gate assignments
- 🧳 Baggage status
- 📢 Passenger announcements
- 🔄 IPC messages
- 🟢 Active processes
- ⏱️ Current operation status

Example dashboard:

```text
=================================================
           AIRPORT GROUND OPERATIONS
=================================================

Flight       Status       Gate       Baggage
-------------------------------------------------
AI202        Arrived      G12        Processing
6E451        Boarding     B08        Ready
UK832        Delayed      --         Waiting

-------------------------------------------------

IPC ACTIVITY

✈️ Flight Process
       ↓
🎛️ Airport Controller
       ↓
🚪 Gate Process       → Gate G12 Assigned
🧳 Baggage Process    → Baggage Processing
📢 Announcement       → Arrival Announced

=================================================
```

The dashboard is mainly used for **visualizing the IPC activity**. The core project remains the Linux/POSIX process communication system.

---

## 🔨 Compilation

Open a Linux/Ubuntu terminal and navigate to the project directory.

Compile the C programs:

```bash
gcc flight_process.c -o flight_process
gcc airport_controller.c -o airport_controller
gcc gate_process.c -o gate_process
gcc baggage_process.c -o baggage_process
gcc announcement_process.c -o announcement_process
```

---

## ▶️ Running the Project

Start the Airport Controller:

```bash
./airport_controller
```

Then start the required processes:

```bash
./flight_process
```

```bash
./gate_process
```

```bash
./baggage_process
```

```bash
./announcement_process
```

The exact execution order depends on the IPC connections implemented in the final system.

---

## 📊 Expected Output

Example:

```text
========================================
        AIRPORT IPC SYSTEM
========================================

[Flight] Flight AI202 has arrived.

[Controller] Flight information received.
[Controller] Processing flight AI202...
[Controller] Gate G12 assigned.

[Gate] Gate G12 assigned to AI202.

[Baggage] Baggage handling started for AI202.

[Announcement] Flight AI202 has arrived at Gate G12.

[Controller] All operations updated successfully.
```

The same information can be reflected on the React dashboard.

---

## 📸 Output Screenshots

Screenshots of the successful project execution will be included in the `screenshots/` folder.

The screenshots will demonstrate:

- Flight process execution
- Airport Controller receiving flight information
- Gate assignment
- Baggage status
- Passenger announcement
- IPC communication
- React dashboard displaying airport status

---

## 🌍 Applications

The concepts demonstrated in this project can be applied to:

- Flight management systems
- Gate allocation systems
- Baggage handling systems
- Passenger information systems
- Airport operation coordination
- Process-based workflows
- Linux-based system applications

This project is a **simulation for demonstrating Linux IPC concepts** and is not intended to replace an actual airport management system.

---

## ⚠️ Limitations

- The airport environment is simulated.
- The project does not connect to real airport systems.
- Gate allocation is based on simulated data.
- The number of processes is limited to the project requirements.
- Pipes and FIFOs provide basic communication mechanisms.
- The React dashboard is used for visualization and does not replace the underlying IPC implementation.

---

## 🔮 Future Scope

The project can be extended by:

- Supporting multiple simultaneous flights.
- Adding multiple gates and dynamic gate allocation.
- Adding synchronization mechanisms such as **semaphores**.
- Supporting multiple baggage handling processes.
- Implementing continuous IPC communication.
- Adding other IPC mechanisms such as message queues and shared memory.
- Adding process monitoring and error handling.
- Improving the React dashboard with live status updates.
- Adding communication logs and history.
- Comparing different Linux IPC mechanisms based on their performance and use cases.

---

## ✅ Expected Outcomes

After completing the project, the system should demonstrate:

1. Successful creation and management of multiple Linux processes.
2. Communication between related processes using unnamed pipes.
3. Communication between independent processes using FIFOs.
4. Exchange of flight and airport operation information.
5. Coordination between flight, gate, baggage, and announcement processes.
6. Visualization of the airport simulation through the React dashboard.
7. Practical understanding of Linux/POSIX IPC mechanisms.

---

## 👥 Team Members and Coordination

| Name | Roll Number | Responsibility |
|---|---|---|
| M Sri Nithya | 2520030447 | Frontend development, GitHub repository management, integration and documentation support |
| G Jahnavi | 2520030483 | Documentation and PPT preparation |
| P Yasasri | 2520030373 | Linux/C IPC implementation and testing |

### Team Coordination

The project is divided into three major areas:

- **Linux/C IPC:** Yasasri is responsible for implementing the Linux processes, Pipes, FIFOs, and related IPC functionality.
- **Documentation & PPT:** Jahnavi is responsible for preparing the project documentation and presentation.
- **Frontend & GitHub:** M Sri Nithya is responsible for developing the React frontend, maintaining the GitHub repository, and integrating the project components.

The team will coordinate the individual components to produce the final integrated airport IPC simulation.

---

## ✅ Conclusion

The **Airport Ground Operations Using Linux IPC** project demonstrates how Linux processes can communicate and coordinate in a realistic simulated airport environment.

The Flight Process communicates flight information to the Airport Controller, which coordinates the Gate, Baggage, and Announcement Processes using **unnamed pipes and named FIFOs**.

A React dashboard provides a visual representation of the airport status and IPC activity.

The project combines **Linux process management, POSIX IPC, C programming, and frontend visualization** to demonstrate a practical application of Inter-Process Communication.

---

## 📚 References

Linux manual pages:

```text
man pipe
man fork
man mkfifo
man open
man read
man write
man close
```

---

## 📌 Project Summary

| Category | Details |
|---|---|
| Project | Airport Ground Operations Using Linux IPC |
| Domain | Airport Ground Operations Simulation |
| Language | C |
| Frontend | React |
| Platform | Linux / Ubuntu |
| IPC Mechanisms | Unnamed Pipes and Named FIFOs |
| Main System Calls | `fork()`, `pipe()`, `mkfifo()`, `open()`, `read()`, `write()`, `close()` |
| Main Processes | Flight, Controller, Gate, Baggage, Announcement |
| Visualization | React Dashboard |
