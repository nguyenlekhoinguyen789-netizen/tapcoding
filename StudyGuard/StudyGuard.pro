#-------------------------------------------------
# StudyGuard.pro
#
# File .pro là file cấu hình cho "qmake" - công cụ của Qt
# đọc file này rồi tự sinh ra Makefile để biên dịch project.
# Nó giống hệt vai trò của GameTimer.pro trong dự án gốc bạn gửi,
# chỉ là mình thêm các file .cpp/.h mới vào đây.
#-------------------------------------------------

QT       += core gui

# Qt5 trở lên thì module "widgets" (QPushButton, QDialog,...) phải
# khai báo riêng thay vì nằm chung trong "gui" như Qt4.
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = StudyGuard
TEMPLATE = app

# C++11 trở lên để dùng auto, nullptr, range-based for, lambda...
CONFIG += c++11

# ỨNG DỤNG NÀY CHỈ CHẠY TRÊN WINDOWS vì nó gọi "tasklist"/"taskkill"
# là 2 lệnh có sẵn trong Windows để liệt kê/tắt tiến trình.
# Dòng dưới chỉ để ghi chú, không có tác dụng kỹ thuật:
# win32: message("Building for Windows - OK")

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    terminaltheme.cpp \
    gameentry.cpp \
    dailylimitdata.cpp \
    savemanager.cpp \
    timetracker.cpp \
    processwatcher.cpp \
    dailycode.cpp \
    setupdialog.cpp \
    overdrivedialog.cpp \
    focustimer.cpp

HEADERS += \
    mainwindow.h \
    terminaltheme.h \
    gameentry.h \
    dailylimitdata.h \
    savemanager.h \
    timetracker.h \
    processwatcher.h \
    dailycode.h \
    setupdialog.h \
    overdrivedialog.h \
    focustimer.h
