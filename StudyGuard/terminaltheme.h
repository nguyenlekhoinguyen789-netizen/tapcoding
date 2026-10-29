#ifndef TERMINALTHEME_H
#define TERMINALTHEME_H

#include <QString>
#include <QWidget>

// TerminalTheme không phải là 1 class Qt bình thường (không kế thừa
// QObject, không có Q_OBJECT) - nó chỉ là một tập hợp các hàm "static"
// (hàm dùng chung, không cần tạo object) để tô màu giao diện giống
// CMD/Terminal của Windows cho mọi widget trong app.
//
// Kỹ thuật dùng: Qt StyleSheet (QSS) - viết gần giống CSS, áp dụng
// bằng widget->setStyleSheet("..."). Đây là cách NHANH và AN TOÀN
// nhất để đổi giao diện, không cần tự vẽ (paintEvent) từng widget.
class TerminalTheme
{
public:
    // Màu nền đen, chữ xanh lá, viền phẳng - style "CMD cổ điển"
    static const QString BACKGROUND;   // "#0C0C0C"
    static const QString FOREGROUND;   // "#33FF33"
    static const QString ACCENT;       // màu cảnh báo / nhấn mạnh
    static const QString DANGER;       // màu đỏ cảnh báo nguy hiểm

    // Font monospace giống Consolas của CMD
    static QString fontFamily();

    // Áp style cho toàn bộ cửa sổ (gọi 1 lần ở MainWindow / mỗi QDialog)
    static void applyTo(QWidget *rootWidget);

    // Trả về đoạn QSS dùng chung, để bạn có thể nối thêm QSS riêng
    // cho từng widget cụ thể nếu muốn.
    static QString baseStyleSheet();
};

#endif // TERMINALTHEME_H
