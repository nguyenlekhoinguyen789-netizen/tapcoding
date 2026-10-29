# StudyGuard

App hỗ trợ học tập, giao diện giả terminal/CMD, giới hạn tổng thời gian
chơi game trong ngày (reset lúc 5h sáng), có mã overdrive 6 ký tự/ngày,
và 1 timer tập trung (Focus Timer) tự dọn app gây xao nhãng.

Dự án này được viết dựa trên, và giải thích song song với, dự án
GameTimer gốc (Qt/C++) mà bạn đã gửi trước đó. Mỗi file .h đều có 1
đoạn comment "SO SÁNH VỚI DỰ ÁN GỐC" giải thích nó tương ứng/khác gì
với file nào trong GameTimer.

## Chỉ chạy được trên Windows

App dùng lệnh `tasklist` / `taskkill` (có sẵn trong Windows) để quét và
tắt tiến trình, nên KHÔNG chạy được trên Linux/Mac.

## Cách build

1. Cài Qt 5.15 + Qt Creator (giống hướng dẫn cho dự án GameTimer gốc).
2. Mở file `StudyGuard.pro` bằng Qt Creator.
3. Configure Project -> chọn bộ Kit Qt 5 (MinGW hoặc MSVC đều được).
4. Ctrl+R để build & chạy.

## Kiến trúc tổng quan

```
main.cpp
  └── MainWindow (bộ não, giống mainwindow.cpp gốc nhưng nối nhiều module hơn)
        ├── SaveManager     - đọc/ghi JSON, có checksum chống sửa tay
        │     └── DailyLimitData  - dữ liệu thuần (giống SavingData gốc)
        │           └── GameEntry[] - path + processName + sha256 mỗi game
        ├── TimeTracker     - "bộ não tính giờ": đếm bằng QElapsedTimer,
        │                      reset lúc 5h sáng, phát cảnh báo 5 phút,
        │                      phát hiện đồng hồ hệ thống bị chỉnh lùi
        ├── ProcessWatcher  - quét tiến trình bằng "tasklist" mỗi 3 giây,
        │                      phát hiện game chạy (kể cả qua Steam/launcher),
        │                      tắt bằng "taskkill" khi hết giờ
        ├── DailyCode       - sinh mã 6 ký tự/ngày từ SHA-256(ngày + secret)
        ├── FocusTimer      - đếm ngược phiên tập trung, tự tắt app xao nhãng
        ├── SetupDialog     - màn hình thiết lập lần đầu (thêm game + đặt giờ)
        ├── OverdriveDialog - luồng "xác nhận -> chờ 5s -> nhận mã -> gõ lại
        │                      mã -> chọn overdrive/thoát"
        └── TerminalTheme   - tô style CMD/terminal cho mọi cửa sổ
```

## Những điểm quan trọng cần đọc kỹ (đã giải thích trong code)

- **`timetracker.cpp` - `onTick()`**: vì sao dùng `QElapsedTimer` thay
  vì chỉ trừ dần như `timedisplayer.cpp` gốc.
- **`timetracker.cpp` - `handlePossibleDayRollover()`**: cách phát hiện
  user chỉnh lùi giờ hệ thống để né reset.
- **`savemanager.cpp` - `computeChecksum()`**: cách phát hiện file save
  bị sửa tay (không phải mã hoá, chỉ là tự phát hiện).
- **`processwatcher.cpp` - `currentRunningProcessNames()`**: cách nhận
  diện game chạy qua launcher mà KHÔNG cần đọc bộ nhớ hay hook API
  (an toàn với anti-cheat).
- **`dailycode.cpp`**: vì sao đây là "ma sát tâm lý" (kiểu 1) chứ
  không phải khoá cứng tuyệt đối - xem comment đầu file .h.
- **`mainwindow.cpp` - `initUi()` và `closeEvent()`**: cách ẩn nút [X]
  và chỉ cho thoát thật qua `OverdriveDialog`.

## Những gì CHƯA làm (bạn tự mở rộng theo "Giai đoạn 4" đã bàn)

- Xác minh thêm `installDir()`/`sha256Hash()` của GameEntry khi quét
  tiến trình (hiện chỉ so khớp theo `processName` - xem gợi ý "nâng
  cao" trong `gameentry.h`).
- Thống kê thời gian chơi theo ngày/tuần.
- Đổi secret trong `dailycode.cpp` và `savemanager.cpp` thành chuỗi bí
  mật của riêng bạn trước khi build bản thật.
- Icon .png riêng cho khay hệ thống (hiện dùng icon vẽ bằng code).

## Lưu ý quan trọng

File này chưa được build/test thật (môi trường viết code này không
cài được Qt + qmake để biên dịch thử - không có kết nối mạng tới kho
gói Linux). Khi bạn build trong Qt Creator trên Windows, nếu gặp lỗi
biên dịch, gửi lại thông báo lỗi, mình sẽ sửa cùng bạn.
