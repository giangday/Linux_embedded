#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>

// Hàm xử lý tín hiệu (Signal Handler) của tiến trình Con
void handler_cua_con(int sig) {
    if (sig == SIGUSR1) {
        printf("[CON] Nhận được tín hiệu SIGUSR1 từ Cha! Đang xử lý yêu cầu...\n");
    } else if (sig == SIGINT) {
        printf("[CON] Ối! Cha gửi lệnh SIGINT bắt con chết. Tạm biệt...\n");
        exit(0); // Con tự sát theo lệnh cha
    }
}

int main() {
    pid_t pid = fork();

    if (pid < 0) {
        perror("Fork thất bại");
        return 1;
    }
    else if (pid == 0) {
        // --- TIẾN TRÌNH CON ---
        // Đăng ký nhận tín hiệu từ cha
        signal(SIGUSR1, handler_cua_con);
        signal(SIGINT, handler_cua_con);

        printf("[CON] Con đang chạy vòng lặp và chờ lệnh từ Cha...\n");
        while(1) {
            sleep(1); // Treo máy đợi tín hiệu
        }
    } 
    else {
        // --- TIẾN TRÌNH CHA ---
        sleep(2); // Đợi 2 giây cho con ổn định
        
        printf("[CHA] Gửi tín hiệu SIGUSR1 (Ra lệnh) tới Con (PID: %d)...\n", pid);
        kill(pid, SIGUSR1); // Dùng hàm kill() để gửi signal tới một PID cụ thể
        
        sleep(3); // Đợi tiếp 3 giây
        
        printf("[CHA] Gửi tín hiệu SIGINT (Kết liễu) tới Con...\n");
        kill(pid, SIGINT);
        
        wait(NULL); // Nhặt xác con để tránh Zombie
        printf("[CHA] Con đã chết bình an. Tiến trình cha kết thúc.\n");
    }

    return 0;
}