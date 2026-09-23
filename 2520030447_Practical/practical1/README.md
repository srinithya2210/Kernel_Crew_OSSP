# Linux System Information and OS Abstraction

## Linux Commands

* **`uname -a`** → Displays basic information about the Linux system, including the kernel version, system architecture, and other OS details.
* **`lscpu`** → Shows details about the CPU, including its architecture, number of cores, threads, and other processor-related information.
* **`lsblk`** → Lists the available block devices, such as hard disks, SSDs, and their partitions.
* **`ps`** → Provides information about the processes currently running on the system.
* **`top`** → Displays live system statistics, including CPU usage, memory usage, and the processes consuming system resources.

## Role of the Operating System

An Operating System (OS) acts as an interface between computer hardware and application programs. It manages the available hardware resources and provides services that allow applications to use the system without having to interact with the hardware directly.

### CPU Abstraction

The OS manages the CPU through processes and scheduling. Instead of applications directly controlling the processor, the OS scheduler determines which process should run and how much CPU time it receives.

### Memory Abstraction

The OS provides each process with a virtual memory space. This allows programs to use memory without having to know where their data is physically stored in RAM.

### Storage Abstraction

The OS simplifies storage management by organizing physical storage devices into files, directories, and file systems. Applications can access data through files instead of dealing directly with disk sectors.

### I/O Device Abstraction

The OS communicates with hardware devices using device drivers. Devices such as keyboards, displays, storage devices, and other peripherals can therefore be accessed through standard OS interfaces and system calls.

## Conclusion

The Operating System simplifies interaction between applications and hardware by providing useful abstractions for the CPU, memory, storage, and I/O devices. This hides much of the hardware complexity and provides applications with a safer and easier way to use system resources.
