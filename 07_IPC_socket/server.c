#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SOCKET_PATH "/tmp/chat_socket"
#define BUFFER_SIZE 256

int main() {
    int server_fd, client_fd;
    struct sockaddr_un server_addr;
    char buffer[BUFFER_SIZE];

    // 1. Tạo Stream Socket với domain AF_UNIX (Local)
    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Không thể tạo socket");
        exit(EXIT_FAILURE);
    }

    // Xóa file socket cũ nếu có để tránh lỗi trùng lặp bận cổng
    unlink(SOCKET_PATH);

    // 2. Cấu hình địa chỉ "điểm hẹn" cho Server
    memset(&server_addr, 0, sizeof(struct sockaddr_un));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    // Bind socket vào file đường dẫn
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_un)) == -1) {
        perror("Bind thất bại");
        exit(EXIT_FAILURE);
    }

    // 3. Chuyển sang chế độ lắng nghe (Listen)
    if (listen(server_fd, 5) == -1) {
        perror("Listen thất bại");
        exit(EXIT_FAILURE);
    }

    printf("Server: Đang đợi Client kết nối...\n");

    // 4. Chấp nhận kết nối từ Client (Hàm này sẽ chặn cho tới khi có Client vào)
    client_fd = accept(server_fd, NULL, NULL);
    if (client_fd == -1) {
        perror("Accept thất bại");
        exit(EXIT_FAILURE);
    }
    printf("Server: Kết nối thành công! Bắt đầu chat.\n");
    printf("----------------------------------------\n");

    // 5. Vòng lặp Chat thay phiên (Server nhận trước -> Gửi sau)
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        
        // Đọc tin nhắn từ Client
        int bytes_read = read(client_fd, buffer, BUFFER_SIZE - 1);
        if (bytes_read <= 0) { // Nếu Client ngắt kết nối (EOF)
            printf("Client đã rời phòng chat.\n");
            break;
        }
        printf("Client: %s", buffer);

        // Chuẩn bị gửi lại tin nhắn cho Client
        printf("Server (Bạn): ");
        fgets(buffer, BUFFER_SIZE, stdin);
        
        write(client_fd, buffer, strlen(buffer));
        
        if (strncmp(buffer, "exit", 4) == 0) {
            printf("Bạn đã đóng phòng chat.\n");
            break;
        }
    }

    // 6. Dọn dẹp giải phóng socket
    close(client_fd);
    close(server_fd);
    unlink(SOCKET_PATH);
    return 0;
}