# BÁO CÁO PHÂN TÍCH VÀ THIẾT KẾ ĐỒ ÁN: RẮN SĂN MỒI (SNAKE GAME CONSOLE C++)

## 1. Giới thiệu tổng quan
Dự án game Rắn Săn Mồi trên giao diện dòng lệnh (Windows Console) được xây dựng bằng ngôn ngữ C++ với lối thiết kế module rõ ràng, tối ưu hóa trải nghiệm người dùng và hoàn toàn không gây hiện tượng nhấp nháy màn hình (flickering).

## 2. Đáp ứng tiêu chuẩn yêu cầu
- **Cấu trúc Menu vòng lặp**: Menu điều khiển chạy lặp vô hạn, chỉ kết thúc khi chọn mục "5. Thoát".
- **Kiểm soát dữ liệu đầu vào**: Hàm `getValidMenuChoice()` bắt lỗi người dùng nhập sai kiểu (ký tự, chuỗi chữ), nhập ngoài khoảng cho phép và yêu cầu nhập lại, tuyệt đối không bị crash hay tràn bộ nhớ đệm.
- **Hàm `main` độc lập**: Đóng vai trò làm điều phối viên (dispatcher), không chứa trực tiếp logic xử lý của game.
- **Xử lý đồ họa Console chống nhấp nháy**:
  - Sử dụng hàm `SetConsoleCursorPosition` để cập nhật tọa độ đầu và xóa đốt đuôi cũ thay vì gọi lệnh xóa toàn màn hình `system("cls")`.
  - Ẩn con trỏ console bằng `SetConsoleCursorInfo` giúp màn hình mượt mà.
- **Cơ chế đọc phím thời gian thực**: Sử dụng kết hợp `_kbhit()` và `_getch()`, hỗ trợ cả 2 cụm phím W/A/S/D và 4 phím mũi tên. Kiểm tra chặn quay đầu góc 180°.
- **Cấu trúc dữ liệu**:
  - Thân rắn lưu bằng 2 mảng tọa độ 1 chiều `snakeX[]`, `snakeY[]`.
  - Mồi thường (`O`) được sinh ngẫu nhiên và kiểm tra không trùng với các đốt thân rắn.
- **Tính năng nâng cao (Điểm cộng)**:
  - **Tô màu Console**: Sử dụng `SetConsoleTextAttribute` để phân biệt khung viền, đầu rắn, thân rắn, mồi và điểm số.
  - **Mồi đặc biệt (`$`)**: Xuất hiện ngẫu nhiên theo thời gian thực (tồn tại 8 giây), đem lại 30 điểm.
  - **Chế độ Xuyên tường**: Người chơi có thể lựa chọn chế độ xuyên qua mép đối diện hoặc va chạm biên là thua.
  - **Lưu kỷ lục ra file**: Ghi nhận và sắp xếp Top 5 người chơi điểm cao vào file `highscores.txt`.
  - **Phím P (Pause)**: Cho phép tạm dừng màn chơi tức thời.

## 3. Hướng dẫn biên dịch và quản lý Git

### Biên dịch bằng MinGW g++:
```bash
g++ -O2 main.cpp -o snake_game.exe
./snake_game.exe