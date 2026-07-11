#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>

struct msg_buffer {
    long msg_type;
    char msg_text[100];
};

int main() {
    key_t key = ftok("queue_key_file", 65);
    int msgid = msgget(key, 0666 | IPC_CREAT);

    struct msg_buffer msg;

    printf("Receiver: Tôi sẽ chủ động lọc lấy tin nhắn Type 2 trước...\n");
    
    /* Tham số thứ 4 chính là Type cần lọc (ở đây chọn số 2) */
    msgrcv(msgid, &msg, sizeof(msg.msg_text), 2, 0);

    printf("Receiver nhận được tin nhắn Type %ld: %s\n", msg.msg_type, msg.msg_text);

    // Xóa Message Queue khỏi hệ thống RAM của Kernel sau khi dùng xong
    msgctl(msgid, IPC_RMID, NULL);
    return 0;
}