#ifndef SETUPDIALOG_H
#define SETUPDIALOG_H

#include <QDialog>
#include <QVector>
#include "gameentry.h"

class QListWidget;
class QSpinBox;
class QPushButton;
class QLabel;

// ====================== SO SÁNH VỚI DỰ ÁN GỐC ======================
// Dự án gốc KHÔNG có màn hình setup riêng: "Add New Game" nằm trong
// menu File, và tổng thời gian đặt qua "InputTimeDialog" (xem
// inputtimedialog.cpp gốc - dùng Qt Designer .ui + các slot tự nối
// on_hourBox_valueChanged...).
//
// Ở đây mình gộp "thêm game" + "đặt tổng thời gian" vào 1 dialog hiện
// ra lúc KHỞI ĐỘNG LẦN ĐẦU (chưa có file save). Dialog này được viết
// HOÀN TOÀN BẰNG CODE (new QLabel(...), new QPushButton(...)...)
// thay vì dùng file .ui của Qt Designer - để bạn thấy rõ: file .ui
// chỉ là 1 cách TIỆN LỢI để không phải gõ code tạo widget bằng tay,
// bản chất nó sinh ra đúng những dòng code new QWidget(...) như vầy.
// =========================================================================
class SetupDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SetupDialog(QWidget *parent = nullptr);

    QVector<GameEntry> chosenGames() const { return m_games; }
    qint64 chosenTotalSeconds() const;

private slots:
    void onAddGameClicked();
    void onRemoveGameClicked();
    void onFinishClicked();

private:
    QVector<GameEntry> m_games;

    QListWidget *m_listWidget;
    QSpinBox *m_hourBox;
    QSpinBox *m_minuteBox;
    QPushButton *m_addButton;
    QPushButton *m_removeButton;
    QPushButton *m_finishButton;
    QLabel *m_hintLabel;

    void buildUi();

    // Tính SHA-256 của 1 file - dùng để nhận diện game chắc chắn hơn
    // (xem giải thích trong gameentry.h và câu trả lời trong chat về
    // "đổi tên file để né danh sách").
    static QString hashOfFile(const QString &path);
};

#endif // SETUPDIALOG_H
