&#x09;						Tóm tắt các điểm kỹ thuật nổi bật

1.Chống nhấp nháy màn hình\*\*: Chỉ vẽ lại đầu rắn mới (`@`), khúc cổ (`#`) và xóa ký tự đuôi cũ thành khoảng trắng (` `), không render lại toàn bộ bàn cờ mỗi khung hình.

2.Xử lý phím mũi tên chuẩn Windows\*\*: Phím mũi tên trả về 2 byte (mã đầu `224`, mã sau `72, 80, 75, 77`), code đã xử lý trọn vẹn cả phím W/A/S/D lẫn mũi tên.

3.Chống crash nhập liệu\*\*: Sử dụng `cin.fail()`, `cin.clear()` và `cin.ignore()` đảm bảo khi gõ chữ linh tinh vào menu thì chương trình vẫn yêu cầu nhập lại ổn định.

