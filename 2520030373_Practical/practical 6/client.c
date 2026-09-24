#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
int main()
{
char message[100];
int fd1, fd2;
fd1 = open("client_fifo", O_WRONLY);
printf("Enter message: ");
fgets(message, sizeof(message), stdin);
write(fd1, message, strlen(message) + 1);
close(fd1);
fd2 = open("server_fifo", O_RDONLY);
read(fd2, message, sizeof(message));
printf("Server says: %s\n", message);
close(fd2);
return 0;
}
