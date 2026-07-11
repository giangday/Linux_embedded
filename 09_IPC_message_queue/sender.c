#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>

struct msg_buffer {
    long msg_type;
    char msg_text[100];
};

int main() {
    key_t key = ftok("queue_key_file", 65); // Tạo key định danh
    int msgid = msgget(key, 0666 | IPC_CREAT); // Tạo/Mở Message Queue

    struct msg_buffer msg1, msg2;

    // Tin nhắn 1: Loại 1 (Tin khẩn cấp)
    msg1.msg_type = 1;
    strcpy(msg1.msg_text, "CẢNH BÁO: Hệ thống quá nhiệt!");
    msgsnd(msgid, &msg1, sizeof(msg1.msg_text), 0);

    // Tin nhắn 2: Loại 2 (Tin thông thường)
    msg2.msg_type = 2;
    strcpy(msg2.msg_text, "Thông báo: Bảo trì hệ thống vào 12h đêm.");
    msgsnd(msgid, &msg2, sizeof(msg2.msg_text), 0);

    printf("Sender: Đã bắn 2 thông điệp (Type 1 và Type 2) vào Queue.\n");
    return 0;
}