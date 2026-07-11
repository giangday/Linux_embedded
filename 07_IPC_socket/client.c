#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SOCKET_PATH "/tmp/chat_socket"
#define BUFFER_SIZE 256

int main() {
    int client_fd;
    struct sockaddr_un server_addr;
    char buffer[BUFFER_SIZE];

    // 1. Tạo Stream Socket
    client_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_fd == -1) {
        perror("Không thể tạo socket");
        exit(EXIT_FAILURE);
    }

    // 2. Cấu hình địa chỉ đích (Server) để kết nối
    memset(&server_addr, 0, sizeof(struct sockaddr_un));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    printf("Client: Đang kết nối tới Server...\n");
    
    // 3. Kết nối tới Server
    if (connect(client_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_un)) == -1) {
        perror("Kết nối thất bại. Hãy chắc chắn Server đang chạy!");
        exit(EXIT_FAILURE);
    }
    printf("Client: Kết nối thành công! Bắt đầu chat.\n");
    printf("----------------------------------------\n");

    // 4. Vòng lặp Chat thay phiên (Client gửi trước -> Nhận sau)
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);

        // Nhập tin nhắn gửi đi
        printf("Client (Bạn): ");
        fgets(buffer, BUFFER_SIZE, stdin);
        
        write(client_fd, buffer, strlen(buffer));

        if (strncmp(buffer, "exit", 4) == 0) {
            printf("Bạn đã rời phòng chat.\n");
            break;
        }

        // Đợi nhận tin nhắn phản hồi từ Server
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_read = read(client_fd, buffer, BUFFER_SIZE - 1);
        if (bytes_read <= 0) {
            printf("Server đã đóng kết nối.\n");
            break;
        }
        printf("Server: %s", buffer);
    }

    // 5. Đóng socket
    close(client_fd);
    return 0;
}