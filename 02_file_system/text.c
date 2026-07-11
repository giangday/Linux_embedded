#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
// int open(const char *pathname, int flags);
// int open(const char *pathname, int flags, mode_t mode);


int main(){
    int fd = open("text.txt", O_RDWR, 0644);

    if(fd == -1){
        perror("open error");
        return -1;
    }

    char text[] = "hello world\n";
    write(fd, text, sizeof(text) - 1);

    lseek(fd, 12, SEEK_SET);
    write(fd, "linux\n", 6);

    close(fd);
    return 0;

}