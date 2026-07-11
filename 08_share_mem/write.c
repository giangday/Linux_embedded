#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

int main() {
    const int SIZE = 4096; // Kích thước vùng nhớ (4KB)
    const char *name = "/shm_example"; // Tên Object
    const char *message = "Xin chào, dữ liệu này nằm hoàn toàn trên RAM chia sẻ!";

    int shm_fd;
    void *ptr;

    // 1. Tạo Shared Memory Object
    shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);

    // 2. Thiết lập kích thước vùng nhớ
    ftruncate(shm_fd, SIZE);

    // 3. Ánh xạ vùng nhớ vào con trỏ ptr của tiến trình
    ptr = mmap(0, SIZE, PROT_WRITE, MAP_SHARED, shm_fd, 0);

    // 4. Ghi dữ liệu vào vùng nhớ qua con trỏ
    sprintf(ptr, "%s", message);
    printf("Writer: Đã ghi dữ liệu lên RAM.\n");

    // Thao tác xong thì đóng (vùng nhớ vẫn tồn tại trên RAM)
    close(shm_fd);
    return 0;
}