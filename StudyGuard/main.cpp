#include "mainwindow.h"
#include <QApplication>

// main.cpp giống hệt vai trò file main.cpp của dự án GameTimer gốc:
// - tạo 1 QApplication (bắt buộc phải có DUY NHẤT 1 cái, quản lý toàn
//   bộ vòng lặp sự kiện: chuột, bàn phím, timer...)
// - tạo cửa sổ chính
// - show() nó lên
// - gọi a.exec() để bắt đầu vòng lặp sự kiện. Chương trình sẽ "đứng"
//   ở dòng này, chạy mãi cho tới khi có lệnh thoát (qApp->quit()),
//   lúc đó a.exec() mới trả về và chương trình kết thúc thật sự.
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Đặt tên tổ chức/ứng dụng - Qt dùng 2 cái này để biết nên lưu
    // QSettings (nếu sau này bạn dùng) ở đâu trong registry/AppData.
    QApplication::setOrganizationName("StudyGuard");
    QApplication::setApplicationName("StudyGuard");

    MainWindow w;
    w.show();

    return a.exec();
}
