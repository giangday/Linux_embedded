#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

#define LIMIT 1000000000LL  // 1 Tỷ

// Cấu trúc dữ liệu để truyền tham số vào từng Thread
typedef struct {
    long long start;
    long long end;
    long long partial_sum;
} ThreadData;

// Hàm tính tổng tuần tự cho một khoảng [start, end]
long long calculate_sum(long long start, long long end) {
    long long sum = 0;
    for (long long i = start; i <= end; i++) {
        sum += i;
    }
    return sum;
}

// Hàm Wrapper để Thread gọi thực thi
void* thread_worker(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    data->partial_sum = calculate_sum(data->start, data->end);
    return NULL;
}

int main() {
    struct timeval start_time, end_time;
    double elapsed_single, elapsed_multi;
    long long total_sum = 0;

    // =================================================================
    // CÁCH 1: CHỈ CHẠY 1 THREAD TRÊN 1 PROCESS (TUẦN TỰ)
    // =================================================================
    printf("[HỆ THỐNG] Đang tính toán tuần tự với 1 Thread...\n");
    gettimeofday(&start_time, NULL);

    total_sum = calculate_sum(0, LIMIT);

    gettimeofday(&end_time, NULL);
    elapsed_single = (end_time.tv_sec - start_time.tv_sec) * 1000.0 + 
                     (end_time.tv_usec - start_time.tv_usec) / 1000.0;
                     
    printf("[1 THREAD]  Tổng: %lld | Thời gian: %.2f ms\n", total_sum, elapsed_single);
    printf("--------------------------------------------------\n");


    // =================================================================
    // CÁCH 2: CHẠY ĐA LUỒNG (2 THREADS SONG SONG)
    // =================================================================
    printf("[HỆ THỐNG] Đang tính toán song song với 2 Threads...\n");
    gettimeofday(&start_time, NULL);

    pthread_t thread1, thread2;
    ThreadData data1, data2;

    // Chia bài toán làm 2 nửa độc lập để tránh Race Condition (Không dùng chung biến)
    data1.start = 0;
    data1.end = LIMIT / 2;
    data1.partial_sum = 0;

    data2.start = (LIMIT / 2) + 1;
    data2.end = LIMIT;
    data2.partial_sum = 0;

    // Khởi tạo và kích hoạt 2 luồng phụ chạy song song trên các nhân CPU
    pthread_create(&thread1, NULL, thread_worker, &data1);
    pthread_create(&thread2, NULL, thread_worker, &data2);

    // Ép hàm main phải đợi cả 2 luồng hoàn thành nhiệm vụ
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    // Thu gom kết quả từ 2 luồng phụ
    long long total_multi_sum = data1.partial_sum + data2.partial_sum;

    gettimeofday(&end_time, NULL);
    elapsed_multi = (end_time.tv_sec - start_time.tv_sec) * 1000.0 + 
                    (end_time.tv_usec - start_time.tv_usec) / 1000.0;

    printf("[2 THREADS] Tổng: %lld | Thời gian: %.2f ms\n", total_multi_sum, elapsed_multi);
    printf("--------------------------------------------------\n");

    // =================================================================
    // ĐÁNH GIÁ HIỆU NĂNG
    // =================================================================
    printf("[KẾT LUẬN] Đa luồng nhanh hơn tuần tự: %.2fx lần\n", elapsed_single / elapsed_multi);

    return 0;
}