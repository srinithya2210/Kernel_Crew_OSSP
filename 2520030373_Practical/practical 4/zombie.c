#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>
int main()
{
int pid;
pid=fork();
if(pid==0)
{
printf("Child process %d\n",getpid());
exit(0);
}
else
{
printf("Parent process %d\n",getpid());
//sleep(30);
wait(NULL); //collecting 
printf("Parent process completed\n");
}
}
