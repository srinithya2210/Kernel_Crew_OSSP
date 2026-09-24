#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
int main()
{
char message[100];
int fd;
mkfifo("myfifo", 0666);
printf("Server waiting for client...\n");
fd = open("myfifo", O_RDWR);
read(fd, message, sizeof(message));
printf("Client says: %s\n", message);
strcpy(message, "Message received by server");
write(fd, message, strlen(message) + 1);
close(fd);
}
