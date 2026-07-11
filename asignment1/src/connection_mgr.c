#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <stdarg.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "connection_mgr.h"
#include "sbuffer.h"
#include "logger.h"
// static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
static int server_socket_fd = -1; // dùng để có thể ép đóng từ bên ngoài khi shutdown


typedef struct {
    int client_fd;
    sbuffer_t *buffer;
} worker_args_t;

// Đảm bảo đọc ĐỦ count byte từ TCP stream, xử lý trường hợp short read
static ssize_t read_all(int fd, void *buf, size_t count) {
    size_t bytes_read = 0;
    char *ptr = (char *)buf;

    while (bytes_read < count) {
        ssize_t n = read(fd, ptr + bytes_read, count - bytes_read);
        if (n == 0) return (ssize_t)bytes_read; // client đóng kết nối giữa chừng
        if (n < 0) {
            if (errno == EINTR) continue;
            return -1; // lỗi thật sự
        }
        bytes_read += (size_t)n;
    }
    return (ssize_t)bytes_read;
}

static int create_listen_socket(int port) {
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        perror("socket");
        return -1;
    }

    int opt = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(listenfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(listenfd);
        return -1;
    }

    if (listen(listenfd, 10) < 0) {
        perror("listen");
        close(listenfd);
        return -1;
    }

    return listenfd;
}

// --- Luồng phụ: đọc dữ liệu liên tục từ 1 sensor cho tới khi ngắt kết nối ---
static void *sensor_worker_thread(void *arg) {
    worker_args_t *wargs = (worker_args_t *)arg;
    int fd = wargs->client_fd;
    sbuffer_t *buffer = wargs->buffer;
    free(wargs); // đã lấy hết thông tin cần, giải phóng ngay

    int id_known = 0;
    int current_sensor_id = 0;

    while (1) {
        sensor_data_t data;
        ssize_t n = read_all(fd, &data, sizeof(sensor_data_t));

        if (n <= 0) {
            // n == 0: client đóng kết nối bình thường
            // n < 0 : lỗi đọc thật sự
            if (n < 0) perror("read_all");
            break;
        }

        if ((size_t)n != sizeof(sensor_data_t)) {
            // Trên lý thuyết read_all chỉ trả về đủ count hoặc ít hơn khi client đóng giữa chừng,
            // nhưng vẫn kiểm tra tường minh cho chắc chắn.
            fprintf(stderr, "Received incomplete packet, dropping connection\n");
            break;
        }

        // Gói tin đầu tiên -> giờ mới biết ID của sensor này
        if (!id_known) {
            current_sensor_id = data.sensor_id;
            id_known = 1;
            log_event("A sensor node with %d has opened a new connection.", current_sensor_id);
        }

        if (sbuffer_insert(buffer, &data) != SBUFFER_SUCCESS) {
            fprintf(stderr, "Failed to insert data into sbuffer\n");
        }
    }

    if (id_known) {
        log_event("The sensor node with %d has closed the connection.", current_sensor_id);
    }

    close(fd);
    return NULL;
}

// --- Luồng chính: accept vô hạn, spawn 1 thread cho mỗi kết nối mới ---
void *connection_manager_main(void *arg) {
    conn_mgr_args_t *args = (conn_mgr_args_t *)arg;
    int port = args->port;
    sbuffer_t *buffer = args->buffer;

    int listenfd = create_listen_socket(port);
    if (listenfd < 0) {
        fprintf(stderr, "Connection manager: cannot start listening on port %d\n", port);
        return NULL;
    }
    server_socket_fd = listenfd;

    printf("Connection manager listening on port %d...\n", port);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t addrlen = sizeof(client_addr);

        int client_fd = accept(listenfd, (struct sockaddr *)&client_addr, &addrlen);
        if (client_fd < 0) {
            if (errno == EINTR) continue;
            if (errno == EBADF || errno == EINVAL) {
                // listenfd đã bị đóng chủ động từ connection_mgr_force_close()
                printf("Connection manager: listening socket closed, shutting down.\n");
                break;
            }
            perror("accept");
            continue; // lỗi accept đơn lẻ không nên làm chết cả server
        }

        worker_args_t *wargs = malloc(sizeof(worker_args_t));
        if (wargs == NULL) {
            fprintf(stderr, "malloc failed, rejecting connection\n");
            close(client_fd);
            continue;
        }
        wargs->client_fd = client_fd;
        wargs->buffer = buffer;

        pthread_t tid;
        if (pthread_create(&tid, NULL, sensor_worker_thread, wargs) != 0) {
            perror("pthread_create");
            free(wargs);
            close(client_fd);
            continue;
        }

        pthread_detach(tid);
    }

    return NULL;
}

// Gọi từ thread khác (ví dụ storage_mgr khi cần shutdown toàn hệ thống)
void connection_mgr_force_close(void) {
    if (server_socket_fd != -1) {
        int fd = server_socket_fd;
        server_socket_fd = -1;
        close(fd); // làm accept() đang block ở luồng chính trả về lỗi -> vòng lặp thoát
    }
}