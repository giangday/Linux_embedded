#ifndef _LOGGER_H_
#define _LOGGER_H_

#define LOG_FIFO_NAME "logFifo"

// Gọi 1 lần ở Main Process (cha) trước khi fork() và trước khi tạo 3 thread,
// để mở FIFO ở chế độ ghi. Trả về 0 nếu thành công, -1 nếu lỗi.
int logger_init(void);

// Cả 3 thread (Connection/Data/Storage Manager) gọi hàm này để ghi log.
// Thread-safe: có mutex bảo vệ bên trong.
void log_event(const char *fmt, ...);

// Đóng FIFO ghi, gọi khi Main Process chuẩn bị kết thúc.
void logger_close(void);

// --- Dành riêng cho Log Process (tiến trình con) ---
// Chạy vòng lặp đọc từ FIFO và ghi ra file log_filename cho tới khi FIFO bị đóng hết đầu ghi.
void log_process_run(const char *log_filename);

#endif