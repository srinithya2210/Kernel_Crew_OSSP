#include<stdio.h>
#include<unistd.h>
#include<sys/wait.h>//used for wait() and waitpid() syscalls
#include<stdlib.h>//used for exit syscall
int main()
{
int pid[3];//inititalizing 3 childs
for(int i=0;i<3;i++){//loop running for 3 childs
pid[i]=fork();
if(pid[i]==0)//indicates a child process
{
printf("Child %d pid=%d\n",i+1,getpid());
sleep(i+1);//every child runs it gives pause after every child runs
exit(0);//exit std input dont take keyboard input exiting staandard input
}
}
wait(NULL);//waiting for any one child
printf("Parent One child is completed with wait()\n");
waitpid(pid[2],NULL,0);//waiting using a specific pid
printf("Parent: Child 3 has completed with wait pid()\n");
wait(NULL);//waiting for the last child to complete
printf("Parent: All the childs are completed\n");//parent is returning the last child status
}
