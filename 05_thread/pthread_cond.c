#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

int fuel_tank = 0;              // Tài nguyên dùng chung
pthread_mutex_t mutex_fuel;     // Ổ khóa Mutex bảo vệ biến fuel_tank
pthread_cond_t cond_fuel;       // Biến điều kiện để báo thức

void* car_consumer(void* arg) {
    pthread_mutex_lock(&mutex_fuel);
    
    // BẮT BUỘC dùng vòng lặp while thay vì if để tránh hiện tượng báo thức giả (Spurious Wakeup)
    while (fuel_tank == 0) {
        printf("[XE Ô TÔ] Hết xăng rồi (Hiện tại: %dL). Đi ngủ chờ bơm xăng...\n", fuel_tank);
        
        // Hàm này cực kỳ đặc biệt: Nó vừa thả khóa mutex_fuel ra cho luồng khác dùng, 
        // vừa đưa luồng car_consumer vào trạng thái ngủ đông.
        pthread_cond_wait(&cond_fuel, &mutex_fuel);
        
        // Khi được luồng khác wake up, hàm này sẽ TỰ ĐỘNG lấy lại khóa mutex_fuel trước khi chạy tiếp xuống dưới.
        printf("[XE Ô TÔ] Vừa được báo thức! Thức dậy kiểm tra lại bình xăng...\n");
    }
    
    fuel_tank -= 40;
    printf("[XE Ô TÔ] Đã chạy! Trừ 40L xăng. Còn lại: %dL.\n", fuel_tank);
    
    pthread_mutex_unlock(&mutex_fuel);
    return NULL;
}

void* station_producer(void* arg) {
    for (int i = 0; i < 5; i++) {
        sleep(2); // Giả lập mất 2 giây để trạm chuẩn bị xăng
        
        pthread_mutex_lock(&mutex_fuel);
        fuel_tank += 40;
        printf("[TRẠM XĂNG] Đã bơm thêm 20L. Bình hiện tại: %dL.\n", fuel_tank);
        
        // Phát tín hiệu báo thức cho luồng đang ngủ (nếu bình đã đủ xăng)
        if (fuel_tank >= 40) {
            printf("[TRẠM XĂNG] Đủ xăng rồi! Bấm chuông gọi xe ô tô dậy nào...\n");
            pthread_cond_signal(&cond_fuel); // Đánh thức 1 luồng đang đợi trên cond_fuel
        }
        
        pthread_mutex_unlock(&mutex_fuel);
    }
    return NULL;
}

int main() {
    pthread_t th_car, th_station;
    
    pthread_mutex_init(&mutex_fuel, NULL);
    pthread_cond_init(&cond_fuel, NULL);
    
    pthread_create(&th_car, NULL, car_consumer, NULL);
    pthread_create(&th_station, NULL, station_producer, NULL);
    
    pthread_join(th_car, NULL);
    pthread_join(th_station, NULL);
    
    pthread_mutex_destroy(&mutex_fuel);
    pthread_cond_destroy(&cond_fuel);
    return 0;
}