#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/wait.h>
int main(){
int pid= fork();
if(pid==0){
printf("Child Process %d\n",getpid());
exit(0);
}else{
printf("Parent process %d\n",getpid());
sleep(30);
//wait(NULL);
printf("Parent process completed\n");
}
}
