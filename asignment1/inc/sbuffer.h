#ifndef _SBUFFER_H_
#define _SBUFFER_H_

#include <pthread.h>
#include <time.h>
#include <stdint.h>

// Mã lỗi trả về
#define SBUFFER_SUCCESS 0
#define SBUFFER_FAILURE -1
#define SBUFFER_NO_DATA -2
#define SBUFFER_EMPTY   -3


//cau truc du lieu cua 1 goi tin cam bien gui den server
typedef struct{
    int sensor_id;
    double temperature;
    time_t timestamp;
}sensor_data_t;

//Node trong linked list

typedef struct sbuffer_node{
    sensor_data_t data;
    struct sbuffer_node *next;
    int read_by_datamgr;
    int read_by_storagemgr;
}sbuffer_node_t;

//cau truc du lieu cua 1 buffer
typedef struct{
    sbuffer_node_t *head;
    sbuffer_node_t *tail;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int shutdown_flag;
}sbuffer_t;


// khoi tao buffer
int sbuffer_init(sbuffer_t **buffer);

// xoa buffer
int sbuffer_free(sbuffer_t **buffer);

// them du lieu vao buffer
int sbuffer_insert(sbuffer_t *buffer, sensor_data_t *data);

// lay du lieu tu buffer ma data manager chua doc
int sbuffer_read_datamgr(sbuffer_t *buffer, sensor_data_t *data);

// lay du lieu tu buffer ma storage manager chua doc
int sbuffer_read_storagemgr(sbuffer_t *buffer, sensor_data_t *data);

// xoa node da doc ra khoi lish
// int sbuffer_remove_node(sbuffer_t *buffer);

void sbuffer_shutdown(sbuffer_t *buffer);

#endif // _SBUFFER_H_



