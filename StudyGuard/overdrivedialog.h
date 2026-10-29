#ifndef OVERDRIVEDIALOG_H
#define OVERDRIVEDIALOG_H

#include <QDialog>
#include <QDate>
#include "dailycode.h"

class QLabel;
class QPushButton;
class QLineEdit;
class QRadioButton;
class QSpinBox;
class QTimer;

// ====================== LUỒNG OVERDRIVE/EXIT - KIỂU 1 ======================
// Không có gì trong dự án gốc giống cái này - đây là tính năng bạn yêu
// cầu. Luồng hoạt động (đúng y như bạn mô tả):
//
//  Bước 1: hỏi "Bạn có chắc chắn muốn lấy mã không?" [Có] [Không]
//  Bước 2: nếu [Có] -> ẩn 2 nút đó, đếm ngược 5 giây, nút "Nhận mã"
//          CHỈ hiện/bật SAU khi đếm xong.
//  Bước 3: bấm "Nhận mã" -> hiện mã 6 ký tự của hôm nay (DailyCode).
//  Bước 4: user chọn hành động (Overdrive thêm bao nhiêu phút, hoặc
//          Thoát chương trình) + GÕ LẠI mã vừa thấy vào ô nhập liệu.
//          Việc "GÕ LẠI" (chứ không chỉ bấm nút) là 1 lớp ma sát nữa -
//          tạo thêm 1 khoảnh khắc dừng lại suy nghĩ.
//  Bước 5: nếu gõ đúng mã -> thực hiện hành động đã chọn.
//
// QTimer ở đây dùng để làm bộ đếm ngược HIỂN THỊ (khác với QTimer
// trong timetracker.h dùng để ĐẾM GIỜ CHƠI - cùng 1 class QTimer
// nhưng 2 mục đích hoàn toàn khác nhau, đây là ví dụ về việc 1 công cụ
// (QTimer) có thể tái dùng cho nhiều việc).
// ==============================================================================
class OverdriveDialog : public QDialog
{
    Q_OBJECT
public:
    enum class Action { None, Overdrive, Exit };

    explicit OverdriveDialog(const DailyCode &codeProvider,
                              const QDate &logicalDate,
                              QWidget *parent = nullptr);

    Action resultAction() const { return m_resultAction; }
    int overdriveMinutes() const;

private slots:
    void onConfirmYes();
    void onConfirmNo();
    void onCountdownTick();
    void onReceiveCodeClicked();
    void onApplyClicked();

private:
    const DailyCode &m_codeProvider;
    QDate m_logicalDate;
    QString m_revealedCode;
    Action m_resultAction = Action::None;

    int m_secondsLeft = 5;
    QTimer *m_countdownTimer;

    // --- các widget của từng bước, ẩn/hiện tuỳ theo state hiện tại ---
    QLabel *m_stepLabel;
    QPushButton *m_yesButton;
    QPushButton *m_noButton;

    QPushButton *m_receiveCodeButton;

    QLabel *m_codeLabel;
    QRadioButton *m_overdriveRadio;
    QRadioButton *m_exitRadio;
    QSpinBox *m_overdriveMinutesBox;
    QLineEdit *m_retypeCodeEdit;
    QPushButton *m_applyButton;

    void buildUi();
    void showStepConfirm();
    void showStepCountdown();
    void showStepCodeAndAction();
};

#endif // OVERDRIVEDIALOG_H
