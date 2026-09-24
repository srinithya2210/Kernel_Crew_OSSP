## Pipe Communication

In this program, a **pipe** is used to allow two related processes to exchange data.

* The **parent process** works as the producer and sends data through the pipe.
* The **child process** acts as the consumer and receives the data.
* An **anonymous pipe** provides the communication channel between the two processes.
* Data is written into the pipe by the parent and read from it by the child.
* The efficiency of communication can be evaluated by measuring the time required to transfer the data.

## System Calls Used

* **`fork()`** → Creates a new child process.
* **`pipe()`** → Establishes a pipe for communication between the processes.
* **`dup2()`** → Redirects standard input or output to the appropriate end of the pipe.
* **`exec()`** → Replaces the current process with another program or command, such as `ls` or `grep`.

## Data Flow

```text
ls -l  →  Pipe  →  grep ".c"
```

The `ls -l` command generates the directory listing and sends its output into the pipe. The `grep ".c"` command receives this output and filters it to display entries matching `.c` files.
