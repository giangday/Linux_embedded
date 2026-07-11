#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sbuffer.h"


int sbuffer_init(sbuffer_t **buffer){
    if(buffer == NULL) return SBUFFER_FAILURE;

    *buffer = malloc(sizeof(sbuffer_t));
    if(*buffer == NULL) return SBUFFER_FAILURE;

    (*buffer)->head = NULL;
    (*buffer)->tail = NULL;
    (*buffer)->shutdown_flag = 0;

    pthread_mutex_init(&(*buffer)->mutex, NULL);
    pthread_cond_init(&(*buffer)->cond, NULL);

    return 0;
}


int sbuffer_free(sbuffer_t **buffer){
    if(buffer == NULL || *buffer == NULL) return SBUFFER_FAILURE;

    pthread_mutex_lock(&(*buffer)->mutex);
    sbuffer_node_t *node = (*buffer)->head;
    while(node != NULL){
        sbuffer_node_t *tmp = node;
        node = node->next;
        free(tmp);
    }
    pthread_mutex_unlock(&(*buffer)->mutex);

    pthread_mutex_destroy(&(*buffer)->mutex);
    pthread_cond_destroy(&(*buffer)->cond);

    free(*buffer);
    *buffer = NULL;
    
    return SBUFFER_SUCCESS;
}


int sbuffer_insert(sbuffer_t *buffer, sensor_data_t *data){
    if (buffer == NULL || data == NULL) return SBUFFER_FAILURE;
    
    sbuffer_node_t *node = malloc(sizeof(sbuffer_node_t));
    if (node == NULL) return SBUFFER_FAILURE;
    //Sao chép dữ liệu và đặt cờ mặc định là chưa luồng nào đọc (0)
    node->data = *data;
    node->next = NULL;
    node->read_by_datamgr = 0;
    node->read_by_storagemgr = 0;

    pthread_mutex_lock(&buffer->mutex);
    // Chèn node mới vào đuôi danh sách liên kết
    if(buffer->tail == NULL){
        buffer->head = node;
        buffer->tail = node;
    }
    else{
        buffer->tail->next = node;
        buffer->tail = node;
    }
    // Phát tín hiệu đánh thức tất cả các luồng đang ngủ đợi dữ liệu
    pthread_cond_broadcast(&buffer->cond);
    pthread_mutex_unlock(&buffer->mutex);

    return SBUFFER_SUCCESS;
}


int sbuffer_read_datamgr(sbuffer_t *buffer, sensor_data_t *data){
    if(buffer == NULL || data == NULL) return SBUFFER_FAILURE;

    pthread_mutex_lock(&buffer->mutex);

    // Duyệt từ đầu chuỗi tìm node đầu tiên mà datamgr chưa đọc
    sbuffer_node_t *current = buffer->head;
    while(current != NULL && current->read_by_datamgr == 1){
        current = current->next;
    }

    // Nếu không tìm thấy node nào mới, luồng sẽ đi ngủ đợi tín hiệu từ insert
    while (current == NULL && !buffer->shutdown_flag){
        pthread_cond_wait(&buffer->cond, &buffer->mutex);

        current = buffer->head;
        while(current != NULL && current->read_by_datamgr == 1){
            current = current->next;
        }
    }

    // Nếu thoát ra do shutdown mà vẫn không có dữ liệu mới
    if (current == NULL && buffer->shutdown_flag) {
        pthread_mutex_unlock(&buffer->mutex);
        return SBUFFER_EMPTY;
    }

    // Đã tìm thấy node mới, copy dữ liệu ra ngoài và đánh dấu đã đọc
    *data = current->data;
    current->read_by_datamgr = 1;

    // Dọn dẹp node đã đọc
    while(buffer->head != NULL && buffer->head->read_by_datamgr && buffer->head->read_by_storagemgr){
        sbuffer_node_t *temp = buffer->head;
        buffer->head = buffer->head->next;

        if(buffer->head == NULL){
            buffer->tail = NULL;
        }

        free(temp);
    }

    pthread_mutex_unlock(&buffer->mutex);
    return SBUFFER_SUCCESS;
}


int sbuffer_read_storagemgr(sbuffer_t *buffer, sensor_data_t *data){
    if(buffer == NULL || data == NULL) return SBUFFER_FAILURE;

    pthread_mutex_lock(&buffer->mutex);

    // Duyệt từ đầu chuỗi tìm node đầu tiên mà storagemgr chưa đọc
    sbuffer_node_t *current = buffer->head;
    while(current != NULL && current->read_by_storagemgr == 1){
        current = current->next;
    }

    // Nếu không tìm thấy node nào mới, luồng sẽ đi ngủ đợi tín hiệu từ insert
    while (current == NULL && !buffer->shutdown_flag){
        pthread_cond_wait(&buffer->cond, &buffer->mutex);

        current = buffer->head;
        while(current != NULL && current->read_by_storagemgr == 1){
            current = current->next;
        }
    }

    // Nếu thoát ra do shutdown mà vẫn không có dữ liệu mới
    if (current == NULL && buffer->shutdown_flag) {
        pthread_mutex_unlock(&buffer->mutex);
        return SBUFFER_EMPTY;
    }

    // Đã tìm thấy node mới, copy dữ liệu ra ngoài và đánh dấu đã đọc
    *data = current->data;
    current->read_by_storagemgr = 1;

    // Dọn dẹp node đã đọc
    while(buffer->head != NULL && buffer->head->read_by_datamgr && buffer->head->read_by_storagemgr){
        sbuffer_node_t *temp = buffer->head;
        buffer->head = buffer->head->next;

        if(buffer->head == NULL){
            buffer->tail = NULL;
        }

        free(temp);
    }

    pthread_mutex_unlock(&buffer->mutex);
    return SBUFFER_SUCCESS;
}

void sbuffer_shutdown(sbuffer_t *buffer) {
    if (buffer == NULL) return;
    pthread_mutex_lock(&buffer->mutex);
    buffer->shutdown_flag = 1;
    pthread_cond_broadcast(&buffer->cond); // Đánh thức tất cả để thoát
    pthread_mutex_unlock(&buffer->mutex);
}