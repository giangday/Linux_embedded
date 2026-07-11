Dưới đây là bảng tóm tắt toàn bộ "bản đồ kiến thức" về **IPC Signal (Tín hiệu giao tiếp giữa các tiến trình)** trong hệ điều hành Unix/Linux giúp bạn dễ dàng ôn tập và hệ thống hóa lại:

---

## 1. Định nghĩa & Bản chất Cốt lõi

* **Bản chất:** Signal là hình thức IPC **cổ điển nhất, nhanh nhất và gọn nhẹ nhất**.
* **Dữ liệu:** Không truyền được gói dữ liệu lớn (chuỗi, mảng). Nó **chỉ truyền duy nhất một số nguyên** (mã định danh sự kiện).
* **Cơ chế:** Hoạt động **Bất đồng bộ (Asynchronous Interrupt)**. Nó ngắt ngang xương luồng chạy hiện tại của tiến trình nhận để ép xử lý sự kiện khẩn cấp.

---

## 2. Các Tín hiệu (Signals) Kinh điển cần nhớ

| Mã số | Tên Hằng số | Nguồn kích hoạt phổ biến | Hành vi mặc định của hệ thống |
| --- | --- | --- | --- |
| **1** | `SIGHUP` | Tắt Terminal | Tải lại cấu hình (Reload Config) / Tắt tiến trình |
| **2** | `SIGINT` | Bấm **`Ctrl + C`** trên Terminal | Kết thúc tiến trình (Tắt lịch sự) |
| **3** | `SIGQUIT` | Bấm **`Ctrl + \`** trên Terminal | Kết thúc tiến trình + Tạo file Core Dump |
| **9** | `SIGKILL` | Lệnh `kill -9 <PID>` | **Chết ngay lập tức** (Quyền tối cao của Kernel) |
| **15** | `SIGTERM` | Lệnh `kill <PID>` mặc định | Kết thúc tiến trình (Cho phép dọn dẹp trước khi tắt) |
| **17** | `SIGCHLD` | Tiến trình con chết (`exit()`) | Mặc định bỏ qua (Dùng để cha biết mà gọi `wait()`) |
| **19** | `SIGSTOP` | Lệnh `kill -19 <PID>` | **Tạm dừng/Đóng băng** tiến trình ngay lập tức |
| **18** | `SIGCONT` | Lệnh `kill -18 <PID>` | Đánh thức tiến trình đang bị đóng băng chạy tiếp |

---

## 3. 3 Cách Tiến trình Phản ứng với một Signal

Khi một tín hiệu (trừ số 9 và 19) gửi đến, lập trình viên có thể cấu hình tiến trình xử lý theo 3 hướng:

1. **Mặc định (`SIG_DFL`):** Để hệ điều hành tự xử lý (thường là sập ứng dụng).
2. **Bỏ qua (`SIG_IGN`):** Vứt tín hiệu vào sọt rác, tín hiệu biến mất hoàn toàn.
3. **Bắt tín hiệu (Catch - Signal Handler):** Định nghĩa một hàm C riêng. Khi tín hiệu đến, CPU nhảy vào chạy hàm này, chạy xong **quay lại dòng code cũ chạy tiếp chứ không tắt tiến trình**.

---

## 4. Phân biệt Bỏ qua (Ignore) vs Chặn tạm thời (Blocking)

Đây là ranh giới rất dễ bị lầm tưởng khi mới học:

* **Ignore (`SIG_IGN`):** Không thèm nhận $\rightarrow$ Tín hiệu **bị xóa bỏ ngay lập tức** khi vừa tới.
* **Blocking (`SIG_BLOCK`):** Chưa thèm nhận lúc này $\rightarrow$ Tín hiệu được hệ điều hành giữ lại trong **Hàng đợi chờ (Pending Queue)**. Ngay khi tiến trình thực hiện **Unblock**, tín hiệu đó sẽ lập tức được phóng thích và kích hoạt hàm xử lý bù. (Thường dùng để bảo vệ các đoạn code ghi dữ liệu quan trọng không bị ngắt quãng).

> ⚠️ **Quy tắc gộp tín hiệu (Signal Merging):** Trong lúc đang bị Block, nếu bạn gửi một tín hiệu đến 100 lần, Kernel chỉ ghi nhận **1 lần duy nhất** vào hàng chờ Pending. Khi Unblock, hàm xử lý sẽ chỉ chạy 1 lần.

---

## 5. Hai Chốt chặn An toàn của Hệ điều hành

Lập trình viên có thể dùng vòng lặp để chặn hoặc thay đổi hành vi của tất cả các tín hiệu, **NGOẠI TRỪ 2 tín hiệu quyền lực tuyệt đối**:

* **`SIGKILL` (số 9)** * **`SIGSTOP` (số 19)**

Hệ điều hành cấm tuyệt đối việc can thiệp vào 2 mã này để đảm bảo Quản trị viên (Sysadmin) luôn có quyền sinh quyền sát, hạ gục hoặc đóng băng bất kỳ tiến trình lỗi/độc hại nào trên máy tính.

---

## 6. Các hàm System Call cốt lõi trong code C

* `signal(signum, handler)`: Hàm đăng ký cách xử lý cho một tín hiệu.
* `kill(pid, signum)`: Hàm dùng để **gửi** một tín hiệu từ tiến trình này sang tiến trình khác (hoặc dùng lệnh `kill -<số> <PID>` trên Terminal).
* `sigprocmask(mode, &new_set, &old_set)`: Hàm dùng để bật/tắt chế độ **Block/Unblock** tín hiệu.
* `sigemptyset()`, `sigaddset()`: Các hàm bổ trợ để tạo lập danh sách các tín hiệu cần Block.