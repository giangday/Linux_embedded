#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int main(void) {
    // 1. Mở file để đọc và ghi
    int fd = open("test_fcntl.txt", O_RDWR | O_CREAT, 0644);
    if (fd == -1) {
        perror("Không thể mở file");
        exit(1);
    }

    // 2. Cấu hình struct flock để khóa 50 byte đầu tiên
    struct flock lock;
    lock.l_type = F_WRLCK;    // Xin khóa GHI độc quyền
    lock.l_whence = SEEK_SET; // Mốc tính từ ĐẦU FILE
    lock.l_start = 0;         // Bắt đầu từ byte số 0
    lock.l_len = 50;          // Khóa đúng 50 byte

    printf("Đang xin quyền khóa 50 byte đầu tiên bằng fcntl()...\n");

    // 3. Gọi fcntl với lệnh F_SETLKW (Set Lock Wait)
    // Lệnh này sẽ treo tiến trình để đợi nếu phân vùng này đang bị đứa khác khóa
    if (fcntl(fd, F_SETLKW, &lock) == -1) {
        perror("Cài đặt khóa thất bại");
        close(fd);
        exit(1);
    }

    printf("--> Đã chiếm giữ khóa 50 byte thành công!\n");
    
    // Ghi thử dữ liệu vào vùng được bảo vệ
    write(fd, "Giang đang test fcntl locking...\n", 33);

    // Giữ khóa trong 10 giây để bạn có thời gian mở Terminal khác lên test
    printf("Chương trình sẽ giữ khóa trong 10 giây, chuẩn bị giải phóng...\n");
    sleep(10);

    // 4. Mở khóa (Unlock)
    printf("Đang giải phóng khóa...\n");
    lock.l_type = F_UNLCK; // Đổi loại khóa thành Unlock
    if (fcntl(fd, F_SETLK, &lock) == -1) {
        perror("Giải phóng khóa thất bại");
        close(fd);
        exit(1);
    }

    printf("--> Đã mở khóa hoàn toàn. Kết thúc!\n");
    close(fd);
    return 0;
}