// #include <stdio.h>
// #include <unistd.h>
// #include <sys/types.h>

// int main() {
//     pid_t pid;

//     printf("--- Trước khi gọi fork(), chỉ có 1 tiến trình gốc chạy ---\n");
//     printf("PID của tiến trình gốc: %d\n\n", getpid());

//     // Gọi hàm fork() để nhân bản tiến trình
//     pid = fork();

//     // Kể từ dòng này, CẢ HAI tiến trình (cha và con) đều chạy song song
//     // nhưng giá trị biến `pid` ở mỗi tiến trình là khác nhau.

//     if (pid < 0) {
//         // Trường hợp lỗi (hiếm gặp, ví dụ: hết bộ nhớ hệ thống)
//         fprintf(stderr, "Fork thất bại!\n");
//         return 1;
//     } 
//     else if (pid == 0) {
//         // Đoạn code này CHỈ tiến trình CON thực thi
//         printf("[CON] Xin chào! Tôi là tiến trình CON.\n");
//         printf("[CON] PID của tôi là: %d\n", getpid());
//         printf("[CON] PID của cha tôi (PPID) là: %d\n", getppid());
//         printf("[CON] Giá trị fork() trả về cho tôi là: %d\n\n", pid);
//     } 
//     else {
//         // Đoạn code này CHỈ tiến trình CHA thực thi
//         printf("[CHA] Xin chào! Tôi là tiến trình CHA.\n");
//         printf("[CHA] PID của tôi là: %d\n", getpid());
//         printf("[CHA] Giá trị fork() trả về cho tôi (chính là PID của con) là: %d\n\n", pid);
        
//         // Chờ một chút để tiến trình con kịp in ra màn hình trước khi cha kết thúc
//         // sleep(1); 
//         while(1);
//     }

//     printf("[Chung] Dòng này sẽ được CẢ CHA VÀ CON in ra (PID: %d)\n", getpid());

//     return 0;
// }










// #include <stdio.h>
// #include <unistd.h>
// #include <sys/types.h>
// #include <sys/wait.h>

// int main() {
//     int shared_var = 100; // Biến này sẽ được copy sang các tiến trình con

//     pid_t child1 = fork();

//     if (child1 == 0) {
//         // Đây là Tiến trình Con 1
//         shared_var = 200; // Thay đổi giá trị của Con 1
//         printf("[Con 1] Giá trị biến: %d | Địa chỉ biến: %p\n", shared_var, (void*)&shared_var);
//         return 0;
//     }

//     // Cha tiếp tục đẻ con thứ 2
//     pid_t child2 = fork();

//     if (child2 == 0) {
//         // Đây là Tiến trình Con 2
//         shared_var = 300; // Thay đổi giá trị của Con 2
//         printf("[Con 2] Giá trị biến: %d | Địa chỉ biến: %p\n", shared_var, (void*)&shared_var);
//         return 0;
//     }

//     // Cha đợi các con chạy xong
//     wait(NULL);
//     wait(NULL);

//     // Đây là Tiến trình Cha
//     printf("[CHA]   Giá trị biến: %d | Địa chỉ biến: %p\n", shared_var, (void*)&shared_var);

//     return 0;
// }









#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    pid_t pid;
    int status;

    printf("[CHA] Bắt đầu khởi tạo tiến trình con...\n");
    pid = fork();

    if (pid < 0) {
        perror("Fork thất bại");
        exit(1);
    } 
    else if (pid == 0) {
        // --- ĐÂY LÀ TIẾN TRÌNH CON ---
        printf("[CON] Xin chào! Tôi đã được sinh ra. PID của tôi là: %d\n", getpid());
        printf("[CON] Tôi đang làm việc nặng trong 3 giây...\n");
        
        // sleep(3); // Giả lập công việc tốn thời gian
        while(1){
            printf("[CON] Công việc đang diễn ra...\n");
            sleep(1);
        }
        
        printf("[CON] Công việc hoàn thành. Tôi tự kết thúc tại đây.\n");
        // Con gọi exit() và trả về mã trạng thái thành công là 42

        exit(42); 
    } 
    else {
        // --- ĐÂY LÀ TIẾN TRÌNH CHA ---
        printf("[CHA] Tôi vừa đẻ con có PID là: %d\n", pid);
        printf("[CHA] Tôi sẽ đứng đợi (wait) cho đến khi con làm xong việc...\n");

        // Cha bị block tại đây, đợi đứa con cụ thể có PID này kết thúc
        pid_t dead_child_pid = wait(&status);

        printf("[CHA] A! Đứa con có PID %d đã kết thúc.\n", dead_child_pid);

        // Kiểm tra xem con kết thúc bình thường (qua hàm exit) hay bị crash
        if (WIFEXITED(status)) {
            // Lấy mã status mà con đã truyền vào hàm exit()
            int exit_code = WEXITSTATUS(status);
            printf("[CHA] Mã trạng thái con trả về (Exit Code) là: %d\n", exit_code);
        } else {
            printf("[CHA] Tiến trình con kết thúc bất thường.\n");
        }
        
        printf("[CHA] Đã dọn dẹp xong tài nguyên của con. Tiến trình cha kết thúc.\n");
    }

    return 0;
}