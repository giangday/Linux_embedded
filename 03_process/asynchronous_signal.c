#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

// Hàm tự động nhặt xác con khi nhận được tín hiệu SIGCHLD
void clean_zombie(int sig) {
    int status;
    pid_t pid = wait(&status); // Nhặt xác con ngay lập tức
    printf("\n[CHA - Handler] Đã tự động dọn dẹp xong con có PID: %d nhờ tín hiệu SIGCHLD.\n", pid);
}

int main() {
    // Đăng ký handler để dọn dẹp zombie tự động
    signal(SIGCHLD, clean_zombie);

    pid_t pid = fork();

    if (pid == 0) {
        // Tiến trình con
        printf("[CON] Tôi sống 2 giây rồi chết đây. PID: %d\n", getpid());
        sleep(2);
        exit(0);
    } 
    else {
        // Tiến trình cha làm việc riêng vô hạn
        printf("[CHA] Tôi cứ làm việc của tôi, không cần chủ động đợi wait()...\n");
        while(1) {
            printf("[CHA] Đang làm việc nặng...\n");
            sleep(1);
        }
    }
    return 0;
}