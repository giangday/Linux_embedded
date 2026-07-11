#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

void handle_sigint(int sig) {
    printf("\n[HANDLER] Đã nhận và xử lý tín hiệu SIGINT (%d)!\n", sig);
}

int main() {
    // Đăng ký handler cho SIGINT như bình thường
    signal(SIGINT, handle_sigint);
    printf("PID của tiến trình: %d\n", getpid());

    sigset_t new_mask, old_mask;

    // 1. Khởi tạo một tập hợp tín hiệu trống và thêm SIGINT vào đó
    sigemptyset(&new_mask);
    sigaddset(&new_mask, SIGINT);

    // 2. BLOCK: Thêm tập hợp new_mask vào bảng chặn của hệ thống
    // old_mask dùng để lưu lại cấu hình cũ trước khi chặn nhằm khôi phục sau này
    printf("[HỆ THỐNG] ---> Bắt đầu BLOCK tín hiệu SIGINT trong 5 giây.\n");
    printf("[HỆ THỐNG] Lúc này nếu bạn bấm Ctrl+C, chương trình sẽ KHÔNG phản ứng ngay.\n");
    sigprocmask(SIG_BLOCK, &new_mask, &old_mask);

    // Giả lập vùng làm việc quan trọng (Critical Section)
    for (int i = 1; i <= 5; i++) {
        printf("  Main đang làm việc quan trọng giây thứ %d...\n", i);
        sleep(1);
    }

    // 3. UNBLOCK: Khôi phục lại trạng thái ban đầu (gỡ bỏ block)
    printf("[HỆ THỐNG] ---> Kết thúc 5 giây. Thực hiện UNBLOCK SIGINT.\n");
    printf("[HỆ THỐNG] Nếu bạn có bấm Ctrl+C trong 5 giây qua, nó sẽ kích hoạt NGAY BÂY GIỜ.\n");
    
    // Cách 1: Khôi phục lại mask cũ
    sigprocmask(SIG_SETMASK, &old_mask, NULL); 
    
    // Cách 2 (Nếu không dùng old_mask): sigprocmask(SIG_UNBLOCK, &new_mask, NULL);

    // Chạy tiếp vòng lặp bình thường để kiểm tra sau khi unblock
    printf("\n[HỆ THỐNG] Trở lại trạng thái bình thường. Bấm Ctrl+C lúc này sẽ xử lý luôn.\n");
    while(1) {
        sleep(1);
    }

    return 0;
}