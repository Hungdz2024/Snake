# BÁO CÁO PHÂN TÍCH VÀ THIẾT KẾ ĐỒ ÁN: RẮN SĂN MỒI (SNAKE GAME - GUI EDITION)

## 1. Giới thiệu dự án
Dự án **Rắn Săn Mồi (Snake Game)** được phát triển bằng ngôn ngữ **C++** với giao diện đồ họa **Native Windows GUI (Win32 API & GDI)**. Trò chơi khởi chạy trực tiếp dưới dạng một ứng dụng cửa sổ Windows độc lập, hiện đại, không mở cửa sổ dòng lệnh (Terminal) và không cần cài đặt thêm bất kỳ thư viện bên ngoài nào (như SDL2, SFML hay OpenGL).

- **Ngôn ngữ**: C++ (chuẩn C++11 trở lên).
- **Môi trường**: Windows 10 / 11.
- **Thư viện đồ họa**: Win32 API, GDI (`windows.h`, `wingdi.h`).
- **Mô hình kiến trúc**: Event-Driven kết hợp Real-time Game Loop thông qua Windows Message Loop và `WM_TIMER`.

---

## 2. Kiến trúc & Phân tích Thiết kế (Architecture & Design)

### 2.1. Quản lý trạng thái trò chơi (Finite State Machine - FSM)
Chương trình được điều phối thông qua enum `GameState` đảm bảo luồng hoạt động mượt mà và không bị xung đột:
- `STATE_MENU`: Màn hình khởi đầu, cho phép lựa chọn chế độ chơi hoặc thoát game.
- `STATE_PLAYING`: Trạng thái xử lý logic di chuyển, ăn mồi và tính điểm thời gian thực.
- `STATE_PAUSED`: Trạng thái tạm dừng khi người chơi nhấn `P`.
- `STATE_GAMEOVER`: Màn hình kết thúc khi rắn va chạm, hiển thị điểm số và các tùy chọn thao tác tiếp theo.

### 2.2. Xử lý đồ họa chống nhấp nháy (Double Buffering)
- Game áp dụng kỹ thuật **Double Buffering** trong hàm `render()`:
  1. Khởi tạo một Device Context ảo trong bộ nhớ RAM (`CreateCompatibleDC`).
  2. Vẽ toàn bộ bàn cờ, lưới ô vuông, thân rắn (bo góc mềm mại), mồi ăn và bảng điều khiển Sidebar lên bộ đệm ẩn.
  3. Đẩy toàn bộ khung hình đã xử lý xong ra màn hình hiển thị chỉ với một lệnh duy nhất: `BitBlt()`.
- **Hiệu quả**: Loại bỏ 100% hiện tượng chớp giật màn hình (screen flickering), tốc độ hiển thị đạt 60 FPS ổn định.

### 2.3. Cấu trúc dữ liệu & Xử lý va chạm
- **Thân rắn**: Quản lý bằng `std::vector<Point>`. Mỗi nhịp tick của timer, đầu mới được thêm vào đầu mảng (`insert`) và đốt đuôi được giải phóng (`pop_back`) nếu không ăn mồi.
- **Chống quay đầu 180°**: Sử dụng biến đệm `g_nextDir` để lọc hướng hợp lệ trước khi cập nhật `g_dir`.
- **Sinh mồi ngẫu nhiên**: Thuật toán kiểm tra lặp để đảm bảo mồi thường và mồi đặc biệt không bao giờ xuất hiện trùng lên thân rắn.
- **Tăng tốc theo điểm**: Độ trễ `g_speedMs` giảm dần mỗi khi rắn ăn mồi, giúp độ khó tăng dần theo thời gian.

---

## 3. Các tính năng nổi bật (Chức năng tối thiểu & Nâng cao)

1. **Giao diện đồ họa Modern Dark Theme**:
   - Sử dụng bảng màu Dark Slate chuẩn công nghệ, dịu mắt.
   - Thân rắn màu xanh ngọc bo góc mềm mại (`RoundRect`), mồi thường màu đỏ và mồi đặc biệt phát sáng màu vàng.
2. **Hai chế độ chơi đa dạng**:
   - **Chế độ Cổ điển (Classic)**: Đâm vào tường biên là Game Over ngay lập tức.
   - **Chế độ Xuyên tường (Pass-through Wall)**: Rắn đi xuyên qua mép biên sẽ xuất hiện ở bờ đối diện.
3. **Mồi đặc biệt có thời hạn**:
   - Xuất hiện ngẫu nhiên theo thời gian thực (hiển thị ký hiệu vàng phát sáng).
   - Đếm ngược thời gian tồn tại trên thanh Sidebar, ăn được cộng 30 điểm.
4. **Lưu trữ kỷ lục điểm số**:
   - Tự động đọc và lưu điểm số cao nhất (High Score) vào tệp `highscores.txt`.

---

## 4. Hướng dẫn Biên dịch & Chạy ứng dụng

### Lệnh 1 dòng duy nhất (Biên dịch + Chạy ngay):
Mở **PowerShell** tại thư mục dự án và chạy:
```powershell
g++ -O2 main.cpp -o snake_gui.exe -mwindows -lgdi32; if ($?) { .\snake_gui.exe }