#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
int main()
{
char message[100];
int fd;
fd = open("myfifo", O_RDWR);
printf("Enter message: ");
fgets(message, sizeof(message), stdin);
write(fd, message, strlen(message) + 1);
read(fd, message, sizeof(message));
printf("Server says: %s\n", message);
close(fd);
}
