#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include "sbuffer.h"
#include "connection_mgr.h"
#include "data_mgr.h"
#include "storage_mgr.h"
#include "logger.h"

#define GATEWAY_LOG_FILE "log/gateway.log"
#define DB_FILE          "sensor_data.db"

// Cờ toàn cục để signal handler báo hiệu cần shutdown
static volatile sig_atomic_t g_shutdown_requested = 0;
static sbuffer_t *g_buffer = NULL; // để signal handler có thể gọi sbuffer_shutdown()

static void sigint_handler(int signum) {
    (void)signum;
    g_shutdown_requested = 1;
    // Đánh thức data_mgr/storage_mgr đang chờ trong sbuffer
    if (g_buffer != NULL) {
        sbuffer_shutdown(g_buffer);
    }
    // Phá vỡ accept() đang block ở connection_mgr
    connection_mgr_force_close();
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    if (port <= 0) {
        fprintf(stderr, "Invalid port: %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    // --- Khởi tạo sbuffer ---
    sbuffer_t *buffer = NULL;
    if (sbuffer_init(&buffer) != SBUFFER_SUCCESS) {
        fprintf(stderr, "Failed to initialize sbuffer\n");
        exit(EXIT_FAILURE);
    }
    g_buffer = buffer;

    // --- Tạo FIFO trước khi fork() ---
    // (logger_init() sẽ tự mkfifo nếu chưa có, nhưng ta cần chắc chắn FIFO
    //  tồn tại trước khi fork() để cả 2 tiến trình đều thấy cùng 1 file FIFO)

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        sbuffer_free(&buffer);
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        // ================= LOG PROCESS (con) =================
        // Mở FIFO ở chế độ đọc và chạy vòng lặp ghi ra gateway.log
        // cho tới khi Main Process đóng hết đầu ghi (EOF).
        log_process_run(GATEWAY_LOG_FILE);
        exit(EXIT_SUCCESS);
    }

    // ================= MAIN PROCESS (cha) =================

    // Đăng ký signal handler để Ctrl+C dừng chương trình sạch sẽ
    // thay vì kill đột ngột (mất log, DB chưa flush, v.v.)
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    // Mở FIFO để ghi (BLOCK cho tới khi Log Process mở đầu đọc -> luôn xảy ra
    // sau fork() ở trên nên không bị treo)
    if (logger_init() < 0) {
        fprintf(stderr, "Main process: failed to init logger\n");
        sbuffer_free(&buffer);
        exit(EXIT_FAILURE);
    }

    printf("Sensor Gateway starting on port %d (PID=%d, LogProcess PID=%d)\n",
           port, getpid(), pid);

    // --- Chuẩn bị tham số cho 3 thread ---
    conn_mgr_args_t conn_args = { .port = port, .buffer = buffer };
    data_mgr_args_t data_args = { .buffer = buffer };
    storage_mgr_args_t storage_args = { .buffer = buffer, .db_filename = DB_FILE };

    pthread_t conn_tid, data_tid, storage_tid;

    if (pthread_create(&conn_tid, NULL, connection_manager_main, &conn_args) != 0) {
        perror("pthread_create connection_mgr");
        goto cleanup;
    }
    if (pthread_create(&data_tid, NULL, data_manager_main, &data_args) != 0) {
        perror("pthread_create data_mgr");
        goto cleanup;
    }
    if (pthread_create(&storage_tid, NULL, storage_manager_main, &storage_args) != 0) {
        perror("pthread_create storage_mgr");
        goto cleanup;
    }

    // --- Đợi cả 3 thread kết thúc ---
    // (chỉ xảy ra khi: nhận SIGINT, hoặc storage_mgr lỗi DB quá 3 lần và tự shutdown)
    pthread_join(storage_tid, NULL);
    pthread_join(data_tid, NULL);

    // connection_manager_main() có thể vẫn đang block ở accept() nếu chưa ai
    // gọi connection_mgr_force_close() -> đảm bảo gọi trước khi join nó
    connection_mgr_force_close();
    pthread_join(conn_tid, NULL);

cleanup:
    printf("Main process: all threads joined, shutting down.\n");

    logger_close();       // đóng đầu ghi FIFO -> Log Process nhận EOF và tự thoát
    waitpid(pid, NULL, 0); // đợi Log Process (con) kết thúc hẳn, tránh zombie

    sbuffer_free(&buffer);
    g_buffer = NULL;

    printf("Sensor Gateway shut down cleanly.\n");
    return 0;
}