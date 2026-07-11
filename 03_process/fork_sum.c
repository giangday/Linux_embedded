#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>

#define LIMIT 100000000

// Hàm tính tổng trong một khoảng [start, end]
long long calculate_sum(int start, int end) {
    long long sum = 0;
    for (int i = start; i <= end; i++) {
        sum += i;
    }
    return sum;
}

int main() {
    struct timeval start_time, end_time;
    long long total_sum = 0;
    double elapsed_single, elapsed_multi;

    // =================================================================
    // CÁCH 1: CHỈ CHẠY 1 PROCESS (TUẦN TỰ)
    // =================================================================
    gettimeofday(&start_time, NULL);
    
    total_sum = calculate_sum(0, LIMIT);
    
    gettimeofday(&end_time, NULL);
    elapsed_single = (end_time.tv_sec - start_time.tv_sec) * 1000.0 + 
                     (end_time.tv_usec - start_time.tv_usec) / 1000.0;
                     
    printf("[1 PROCESS] Tổng: %lld | Thời gian: %.3f ms\n", total_sum, elapsed_single);
    printf("--------------------------------------------------\n");

    // =================================================================
    // CÁCH 2: TẠO 2 PROCESS (CHA VÀ CON)
    // =================================================================
    // Vì 2 process có bộ nhớ độc lập, ta cần một cơ chế để con truyền kết quả về cho cha.
    // Cách đơn giản nhất ở đây là con trả kết quả thông qua mã `exit(status)` 
    // (Lưu ý: status chỉ nhận tối đa giá trị từ 0-255, nhưng ở đây ta chia nhỏ giới hạn hoặc chấp nhận xử lý)
    // Để tránh giới hạn của exit status (8-bit), ta dùng một tính năng nâng cao hơn một chút: Pipe (Đường ống)
    
    int fd[2]; // fd[0]: đầu đọc, fd[1]: đầu ghi
    if (pipe(fd) == -1) {
        perror("Pipe thất bại");
        return 1;
    }

    gettimeofday(&start_time, NULL);

    pid_t pid = fork();

    if (pid < 0) {
        perror("Fork thất bại");
        return 1;
    } 
    else if (pid == 0) {
        // --- PROCESS CON: Tính nửa đầu [0, LIMIT/2] ---
        close(fd[0]); // Con không đọc, đóng đầu đọc
        
        long long child_sum = calculate_sum(0, LIMIT / 2);
        
        // Ghi kết quả vào pipe gửi cho cha
        write(fd[1], &child_sum, sizeof(child_sum));
        close(fd[1]);
        exit(0);
    } 
    else {
        // --- PROCESS CHA: Tính nửa sau [LIMIT/2 + 1, LIMIT] ---
        close(fd[1]); // Cha không ghi, đóng đầu ghi
        
        long long parent_part_sum = calculate_sum((LIMIT / 2) + 1, LIMIT);
        
        long long child_part_sum = 0;
        // Đọc kết quả từ con thông qua pipe
        read(fd[0], &child_part_sum, sizeof(child_part_sum));
        close(fd[0]);
        
        // Đợi con giải phóng hoàn toàn
        wait(NULL); 
        
        // Cộng tổng 2 nửa
        long long total_multi_sum = parent_part_sum + child_part_sum;
        
        gettimeofday(&end_time, NULL);
        elapsed_multi = (end_time.tv_sec - start_time.tv_sec) * 1000.0 + 
                        (end_time.tv_usec - start_time.tv_usec) / 1000.0;
                        
        printf("[2 PROCESS] Tổng: %lld | Thời gian: %.3f ms\n", total_multi_sum, elapsed_multi);
    }

    return 0;
}