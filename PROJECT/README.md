# Linux Inter-Process Communication Using Pipes and FIFOs

## 📌 Project Overview

This project demonstrates **Inter-Process Communication (IPC)** in Linux using **Unnamed Pipes** and **Named Pipes (FIFOs)**.

In Linux, processes normally have separate memory spaces. Therefore, a mechanism is required for processes to exchange information. This project demonstrates how processes can communicate by creating communication channels and transferring messages between them.

The project is implemented using the **C programming language** and Linux system calls such as:

- `fork()`
- `pipe()`
- `mkfifo()`
- `read()`
- `write()`
- `close()`

---

## 🎯 Objectives

- To understand the concept of Inter-Process Communication (IPC).
- To demonstrate communication between related processes using unnamed pipes.
- To demonstrate communication between independent processes using named pipes (FIFOs).
- To understand the use of Linux system calls for process communication.
- To demonstrate data transfer between processes using `read()` and `write()`.
- To understand file descriptors and communication channels in Linux.
- To demonstrate proper opening and closing of communication channels.

---

## 🧠 Concepts Demonstrated

### 1. Inter-Process Communication (IPC)

IPC allows processes to exchange data and coordinate with each other.

This project demonstrates IPC using:

- Unnamed Pipes
- Named Pipes (FIFOs)

### 2. Unnamed Pipe

An unnamed pipe is a communication channel used mainly between related processes, such as a parent process and its child process.

The project uses `pipe()` to create the pipe and `fork()` to create the child process.

The communication flow is:

```text
Parent Process
|
| Message
v
PIPE
|
v
Child Process
```

The project also demonstrates two-way communication using two pipes:

```text
Parent -----> Child
Pipe 1

Parent <----- Child
Pipe 2
```

### 3. Named Pipe / FIFO

A FIFO (First In, First Out) is a named communication channel that can be used by independent processes.

The FIFO is created using `mkfifo()`.

One process acts as the writer and another process acts as the reader.

The communication flow is:

```text
Writer Process
|
| Message
v
FIFO
|
v
Reader Process
```

---

## 🛠️ Technologies Used

| Technology | Purpose |
|---|---|
| C | Programming language |
| Linux / Ubuntu | Operating system environment |
| GCC | C compiler |
| WSL | Linux environment on Windows |
| GitHub | Project repository and documentation |

---

## 📂 Project Structure

```text
OSSP_PROJECT/
│
├── pipe_ipc.c
├── fifo_writer.c
├── fifo_reader.c
├── README.md
├── screenshots/
└── PPT/
```

### File Description

| File | Description |
|---|---|
| `pipe_ipc.c` | Demonstrates two-way communication between a parent and child process using unnamed pipes |
| `fifo_writer.c` | Sends a message through the named FIFO |
| `fifo_reader.c` | Receives a message from the named FIFO |
| `README.md` | Project documentation |
| `screenshots/` | Contains screenshots of program execution |
| `PPT/` | Contains the project presentation |

---

## ⚙️ System Calls Used

### `fork()`

Creates a new child process.

```c
pid = fork();
```

After `fork()`, there are two processes:

- Parent process
- Child process

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
mkfifo("my_fifo", 0666);
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

The overall workflow of the project is:

```text
Create Processes
       ↓
Establish Pipe / FIFO
       ↓
Send Data
       ↓
Receive Data
       ↓
Display Output
       ↓
Close Communication Channel
```

---

## 🔨 Compilation

Open a Linux/Ubuntu terminal and navigate to the project directory.

Compile the unnamed pipe program:

```bash
gcc pipe_ipc.c -o pipe_ipc
```

Compile the FIFO writer:

```bash
gcc fifo_writer.c -o fifo_writer
```

Compile the FIFO reader:

```bash
gcc fifo_reader.c -o fifo_reader
```

---

## ▶️ Running the Project

### 1. Unnamed Pipe

Run:

```bash
./pipe_ipc
```

The program creates a child process and demonstrates two-way communication between the parent and child.

Example output:

```text
UNNAMED PIPE - TWO WAY IPC DEMO

[Parent] Process started.
[Parent] Sending message to Child...
[Parent] Message sent successfully.

[Child] Process started.
[Child] Received from Parent: Hello Child! This message is from the Parent.
[Child] Response sent to Parent.

[Parent] Received from Child: Hello Parent! Message received successfully.

[Parent] Child process completed.
[Parent] IPC demonstration completed successfully.
```

### 2. FIFO Communication

The FIFO demonstration uses two separate programs:

- FIFO Reader
- FIFO Writer

#### Terminal 1 – Reader

Run:

```bash
./fifo_reader
```

The reader waits for a message:

```text
FIFO READER PROCESS

========================================

FIFO 'my_fifo' is ready.
[Reader] Waiting for a message...
```

Keep this terminal running.

#### Terminal 2 – Writer

Open another terminal and navigate to the project directory:

```bash
cd ~/OSSP_PROJECT
```

Then run:

```bash
./fifo_writer
```

Enter a message when prompted:

```text
Enter message to send: Hello from the FIFO writer!
```

The writer sends the message through the FIFO.

The reader receives:

```text
[Reader] Message received: Hello from the FIFO writer!
[Reader] FIFO closed.
[Reader] Communication completed successfully.
```

---

## 📊 Pipe vs FIFO

| Feature | Unnamed Pipe | FIFO |
|---|---|---|
| Name | No name | Has a name |
| Creation | `pipe()` | `mkfifo()` |
| Main use | Related processes | Independent processes |
| Example | Parent ↔ Child | Writer ↔ Reader |
| File system entry | Not normally visible as a named file | Appears as a FIFO |
| Programs | One program demonstrates both processes | Separate writer and reader programs |

---

## 🏗️ System Architecture

```text
                    IPC PROJECT
                         |
             +-----------+-----------+
             |                       |
             ↓                       ↓
       UNNAMED PIPE                FIFO
             |                       |
       Parent ↔ Child          Writer ↔ Reader
             |                       |
          pipe()                 mkfifo()
          fork()                  open()
          write()                write()
          read()                  read()
          close()                close()
```

---

## 💻 Implementation

### Unnamed Pipe

The unnamed pipe implementation creates two pipes:

```c
int parent_to_child[2];
int child_to_parent[2];
```

One pipe transfers data from the parent to the child.

The second pipe transfers the response from the child back to the parent.

```text
Parent
  |
  | parent_to_child
  ↓
Child
  |
  | child_to_parent
  ↓
Parent
```

### FIFO

The FIFO implementation uses:

```c
#define FIFO_NAME "my_fifo"
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

Messages are transferred using `write()` and `read()`.

---

## 📸 Output Screenshots

Screenshots of the successful program execution are included in the `screenshots` folder.

### Unnamed Pipe Output

The screenshot shows successful two-way communication between the parent and child processes.

### FIFO Output

The screenshots show:

- FIFO Reader waiting for a message.
- FIFO Writer sending a message.
- FIFO Reader receiving the message.

---

## 🌍 Applications

Pipes and FIFOs can be used for:

- Communication between processes in Linux systems.
- Data transfer between related processes.
- Communication between independent processes.
- Shell command pipelines.
- Producer-consumer style communication.
- Coordination between system processes.

---

## ⚠️ Limitations

- Unnamed pipes are mainly useful for communication between related processes.
- FIFO communication requires the communicating processes to correctly open the FIFO.
- Pipes provide a relatively simple communication mechanism.
- This project demonstrates basic message transfer rather than a large-scale IPC system.
- The project focuses specifically on pipes and FIFOs.

---

## 🔮 Future Scope

The project can be extended by:

- Supporting multiple readers and writers.
- Implementing continuous message exchange.
- Adding more IPC mechanisms such as message queues and shared memory.
- Adding synchronization mechanisms.
- Developing a menu-driven interface for selecting different IPC mechanisms.
- Comparing the performance of different IPC techniques.

---

## ✅ Conclusion

This project demonstrates Inter-Process Communication in Linux using unnamed pipes and named pipes (FIFOs).

The unnamed pipe implementation demonstrates communication between a parent and child process using `fork()` and `pipe()`.

The FIFO implementation demonstrates communication between separate writer and reader processes using `mkfifo()`.

Through this project, the basic concepts of Inter-Process Communication, process creation, file descriptors, system calls, data transfer, and resource management are demonstrated.

---

## 👥 Team Members

| Name | Roll Number | Contribution |
|---|---|---|
| M Sri Nithya | 2520030447 | Project implementation, testing and documentation |
| G Jahnavi | 2520030483 | PPT and documentation |
| P Yasasri | 2520030373 | Testing and implementation |

---

## 📚 References

Linux manual pages (man pages):

- `pipe()` system call
- `fork()` system call
- `mkfifo()` system call
- `read()` system call
- `write()` system call
- `close()` system call

Linux manual pages can be accessed using:

```bash
man pipe
man fork
man mkfifo
man read
man write
man close
```

---

## 📌 Project Summary

| Category | Details |
|---|---|
| Project | Linux Inter-Process Communication Using Pipes and FIFOs |
| Language | C |
| Platform | Linux / Ubuntu |
| IPC Mechanisms | Unnamed Pipes and Named Pipes (FIFOs) |
| Main System Calls | `fork()`, `pipe()`, `mkfifo()`, `read()`, `write()`, `close()` |
