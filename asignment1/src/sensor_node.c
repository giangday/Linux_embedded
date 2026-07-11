#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>
#include "sbuffer.h"   // dùng chung định nghĩa sensor_data_t với server

#define DEFAULT_INTERVAL 5

static volatile int running = 1;

// Sinh nhiệt độ giả lập, thỉnh thoảng ra giá trị cực đoan để test cảnh báo
static double generate_temperature(void) {
    int roll = rand() % 100;

    if (roll < 10) {
        // 10% khả năng: quá lạnh (5.0 - 12.0)
        return 5.0 + (rand() % 700) / 100.0;
    } else if (roll < 20) {
        // 10% khả năng: quá nóng (30.0 - 40.0)
        return 30.0 + (rand() % 1000) / 100.0;
    } else {
        // 80% khả năng: bình thường (18.0 - 26.0)
        return 18.0 + (rand() % 800) / 100.0;
    }
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <sensorID> <server_IP> <port> [interval_seconds]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int sensor_id = (int)atoi(argv[1]);
    const char *server_ip = argv[2];
    int port = atoi(argv[3]);
    int interval = (argc >= 5) ? atoi(argv[4]) : DEFAULT_INTERVAL;

    if (sensor_id == 0 || port <= 0) {
        fprintf(stderr, "Invalid sensorID or port\n");
        exit(EXIT_FAILURE);
    }

    srand(time(NULL) ^ getpid());

    // --- Tạo socket TCP ---
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        fprintf(stderr, "Invalid server IP address: %s\n", server_ip);
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Sensor node %u connected to %s:%d (interval = %ds)\n",
           sensor_id, server_ip, port, interval);

    // --- Vòng lặp gửi dữ liệu định kỳ ---
    while (running) {
        sensor_data_t data;
        data.sensor_id = sensor_id;
        data.temperature = generate_temperature();
        data.timestamp = time(NULL);

        ssize_t sent = send(sockfd, &data, sizeof(sensor_data_t), 0);

        if (sent < 0) {
            perror("send");
            break; // server đã đóng kết nối hoặc lỗi -> thoát
        } else if (sent != sizeof(sensor_data_t)) {
            fprintf(stderr, "Partial send (%zd/%zu bytes)\n", sent, sizeof(sensor_data_t));
        } else {
            printf("Sent -> id=%u, value=%.2f, ts=%ld\n",
                   data.sensor_id, data.temperature, (long)data.timestamp);
        }

        sleep(interval);
    }

    close(sockfd);
    printf("Sensor node %u disconnected.\n", sensor_id);
    return 0;
}