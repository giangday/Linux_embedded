// #include <stdio.h>
// #include <stdlib.h>
// #include <unistd.h>
// #include <signal.h>

// // Hàm xử lý chung cho tất cả các tín hiệu bắt được
// void universal_signal_handler(int sig) {
//     printf("\n[NHẬN ĐƯỢC] -> Tiến trình vừa nhận Signal số: %d\n", sig);
    
//     // In thêm giải thích cho một số signal phổ biến để bạn dễ theo dõi
//     switch(sig) {
//         case 1:  printf("=> SIGHUP: Thường dùng để yêu cầu reload cấu hình.\n"); break;
//         case 2:  printf("=> SIGINT: Bạn vừa bấm Ctrl+C hoặc dùng 'kill -2'.\n"); break;
//         case 3:  printf("=> SIGQUIT: Phím tắt Ctrl+\\ hoặc 'kill -3'.\n"); break;
//         case 10: printf("=> SIGUSR1: Tín hiệu định nghĩa riêng số 1.\n"); break;
//         case 12: printf("=> SIGUSR2: Tín hiệu định nghĩa riêng số 2.\n"); break;
//         case 15: printf("=> SIGTERM: Lệnh kill mặc định (yêu cầu tắt lịch sự).\n"); break;
//         default: printf("=> Bạn vừa test một signal khác trong danh sách!\n"); break;
//     }
// }

// int main() {
//     // In ra PID để bạn lấy số này test lệnh kill
//     printf("==================================================\n");
//     printf(" TIẾN TRÌNH THỬ NGHIỆM SIGNAL ĐANG CHẠY...\n");
//     printf(" PID CỦA TIẾN TRÌNH NÀY LÀ: %d\n", getpid());
//     printf("==================================================\n");
//     printf("Hướng dẫn: Mở một Terminal khác và gõ:\n");
//     printf("  kill -<số_signal> %d\n", getpid());
//     printf("Ví dụ: kill -2 %d hoặc kill -10 %d\n", getpid(), getpid());
//     printf("--------------------------------------------------\n\n");

//     // Đăng ký bắt một loạt các signal phổ biến từ 1 đến 31
//     // (Bỏ qua các số hệ thống không cho phép hoặc gây crash cứng)
//     for (int i = 1; i <= 31; i++) {
//         if (i != SIGKILL && i != SIGSTOP && i != 11 && i != 4 && i != 7 && i != 8) {
//             signal(i, universal_signal_handler);
//         }
//     }

//     int count = 1;
//     // Vòng lặp while(1) vô hạn để giữ tiến trình luôn sống
//     while(1) {
//         printf("[Giây %d] Đang đợi bạn gửi signal...\n", count++);
//         sleep(1);
//     }

//     return 0;
// }











#include <stdio.h>
#include <unistd.h>
#include <signal.h>

void my_handler(int sig) {
    printf("\n--------------------------------------------------\n");
    printf("[HANDLER] Đang xử lý Signal %d... (Hàm main tạm dừng tại đây)\n", sig);
    sleep(2); // Giả lập xử lý mất 2 giây
    printf("[HANDLER] Xử lý xong! Chuẩn bị trả lại quyền cho main...\n");
    printf("--------------------------------------------------\n\n");
}

int main() {
    // Đăng ký signal số 10 (SIGUSR1)
    signal(SIGUSR1, my_handler);

    printf("PID của tôi là: %d. Hãy gửi 'kill -10 %d' để test.\n\n", getpid(), getpid());

    for (int i = 1; i <= 20; i++) {
        printf("Main đang chạy dòng thứ: %d\n", i);
        sleep(1); // Ngủ 1 giây mỗi vòng để bạn kịp bắn signal
    }

    printf("Main đã chạy xong tới 20 và kết thúc bình thường!\n");
    return 0;
}