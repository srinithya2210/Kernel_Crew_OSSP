#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    char buf[50];
    int pid;

    printf("Enter a Linux command: ");
    scanf("%s", buf);

    pid = fork();

    if(pid == 0)
    {
        printf("Child process\n");
        printf("Child process id is %d\n", getpid());
        printf("Parent process id is %d\n", getppid());

        execlp(buf, buf, NULL);

        printf("Command execution failed\n");
    }
    else if(pid > 0)
    {
        printf("Parent process\n");
        printf("Parent process id is %d\n", getpid());
        printf("My parent process id is %d\n", getppid());

        wait(NULL);

        printf("Child process completed\n");
    }
    else
    {
        printf("Fork didn't execute successfully; it is a failure\n");
    }

    return 0;
}
