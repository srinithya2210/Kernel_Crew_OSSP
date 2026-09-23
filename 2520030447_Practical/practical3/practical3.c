#include <stdio.h>
#include <unistd.h>

int main()
{
    int pid;

    printf("Before fork\n");
    printf("PID: %d\n", getpid());
    printf("PPID: %d\n", getppid());

    pid = fork();

    if(pid == 0)
    {
        printf("\nChild process\n");
        printf("PID: %d\n", getpid());
        printf("PPID: %d\n", getppid());
        printf("Child process is running\n");
    }
    else if(pid > 0)
    {
        printf("\nParent process\n");
        printf("PID: %d\n", getpid());
        printf("PPID: %d\n", getppid());
        printf("Parent process is running\n");
    }
    else
    {
        printf("Fork failed\n");
    }

    return 0;
}
