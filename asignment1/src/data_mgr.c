#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdarg.h>
#include "data_mgr.h"
#include "sbuffer.h"
#include "logger.h"
// --- Cấu hình có thể điều chỉnh theo đúng yêu cầu đề bài ---
#define MAX_SENSORS      64     // số lượng sensor tối đa theo dõi cùng lúc
#define RUN_AVG_WINDOW   5      // tính trung bình động của N giá trị gần nhất
#define TEMP_TOO_COLD    15.0   // ngưỡng lạnh (°C) - CHỈNH LẠI theo đề nếu khác
#define TEMP_TOO_HOT     25.0   // ngưỡng nóng (°C) - CHỈNH LẠI theo đề nếu khác



// Trạng thái running average của 1 sensor
typedef struct {
    int    sensor_id;
    int    in_use;                     // 0 = slot trống, 1 = đang theo dõi sensor này
    double values[RUN_AVG_WINDOW];     // vòng đệm N giá trị gần nhất
    int    count;                      // số giá trị đã có (tối đa RUN_AVG_WINDOW)
    int    index;                      // vị trí ghi tiếp theo trong mảng values
} sensor_state_t;

static sensor_state_t sensor_states[MAX_SENSORS];

// Tìm slot của sensor_id trong mảng, nếu chưa có thì cấp 1 slot mới
static sensor_state_t *get_or_create_sensor_state(int sensor_id) {
    // Tìm slot đã tồn tại
    for (int i = 0; i < MAX_SENSORS; i++) {
        if (sensor_states[i].in_use && sensor_states[i].sensor_id == sensor_id) {
            return &sensor_states[i];
        }
    }
    // Chưa có -> tìm slot trống để cấp mới
    for (int i = 0; i < MAX_SENSORS; i++) {
        if (!sensor_states[i].in_use) {
            sensor_states[i].in_use = 1;
            sensor_states[i].sensor_id = sensor_id;
            sensor_states[i].count = 0;
            sensor_states[i].index = 0;
            return &sensor_states[i];
        }
    }
    // Hết chỗ trống -> không theo dõi được sensor này
    return NULL;
}

// Thêm giá trị mới vào running average, trả về giá trị trung bình hiện tại
static double update_running_average(sensor_state_t *state, double new_value) {
    state->values[state->index] = new_value;
    state->index = (state->index + 1) % RUN_AVG_WINDOW;
    if (state->count < RUN_AVG_WINDOW) state->count++;

    double sum = 0;
    for (int i = 0; i < state->count; i++) {
        sum += state->values[i];
    }
    return sum / state->count;
}

void *data_manager_main(void *arg) {
    data_mgr_args_t *args = (data_mgr_args_t *)arg;
    sbuffer_t *buffer = args->buffer;

    // Khởi tạo bảng trạng thái sensor rỗng
    memset(sensor_states, 0, sizeof(sensor_states));

    printf("Data manager started.\n");

    while (1) {
        sensor_data_t data;
        int ret = sbuffer_read_datamgr(buffer, &data);

        if (ret == SBUFFER_EMPTY) {
            // Hệ thống đang shutdown và không còn dữ liệu mới -> kết thúc thread
            break;
        }
        if (ret != SBUFFER_SUCCESS) {
            fprintf(stderr, "Data manager: error reading sbuffer\n");
            continue;
        }

        sensor_state_t *state = get_or_create_sensor_state(data.sensor_id);

        if (state == NULL) {
            // Không còn chỗ theo dõi sensor mới -> coi như ID không hợp lệ để xử lý
            log_event("Received sensor data with invalid sensor node ID %d.", data.sensor_id);
            continue;
        }

        double avg = update_running_average(state, data.temperature);

        if (avg < TEMP_TOO_COLD) {
            log_event("The sensor node with %d reports it's too cold (running avg temperature = %.2f).",
                       data.sensor_id, avg);
        } else if (avg > TEMP_TOO_HOT) {
            log_event("The sensor node with %d reports it's too hot (running avg temperature = %.2f).",
                       data.sensor_id, avg);
        }
        // Lưu ý: KHÔNG ghi ngược avg vào sbuffer, đúng yêu cầu đề bài
    }

    printf("Data manager shutting down.\n");
    return NULL;
}