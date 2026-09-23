## `wait()`

The `wait()` system call allows a parent process to pause until one of its child processes completes.

```c
wait(NULL);
```

After the child terminates, `wait()` collects its exit status. This also allows the operating system to remove the child's remaining entry from the process table.

## `waitpid()`

The `waitpid()` system call provides more control because the parent can specify which child process it wants to wait for.

```c
waitpid(pid, NULL, 0);
```

Here, `pid` represents the process ID of the child that the parent is waiting for.

## Zombie Process

A zombie is a process that has already completed execution, but its parent has not collected its termination status yet. Until the parent does so, an entry for the completed child remains in the process table.

A zombie can occur when:

1. The child process finishes execution.
2. The parent process continues running.
3. The parent does not call `wait()` or `waitpid()` to collect the child's status.

## Handling a Zombie Process

The parent process can prevent or clear a zombie by waiting for the terminated child:

```c
wait(NULL);
```

Alternatively, `waitpid()` can be used when the parent needs to wait for a particular child. These system calls collect the child's exit information and allow its process-table entry to be released.
