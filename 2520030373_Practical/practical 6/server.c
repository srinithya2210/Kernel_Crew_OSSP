#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
int main()
{
char message[100];
int fd1, fd2;
mkfifo("client_fifo", 0666);
mkfifo("server_fifo", 0666);
printf("Server waiting for client...\n");
fd1 = open("client_fifo", O_RDONLY);
read(fd1, message, sizeof(message));
printf("Client says: %s\n", message);
close(fd1);
strcpy(message, "Message received by server");
fd2 = open("server_fifo", O_WRONLY);
write(fd2, message, strlen(message) + 1);
close(fd2);
return 0;
}
