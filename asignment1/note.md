Dựa trên các hình ảnh đề bài bạn đã cung cấp, đây là bản mô tả chi tiết, có cấu trúc rõ ràng về bài tập lớn (**Assignment: Sensor Monitoring System**) dành cho môn Lập trình hệ thống Linux.

---

## TỔNG QUAN BÀI TẬP: SENSOR MONITORING SYSTEM

Bài tập yêu cầu bạn thiết kế và lập trình một **Sensor Gateway (Cổng kết nối cảm biến)** bằng ngôn ngữ **C** trên hệ điều hành Linux. Hệ thống này có nhiệm vụ thu thập dữ liệu nhiệt độ phòng từ nhiều nút cảm biến giả lập, xử lý dữ liệu, lưu trữ vào cơ sở dữ liệu SQLite và ghi log hệ thống.

Kiến trúc cốt lõi của bài tập tập trung vào việc kết hợp **Đa tiến trình (Multi-processing)**, **Đa luồng (Multi-threading)**, các cơ chế **IPC (Socket, FIFO)** và đảm bảo an toàn dữ liệu (**Thread-safe**).

---

## 1. Kiến trúc Hệ thống & Luồng Dữ liệu

Hệ thống được chia làm 2 phân vùng tiến trình chính bằng hàm `fork()`: **Main Process** và **Log Process**.

### A. Main Process (Tiến trình chính)

Tiến trình này quản lý một cấu trúc dữ liệu dùng chung duy nhất (**Shared Data Structure / sbuffer**) và chạy **3 luồng (threads)** độc lập:

1. **Connection Manager Thread (Luồng quản lý kết nối):**
* Lắng nghe các yêu cầu kết nối từ các nút cảm biến qua **TCP Socket** (Cổng Port được truyền vào từ tham số dòng lệnh, ví dụ: `./server 1234`).
* Nhận các gói dữ liệu từ cảm biến và ghi chúng vào cấu trúc dữ liệu dùng chung (`sbuffer`).


2. **Data Manager Thread (Luồng xử lý dữ liệu):**
* Đọc dữ liệu đo đạc từ `sbuffer`.
* Tính toán trung bình động (running average) của nhiệt độ để đưa ra quyết định hệ thống đang "quá nóng" hoặc "quá lạnh" (không ghi ngược lại giá trị trung bình này vào `sbuffer`).


3. **Storage Manager Thread (Luồng lưu trữ):**
* Đọc dữ liệu từ `sbuffer` và chèn (insert) vào cơ sở dữ liệu **SQLite**.
* **Xử lý lỗi kết nối:** Nếu lỗi kết nối DB, luồng này sẽ đợi một lát rồi thử lại. Dữ liệu cảm biến phải được giữ lại trong `sbuffer` cho đến khi DB hoạt động trở lại. Nếu thử lại quá 3 lần thất bại, Gateway sẽ tự động đóng.



> ⚠️ **Yêu cầu chí mạng (Dữ liệu dùng chung):** Chỉ được phép sử dụng **duy nhất 1 cấu trúc sbuffer** để chia sẻ dữ liệu giữa 3 luồng trên. Việc truy cập (Đọc/Ghi/Cập nhật) vào `sbuffer` bắt buộc phải **Thread-safe** (sử dụng Mutex/Semaphore để đồng bộ hóa).

### B. Log Process (Tiến trình ghi log)

* Được tạo ra từ Main Process bằng hàm `fork()`.
* **Kênh truyền thông:** Nhận các sự kiện log (log-events) từ Main Process gửi sang thông qua một đường ống đặt tên **FIFO** tên là `logFifo`.
* Tất cả 3 luồng của Main Process đều có thể tạo sự kiện log và ghi vào `logFifo` này (do đó việc ghi vào FIFO cũng phải đảm bảo **Thread-safe**).
* **Nhiệm vụ:** Đọc dữ liệu từ FIFO và ghi định dạng vào file log `gateway.log` theo cấu trúc: `<sequence number> <timestamp> <log-event info message>`.

---

## 2. Các Sự kiện Log bắt buộc phải hỗ trợ (Req 9)

Hệ thống phải bắt và ghi lại ít nhất các sự kiện sau vào `gateway.log`:

* **Từ Connection Manager:**
* Khi một nút cảm biến kết nối thành công: `A sensor node with <sensorNodeID> has opened a new connection` *(Lưu ý: Chỉ biết ID sau khi gói dữ liệu đầu tiên đến).*
* Khi một nút cảm biến ngắt kết nối: `The sensor node with <sensorNodeID> has closed the connection`.


* **Từ Data Manager:**
* Cảnh báo lạnh: `The sensor node with <sensorNodeID> reports it's too cold (running avg temperature = <value>)`.
* Cảnh báo nóng: `The sensor node with <sensorNodeID> reports it's too hot (running avg temperature = <value>)`.
* Lỗi ID: `Received sensor data with invalid sensor node ID <node-ID>`.


* **Từ Storage Manager:**
* Kết nối DB thành công: `Connection to SQL server established`.
* Tạo bảng mới thành công: `New table <name-of-table> created`.
* Mất kết nối DB: `Connection to SQL server lost`.
* Thất bại khi kết nối DB: `Unable to connect to SQL server`.



---

## 3. Tiêu chí Đánh giá & Chấm điểm (Grading Criteria)

Để bài tập được chấp nhận và chấm điểm, bạn cần tuân thủ các quy tắc nghiêm ngặt sau:

* **Tính hoàn thiện:** Code phải biên dịch được (`compile`), chạy được (`run`) và sinh ra được đầu ra thực tế (dữ liệu trong DB, file log). Không chấp nhận file `.c` rỗng hoặc code sao chép từ năm trước.
* **Cấu trúc Code:** Phải phân tách rõ ràng thành các file nguồn (`.c`) và file tiêu đề (`.h`), đặt tên biến/hàm có ý nghĩa, thụt lề chuẩn, **không dùng lệnh `goto**` để viết code rối rắm (spaghetti code).
* **Bảo vệ đồ án (Defense):** Bạn phải có khả năng giải thích và trả lời các câu hỏi kỹ thuật liên quan đến:
* Ngôn ngữ C và các System Call của Linux.
* Quy trình build code: Preprocessor, Compiler, Linker, Assembly.
* Cách tạo và liên kết thư viện tĩnh (`static library - ar`) và thư viện chia sẻ (`shared library - gcc, ldd, ldconfig`).
* Kiểm tra rò rỉ bộ nhớ bằng công cụ **Valgrind**.