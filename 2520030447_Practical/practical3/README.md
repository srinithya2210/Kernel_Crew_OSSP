## System Calls Used

* **`fork()`** – Creates a new child process by duplicating the calling process.
* **`getpid()`** – Gives the PID of the process that is currently executing.
* **`getppid()`** – Returns the PID of the parent process associated with the current process.

## Process States

A process can pass through several states during its lifetime:

### 1. Ready

The process has everything it needs to run but is waiting for the CPU to be assigned by the scheduler.

### 2. Running

The process is actively executing instructions on the CPU.

### 3. Waiting

The process is temporarily paused while waiting for an event or resource, such as the completion of an I/O operation.

### 4. Terminated

The process has finished its execution and is removed from the system.

## Tools Used for Monitoring

* **`ps`** – Used to view information about processes currently present in the system.
* **`top`** – Continuously displays running processes along with their CPU and memory consumption.
* **`/proc`** – A virtual file system that provides detailed information about individual processes through directories such as `/proc/PID/`.

## Observation

Process details can be examined during program execution using commands such as `ps`, `top`, and the `/proc` file system. The state of a process changes as it receives CPU time, waits for resources or I/O, and eventually completes its execution.
