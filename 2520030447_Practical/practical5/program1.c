#include<stdio.h>
#include<unistd.h>
#include<sys/wait.h> // for wait systemcall
#include<time.h> //for clocks_per_sec and clock() systemcall
int main(){
int fd[2];//paprameter for anonymous pipe
int data;// variables that write data to pipe
int count; // to count the number of items
double communication_Ef;
int start, end;//variabes that take start and end times
double timetaken;
pipe(fd);//creating a anonymour pipe
start = clock();//storing the start time
if(fork() == 0){  //creating a child as consumer
close(fd[1]);  // closing the write end of the pipe
printf("Consumer\n");  // starting the consumer
for(int i=0;i<=4;i++){  //getting the consumer data from the pipe

read(fd[0],&data,sizeof(data));  //counsumer is reading the data from the pipe
printf("Consumed %d \n",data);  //printing the read data
}
close(fd[0]); //closing the read end of the pipe in the child
}
else{
close(fd[0]); //closing the read end of the pipe in the parent
printf("Producer \n"); //initializing the parent with producer
int count = 0;
for(int i=1;i<=5;i++){ //defining the producer data
data = i * 10; //initializing the data values in the producer
write(fd[1],&data,sizeof(data)); // writing data inot the pipe
printf("Produced %d\n",data); // printing the produced data
count++;
}
close(fd[1]); //closing the write end of the pipe
wait(NULL); //parent waits for the child to complete
end = clock(); // closing the clock to take the communiation time
timetaken = (double)(end - start)/ CLOCKS_PER_SEC;  // calculating the overall time
printf("Communication time is %lf\n",timetaken);  // printing the timetaken
communication_Ef = count / timetaken;
printf("Communication Efficiency = %lf\n",communication_Ef);

}
}
