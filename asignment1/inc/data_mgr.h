#ifndef _DATA_MGR_H_
#define _DATA_MGR_H_

#include "sbuffer.h"

// Tham số truyền vào luồng Data Manager
typedef struct {
    sbuffer_t *buffer;
} data_mgr_args_t;

// Luồng Data Manager: đọc sbuffer, tính running average, log cảnh báo
void *data_manager_main(void *arg);

#endif