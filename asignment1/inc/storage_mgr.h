#ifndef _STORAGE_MGR_H_
#define _STORAGE_MGR_H_

#include "sbuffer.h"

// Tham số truyền vào luồng Storage Manager
typedef struct {
    sbuffer_t *buffer;
    const char *db_filename;
} storage_mgr_args_t;

// Luồng Storage Manager: đọc sbuffer, ghi vào SQLite, xử lý retry khi lỗi DB
void *storage_manager_main(void *arg);

#endif