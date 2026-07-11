#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

int main() {
    const int SIZE = 4096;
    const char *name = "/shm_example";

    int shm_fd;
    void *ptr;

    // 1. Mở Shared Memory Object đã có sẵn
    shm_fd = shm_open(name, O_RDONLY, 0666);

    // 2. Ánh xạ vào vùng nhớ của Reader (Chỉ Đọc)
    ptr = mmap(0, SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);

    // 3. Đọc dữ liệu trực tiếp từ RAM
    printf("Reader nhận được: %s\n", (char *)ptr);

    // 4. Dọn dẹp và xóa hoàn toàn Object khỏi hệ thống RAM
    munmap(ptr, SIZE);
    close(shm_fd);
    shm_unlink(name); 

    return 0;
}