Ngoài Mutex và Condition Variable – hai vũ khí hạng nặng mà bạn đã làm chủ, trong thế giới lập trình hệ thống còn có các kỹ thuật đồng bộ khác. Mỗi kỹ thuật được thiết kế ra để tối ưu hóa cho từng kịch bản phần cứng và bài toán logic cụ thể nhằm đạt hiệu năng tối đa.

Dưới đây là các kỹ thuật đồng bộ cốt lõi còn lại mà một kỹ sư hệ thống cần nằm lòng:

---

## 1. Semaphore (Biến đếm quyền lực)

Nếu Mutex là ổ khóa chỉ cho phép **đúng 1 luồng** vào phòng, thì **Semaphore** là một cái hộp chứa $N$ chiếc chìa khóa. Nó cho phép tối đa $N$ luồng cùng vào khai thác tài nguyên cùng lúc.

* **Cách vận hành:** Semaphore quản lý một biến đếm (Counter):
* Hàm `sem_wait()` (P operation): Nếu biến đếm $> 0$, giảm đi 1 và luồng đi tiếp. Nếu biến đếm $= 0$, luồng bị khóa lại và đi ngủ.
* Hàm `sem_post()` (V operation): Tăng biến đếm thêm 1 và đánh thức một luồng đang ngủ dậy.


* **Ứng dụng kinh điển:** Giới hạn tài nguyên (Connection Pool). Ví dụ, Server của bạn chỉ chịu tải được tối đa 10 kết nối tới Cơ sở dữ liệu cùng lúc. Bạn đặt Semaphore với biến đếm bằng 10. Luồng thứ 11 đến sẽ buộc phải xếp hàng đợi cho đến khi có 1 trong 10 luồng trước trả lại chìa khóa.

> 💡 **Mẹo tư duy:** Mutex thực chất là một trường hợp đặc biệt của Semaphore với biến đếm khởi tạo bằng 1 (gọi là *Binary Semaphore*).

---

## 2. Read-Write Locks (Khóa phân quyền Đọc - Ghi)

Trong thực tế, việc nhiều luồng **cùng đọc** một biến toàn cục tại một thời điểm hoàn toàn không gây ra lỗi Race Condition (vì không ai sửa đổi dữ liệu). Race Condition chỉ xảy ra khi có ít nhất một luồng **ghi (sửa)** dữ liệu.

Nếu dùng Mutex thông thường, bạn sẽ khóa luôn cả các luồng đọc, gây lãng phí hiệu năng rất lớn. **Read-Write Lock (`pthread_rwlock_t`)** ra đời để giải quyết việc này.

* **Cơ chế hoạt động:**
* **Shared Lock (Khóa Đọc):** Cho phép **hàng trăm luồng cùng vào đọc** dữ liệu song song thực sự trên các Core CPU.
* **Exclusive Lock (Khóa Ghi):** Khi có một luồng muốn sửa dữ liệu, nó sẽ bao trọn ổ khóa. Lúc này, **tất cả các luồng đọc và luồng ghi khác đều bị chặn đứng** bên ngoài cho đến khi luồng ghi này làm xong.


* **Ứng dụng kinh điển:** Hệ thống bộ đệm (Cache Database), bảng tra cứu cấu hình hệ thống (nơi dữ liệu được đọc liên tục từng giây nhưng cả ngày mới cập nhật/ghi một lần).

---

## 3. Spinlock (Khóa bận tốc độ cao)

Khi một luồng gọi `pthread_mutex_lock` mà khóa đang bị đóng, Kernel sẽ đẩy luồng đó vào trạng thái ngủ đông (`Context Switch`). Nhưng như bạn đã biết, chi phí để hoán đổi ngữ cảnh, cất/nạp thanh ghi và xóa bộ đệm Cache là rất đắt đỏ.

Nếu luồng giữ khóa chỉ xử lý một phép toán siêu nhanh (như `count++` mất vài nano-giây), việc cho luồng sau đi ngủ rồi gọi dậy là một sự lãng phí cực lớn. **Spinlock** ra đời để tối ưu hóa kịch bản này.

* **Cơ chế hoạt động:** Thay vì đi ngủ, luồng đến sau sẽ **chạy một vòng lặp `while` vô hạn ở tốc độ tối đa** để liên tục rà soát xem khóa đã mở chưa (gọi là *Busy Waiting*).
* **Ưu điểm:** Ngay khi khóa vừa mở, luồng cướp được luôn mà không tốn một nano-giây nào cho việc hoán đổi ngữ cảnh của Kernel. Tốc độ phản xạ là tuyệt đối.
* **Nhược điểm:** Vắt kiệt 100% công suất Core CPU trong lúc đợi. Do đó, Spinlock **chỉ được dùng trong lập trình nhúng, lập trình Driver hoặc Kernel** nơi thời gian giữ khóa cực kỳ ngắn. Nếu luồng trước giữ khóa lâu, Spinlock sẽ làm treo cứng hệ thống.

---

## 4. Barrier (Chốt chặn hội quân)

Hãy tưởng tượng bạn đang viết một Game đồ họa đa luồng: Luồng 1 tính toán vật lý, Luồng 2 tính toán ánh sáng, Luồng 3 tính toán AI quái vật. Để vẽ được khung hình tiếp theo, bạn bắt buộc phải đợi cả 3 luồng này tính xong xuôi phần việc của mình ở khung hình cũ.

**Barrier (`pthread_barrier_t`)** chính là một điểm tập kết được cấu hình số lượng luồng tham gia cố định.

* **Cơ chế hoạt động:** Bạn đặt một Barrier với số lượng là 3.
* Luồng 1 tính xong trước, gọi `pthread_barrier_wait()` $\rightarrow$ bị giữ lại dừng hình tại chỗ.
* Luồng 2 tính xong, gọi `pthread_barrier_wait()` $\rightarrow$ tiếp tục bị giữ lại.
* Khi Luồng 3 (luồng cuối cùng) tính xong và bước đến chốt chặn $\rightarrow$ Barrier lập tức mở toang cửa, **phóng thích cả 3 luồng cùng lúc** để sang màn chơi/bước tính toán tiếp theo.



---

## 5. Atomic Operations (Phép toán nguyên tử - Đồng bộ tầng phần cứng)

Đây là đỉnh cao của tối ưu hiệu năng (gọi là lập trình **Lock-free**). Các kỹ thuật trên (Mutex, Semaphore) đều là giải pháp phần mềm do OS quản lý. Còn **Atomic Operations** là kỹ thuật ép thẳng chip CPU ở tầng phần cứng phải xử lý an toàn mà không cần ổ khóa nào.

* **Bản chất:** CPU hiện đại cung cấp các tập lệnh đặc biệt ở tầng Assembly (như `Lock XCHG` trên x86 hoặc `LDREX/STREX` trên ARM).
* Thay vì chạy 3 bước (Đọc $\rightarrow$ Sửa $\rightarrow$ Ghi), CPU sẽ gộp lệnh `count++` thành **một bước duy nhất, bất khả phân chia** ở mức mạch điện tử. Tại một chu kỳ xung nhịp, hoặc là phép toán thực hiện thành công, hoặc là không làm gì cả, không một core nào khác có thể xen vào giữa.
* **Thư viện chuẩn:** Trong C11, bạn có thể dùng `<stdatomic.h>` với kiểu dữ liệu `atomic_int`. Việc tăng giảm biến atomic chạy nhanh gấp hàng chục lần so với việc bọc biến đó trong Mutex.

---

## 📋 Bảng tổng kết kịch bản chọn lựa kỹ thuật

| Nếu bài toán của bạn là... | Kỹ thuật tối ưu nhất là... |
| --- | --- |
| Bảo vệ một biến đếm, một ô nhớ cơ bản | **Atomic Operations** (Nhanh nhất, không tốn tài nguyên khóa) |
| Chỉ cho phép 1 luồng xử lý khối code lớn | **Mutex** (An toàn, phổ biến nhất) |
| Giới hạn số lượng tài nguyên ($N$ kết nối, $N$ chỗ ngồi) | **Semaphore** (Quản lý biến đếm luồng hiệu quả) |
| Đợi dữ liệu thay đổi từ luồng khác mà chưa biết bao giờ xong | **Condition Variable** (Ngủ đông giải phóng CPU hoàn toàn) |
| Hệ thống đọc dữ liệu hàng triệu lần nhưng rất ít khi ghi/sửa | **Read-Write Lock** (Tăng tốc độ đọc song song tối đa) |
| Thời gian giữ khóa siêu ngắn, cần phản ứng lập tức | **Spinlock** (Bỏ qua chi phí ngủ của Kernel, chấp nhận tốn CPU) |
| Chia việc tính toán song song, cần các luồng đợi nhau đủ quân mới chạy tiếp | **Barrier** (Đồng bộ theo giai đoạn - Phù hợp cho Render, AI) |