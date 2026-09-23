#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main()
{
    int fd1, fd2;
    char buf[100];
    size_t n;

    fd1 = open("source.txt", O_RDONLY);
    fd2 = open("destination.txt", O_WRONLY | O_CREAT, 0644);

    if(fd1 == -1 || fd2 == -1)
    {
        printf("Error opening files\n");
        return 1;
    }

    printf("Files successfully opened\n");

    while((n = read(fd1, buf, 100)) > 0)
    {
        write(fd2, buf, n);
    }

    close(fd1);
    close(fd2);

    printf("File copied successfully\n");

    return 0;
}
