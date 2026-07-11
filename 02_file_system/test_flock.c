#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>

int main(void) {
    int fd = open("text.txt", O_WRONLY | O_CREAT, 0644);
    if (fd == -1) {
        perror("Open lỗi");
        exit(1);
    }

    printf("Đang xin quyền khóa file độc quyền (Exclusive Lock)...\n");
    
    // Hàm này sẽ treo tiến trình tại đây nếu file đang bị tiến trình khác khóa
    if (flock(fd, LOCK_EX) == -1) {
        perror("Khóa lỗi");
        exit(1);
    }

    printf("--> Đã lấy được khóa thành công! Đang ghi dữ liệu an toàn...\n");
    write(fd, "Dữ liệu quan trọng\n", 20);
    
    // Giả lập xử lý tốn thời gian (ví dụ 5 giây)
    sleep(5); 

    printf("Đang giải phóng khóa...\n");
    flock(fd, LOCK_UN); // Mở khóa cho các tiến trình khác vào

    close(fd);
    return 0;
}