#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdarg.h>
#include <sqlite3.h>
#include "storage_mgr.h"
#include "sbuffer.h"
#include "connection_mgr.h"  // để gọi connection_mgr_force_close() khi cần shutdown
#include "logger.h"

#define TABLE_NAME       "SensorData"
#define MAX_RETRY        3
#define RETRY_DELAY_SEC  2


// Mở kết nối DB và tạo bảng nếu chưa có. Trả về con trỏ sqlite3* hoặc NULL nếu lỗi.
static sqlite3 *connect_to_db(const char *filename) {
    sqlite3 *db = NULL;
    int rc = sqlite3_open(filename, &db);

    if (rc != SQLITE_OK) {
        log_event("Unable to connect to SQL server.");
        if (db) sqlite3_close(db);
        return NULL;
    }

    log_event("Connection to SQL server established.");

    // Kiểm tra bảng đã tồn tại chưa (để biết có cần log "New table created" không)
    const char *check_sql =
        "SELECT name FROM sqlite_master WHERE type='table' AND name='" TABLE_NAME "';";
    sqlite3_stmt *stmt = NULL;
    int table_exists = 0;

    if (sqlite3_prepare_v2(db, check_sql, -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            table_exists = 1;
        }
        sqlite3_finalize(stmt);
    }

    const char *create_sql =
        "CREATE TABLE IF NOT EXISTS " TABLE_NAME " ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "sensor_id INTEGER NOT NULL, "
        "temperature REAL NOT NULL, "
        "timestamp INTEGER NOT NULL"
        ");";

    char *errmsg = NULL;
    rc = sqlite3_exec(db, create_sql, NULL, NULL, &errmsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to create table: %s\n", errmsg);
        sqlite3_free(errmsg);
        sqlite3_close(db);
        return NULL;
    }

    if (!table_exists) {
        log_event("New table %s created.", TABLE_NAME);
    }

    return db;
}

// Insert 1 bản ghi vào DB. Trả về 1 nếu thành công, 0 nếu thất bại.
static int insert_data(sqlite3 *db, sensor_data_t *data) {
    const char *insert_sql =
        "INSERT INTO " TABLE_NAME " (sensor_id, temperature, timestamp) VALUES (?, ?, ?);";

    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(db, insert_sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "prepare failed: %s\n", sqlite3_errmsg(db));
        return 0;
    }

    sqlite3_bind_int(stmt, 1, data->sensor_id);
    sqlite3_bind_double(stmt, 2, data->temperature);
    sqlite3_bind_int64(stmt, 3, (sqlite3_int64)data->timestamp);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "insert failed: %s\n", sqlite3_errmsg(db));
        return 0;
    }
    return 1;
}

void *storage_manager_main(void *arg) {
    storage_mgr_args_t *args = (storage_mgr_args_t *)arg;
    sbuffer_t *buffer = args->buffer;
    const char *db_filename = args->db_filename;

    sqlite3 *db = connect_to_db(db_filename);
    if (db == NULL) {
        // Không kết nối được ngay từ đầu -> không thể tiếp tục hoạt động
        log_event("Storage manager: initial DB connection failed, shutting down gateway.");
        sbuffer_shutdown(buffer);
        connection_mgr_force_close();
        return NULL;
    }

    printf("Storage manager started.\n");

    int consecutive_failures = 0;
    sensor_data_t pending_data;
    int has_pending = 0; // 1 nếu đang có dữ liệu chưa insert được, cần retry trước khi đọc mới

    while (1) {
        // Nếu không có dữ liệu đang chờ retry -> đọc gói mới từ sbuffer
        if (!has_pending) {
            int ret = sbuffer_read_storagemgr(buffer, &pending_data);

            if (ret == SBUFFER_EMPTY) {
                break; // hệ thống shutdown, không còn dữ liệu -> kết thúc thread
            }
            if (ret != SBUFFER_SUCCESS) {
                fprintf(stderr, "Storage manager: error reading sbuffer\n");
                continue;
            }
            has_pending = 1;
        }

        // Thử insert dữ liệu hiện tại (mới đọc hoặc đang retry)
        if (insert_data(db, &pending_data)) {
            has_pending = 0;          // insert thành công -> sẵn sàng đọc gói tiếp theo
            consecutive_failures = 0; // reset bộ đếm lỗi liên tiếp
        } else {
            consecutive_failures++;
            log_event("Connection to SQL server lost.");

            if (consecutive_failures >= MAX_RETRY) {
                log_event("Storage manager: failed %d consecutive times, shutting down gateway.",
                           consecutive_failures);
                sbuffer_shutdown(buffer);
                connection_mgr_force_close();
                break;
            }

            sleep(RETRY_DELAY_SEC);

            sqlite3_close(db);
            db = connect_to_db(db_filename);
            while (db == NULL && consecutive_failures < MAX_RETRY) {
                consecutive_failures++;
                if (consecutive_failures >= MAX_RETRY) break;
                sleep(RETRY_DELAY_SEC);
                db = connect_to_db(db_filename);
            }

            if (db == NULL) {
                log_event("Storage manager: failed %d consecutive times, shutting down gateway.",
                           consecutive_failures);
                sbuffer_shutdown(buffer);
                connection_mgr_force_close();
                break;
            }
            // db đã kết nối lại thành công -> vòng lặp while(1) sẽ retry insert_data() ở lượt sau
        }
    }

    if (db != NULL) sqlite3_close(db);
    printf("Storage manager shutting down.\n");
    return NULL;
}