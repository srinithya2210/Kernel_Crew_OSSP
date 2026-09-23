#include<stdio.h>
#include<unistd.h>
#include<sys/wait.h>//used for wait() and waitpid() system calls
#include<stdlib.h>//used for exit() system call
int main(){
int pid[3];//initialising 3 childs
for(int i =0; i<3;i++)//loop which runs for 3 childs
{
pid[i]=fork();//creating 3 childs
if(pid[i]==0)//inidcates a child process
{
printf("Child %d pid=%d\n",i+1,getpid());
sleep(i+1);//giving a pause after every child
exit(0);//exiting the std output

}

}
wait(NULL);//waiting for any one child
printf("Parent: one child is completed with wait()\n");//parent is returning the status
waitpid(pid[2], NULL, 0);//waiting using a specific pid
printf("Parent: Child 3 has completed with waitpid()\n");//parent is checking the status
wait(NULL);// waiting for the child to be complete
printf("Parent: All the childs are completed\n");//parent is chec

}
