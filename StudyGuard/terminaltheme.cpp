#include "terminaltheme.h"
#include <QFont>

// Khai báo (định nghĩa) giá trị thật cho các hằng số static đã khai
// báo trong .h. Với "static const" kiểu không phải số nguyên (ở đây
// là QString), C++ bắt buộc phải định nghĩa giá trị ở file .cpp,
// không được định nghĩa ngay trong .h.
const QString TerminalTheme::BACKGROUND = "#0C0C0C";
const QString TerminalTheme::FOREGROUND = "#33FF33";
const QString TerminalTheme::ACCENT     = "#FFFF55";
const QString TerminalTheme::DANGER     = "#FF5555";

QString TerminalTheme::fontFamily()
{
    // Consolas có sẵn trên hầu hết máy Windows. Nếu không có, Qt sẽ
    // tự rơi về 1 font monospace khác gần giống.
    return "Consolas";
}

QString TerminalTheme::baseStyleSheet()
{
    // Đây là 1 chuỗi QSS (Qt Style Sheet) - cú pháp gần giống CSS.
    // Mỗi "selector { thuộc tính: giá trị; }" áp style cho 1 loại
    // widget. Ví dụ "QPushButton" áp cho MỌI QPushButton trong cửa sổ.
    return QString(
        "QWidget {"
        "   background-color: %1;"
        "   color: %2;"
        "   font-family: '%3';"
        "   font-size: 13px;"
        "}"
        "QMainWindow, QDialog {"
        "   background-color: %1;"
        "}"
        "QPushButton {"
        "   background-color: #1A1A1A;"
        "   color: %2;"
        "   border: 1px solid %2;"
        "   padding: 6px 14px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #222222;"
        "}"
        "QPushButton:disabled {"
        "   color: #555555;"
        "   border-color: #555555;"
        "}"
        "QLineEdit, QSpinBox, QPlainTextEdit, QListWidget, QTextEdit {"
        "   background-color: #050505;"
        "   color: %2;"
        "   border: 1px solid #333333;"
        "   selection-background-color: %2;"
        "   selection-color: #000000;"
        "}"
        "QLabel {"
        "   background-color: transparent;"
        "}"
        "QLCDNumber {"
        "   color: %2;"
        "   background-color: #050505;"
        "   border: 1px solid #333333;"
        "}"
        "QListWidget::item:selected {"
        "   background-color: %2;"
        "   color: #000000;"
        "}"
    ).arg(BACKGROUND, FOREGROUND, fontFamily());
}

void TerminalTheme::applyTo(QWidget *rootWidget)
{
    if (rootWidget == nullptr)
        return;

    // setStyleSheet trên widget gốc (vd MainWindow) sẽ tự động áp
    // xuống MỌI widget con bên trong nó - không cần gọi lại cho
    // từng nút bấm, từng ô nhập liệu.
    rootWidget->setStyleSheet(baseStyleSheet());

    QFont f(fontFamily());
    rootWidget->setFont(f);
}
