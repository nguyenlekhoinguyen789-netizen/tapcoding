#ifndef DAILYCODE_H
#define DAILYCODE_H

#include <QString>
#include <QDate>

// ====================== OVERDRIVE CODE - "KIỂU 1" ======================
// Đây KHÔNG có trong dự án gốc GameTimer - là tính năng mới bạn yêu cầu.
//
// "Kiểu 1" nghĩa là: secret key nằm NGAY TRONG app (biến kSecret bên
// dưới), code được tính HOÀN TOÀN OFFLINE, không cần mạng. Vì secret
// nằm trong chính app, về lý thuyết NGƯỜI VIẾT APP (bạn) luôn có thể
// tìm lại secret này (đọc code, hoặc decompile file .exe). Tác dụng
// thật sự của code không phải là "khoá cứng không thể phá", mà là
// MA SÁT TÂM LÝ - 1 bước dừng lại suy nghĩ trước khi phá giới hạn do
// chính bạn đặt ra. Xem thêm giải thích trong overdrivedialog.cpp.
//
// Thuật toán: băm SHA-256 của (ngày logic + secret) -> lấy 6 ký tự hex
// đầu tiên, viết hoa. Vì SHA-256 là hàm 1 chiều và "nhạy": đổi ngày là
// code đổi hoàn toàn khác, không đoán được ngày mai từ code hôm nay.
// =========================================================================
class DailyCode
{
public:
    explicit DailyCode(QString secret = QString());

    // Code 6 ký tự cho 1 ngày logic cụ thể (xem TimeTracker::logicalDateFor)
    QString codeForDate(const QDate &logicalDate) const;

    // So sánh code user nhập với code đúng của ngày hôm nay (không phân
    // biệt hoa/thường, bỏ khoảng trắng thừa)
    bool verify(const QString &userInput, const QDate &logicalDate) const;

private:
    QString m_secret;
};

#endif // DAILYCODE_H
