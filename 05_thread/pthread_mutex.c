#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

long long bank_balance = 0; // Biến toàn cục dùng chung (Tài khoản ngân hàng)
pthread_mutex_t lock;       // Khai báo ổ khóa Mutex

void* deposit_money(void* arg) {
    for (int i = 0; i < 1000000; i++) {
        // --- BẮT ĐẦU VÙNG TỚI HẠN (CRITICAL SECTION) ---
        // pthread_mutex_lock(&lock); // Khóa cửa lại! Ông khác tới phải đợi.
        
        bank_balance++; // Chỉ có 1 luồng duy nhất được thực hiện dòng này tại 1 thời điểm
        
        // pthread_mutex_unlock(&lock); // Mở khóa cho ông khác vào.
        // --- KẾT THÚC VÙNG TỚI HẠN ---
    }
    return NULL;
}

int main() {
    pthread_t thread1, thread2;

    // Khởi tạo ổ khóa Mutex trước khi dùng
    pthread_mutex_init(&lock, NULL);

    // Tạo 2 luồng cùng thực hiện hàm nạp tiền
    pthread_create(&thread1, NULL, deposit_money, NULL);
    pthread_create(&thread2, NULL, deposit_money, NULL);

    // Đợi cả 2 luồng chạy xong
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    // Hủy khóa sau khi dùng xong để giải phóng tài nguyên
    pthread_mutex_destroy(&lock);

    printf("Số dư tài khoản cuối cùng: %lld\n", bank_balance);
    printf("(Kết quả mong đợi: 2000000)\n");

    return 0;
}