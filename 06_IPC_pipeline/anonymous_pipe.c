#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>

int main() {
    int fd[2];
    pid_t pid;
    char buffer[100];

    // 1. Tạo pipe trước khi fork
    if (pipe(fd) == -1) {
        perror("Tạo pipe thất bại");
        return 1;
    }

    pid = fork();

    if (pid < 0) {
        perror("Fork thất bại");
        return 1;
    }

    if (pid > 0) { // TIẾN TRÌNH CHA (Ghi dữ liệu)
        close(fd[0]); // Cha không đọc, đóng đầu đọc fd[0] để tránh lãng phí
        
        char msg[] = "Xin chào từ Tiến trình Cha!";
        write(fd[1], msg, strlen(msg) + 1);
        
        close(fd[1]); // Ghi xong thì đóng đầu ghi
    } 
    else { // TIẾN TRÌNH CON (Đọc dữ liệu)
        close(fd[1]); // Con không ghi, đóng đầu ghi fd[1]
        
        // Đọc dữ liệu từ pipe vào buffer
        read(fd[0], buffer, sizeof(buffer));
        printf("Tiến trình Con nhận được: %s\n", buffer);
        
        close(fd[0]); // Đọc xong thì đóng đầu đọc
    }

    return 0;
}