#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pthread.h>
#include <stdarg.h>
#include <time.h>
#include <errno.h>
#include "logger.h"

static int fifo_write_fd = -1;
static pthread_mutex_t fifo_mutex = PTHREAD_MUTEX_INITIALIZER;

int logger_init(void) {
    // Tạo FIFO nếu chưa tồn tại (bỏ qua lỗi nếu đã tồn tại từ lần chạy trước)
    if (mkfifo(LOG_FIFO_NAME, 0666) < 0 && errno != EEXIST) {
        perror("mkfifo");
        return -1;
    }

    // Mở FIFO ở chế độ ghi. Lưu ý: open() để ghi sẽ BLOCK cho tới khi có
    // ít nhất 1 bên đọc mở FIFO -> vì vậy phải fork() Log Process (bên đọc)
    // trước khi gọi logger_init(), nếu không cả chương trình sẽ treo ở đây.
    fifo_write_fd = open(LOG_FIFO_NAME, O_WRONLY);
    if (fifo_write_fd < 0) {
        perror("open logFifo for writing");
        return -1;
    }

    return 0;
}

void log_event(const char *fmt, ...) {
    if (fifo_write_fd < 0) return; // logger chưa init hoặc đã đóng -> bỏ qua

    char message[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(message, sizeof(message), fmt, args);
    va_end(args);

    pthread_mutex_lock(&fifo_mutex);
    ssize_t n = write(fifo_write_fd, message, strlen(message) + 1); // +1 để gửi kèm '\0'
    if (n < 0) {
        perror("write to logFifo");
    }
    pthread_mutex_unlock(&fifo_mutex);
}

void logger_close(void) {
    pthread_mutex_lock(&fifo_mutex);
    if (fifo_write_fd != -1) {
        close(fifo_write_fd);
        fifo_write_fd = -1;
    }
    pthread_mutex_unlock(&fifo_mutex);
}

// --- Log Process: đọc từ FIFO, ghi ra file với format <seq> <timestamp> <message> ---
void log_process_run(const char *log_filename) {
    int fifo_read_fd = open(LOG_FIFO_NAME, O_RDONLY);
    if (fifo_read_fd < 0) {
        perror("open logFifo for reading");
        exit(EXIT_FAILURE);
    }

    FILE *logfile = fopen(log_filename, "a");
    if (logfile == NULL) {
        perror("fopen gateway.log");
        close(fifo_read_fd);
        exit(EXIT_FAILURE);
    }

    long sequence = 1;
    char buffer[1024];
    char msg[1024];
    size_t msg_len = 0;

    // Đọc theo từng byte để tách đúng ranh giới message (kết thúc bằng '\0')
    // Cách đơn giản, an toàn cho message ngắn; đủ dùng cho quy mô bài tập.
    while (1) {
        ssize_t n = read(fifo_read_fd, buffer, sizeof(buffer));

        if (n == 0) {
            // Tất cả các đầu ghi (write end) đã đóng -> Main Process đã kết thúc
            break;
        }
        if (n < 0) {
            if (errno == EINTR) continue;
            perror("read from logFifo");
            break;
        }

        for (ssize_t i = 0; i < n; i++) {
            if (buffer[i] == '\0') {
                // Đủ 1 message hoàn chỉnh -> ghi ra file
                msg[msg_len] = '\0';

                time_t now = time(NULL);
                struct tm *tm_info = localtime(&now);
                char timestr[32];
                strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S", tm_info);

                fprintf(logfile, "%ld %s %s\n", sequence, timestr, msg);
                fflush(logfile); // ghi ngay xuống đĩa, tránh mất log nếu process bị kill đột ngột

                sequence++;
                msg_len = 0;
            } else {
                if (msg_len < sizeof(msg) - 1) {
                    msg[msg_len++] = buffer[i];
                }
            }
        }
    }

    fclose(logfile);
    close(fifo_read_fd);
}