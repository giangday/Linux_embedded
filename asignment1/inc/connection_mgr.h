#ifndef _CONNECTION_MGR_H_
#define _CONNECTION_MGR_H_

#include "sbuffer.h"

// Tham số truyền cho luồng chính connection_manager_main
typedef struct {
    int port;
    sbuffer_t *buffer;
} conn_mgr_args_t;

// Luồng chính: bind, listen, accept vô hạn, spawn 1 worker thread / kết nối
void *connection_manager_main(void *arg);

// Đóng cưỡng bức listening socket từ thread khác (dùng khi hệ thống cần shutdown,
// ví dụ storage_mgr lỗi DB quá 3 lần) để phá vỡ accept() đang block
void connection_mgr_force_close(void);

#endif