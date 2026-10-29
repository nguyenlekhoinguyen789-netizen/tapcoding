#ifndef FOCUSTIMER_H
#define FOCUSTIMER_H

#include <QObject>
#include <QTimer>
#include <QStringList>

// ====================== FOCUS TIMER (POMODORO) ======================
// Tính năng mới, không có trong dự án gốc. Ý tưởng: 1 phiên "tập
// trung" trong X phút, trong lúc đó TỰ ĐỘNG dọn các app gây xao nhãng
// (trình duyệt, Discord, Zalo...) bằng cách tái sử dụng kỹ thuật
// "taskkill" giống ProcessWatcher - nhưng CHỈ áp dụng trong lúc phiên
// tập trung đang chạy, và CHỈ với danh sách app do CHÍNH BẠN khai báo
// là "gây xao nhãng" (distractionProcessNames), KHÔNG đụng tới bất kỳ
// app nào khác.
//
// Khác với TimeTracker (đo thời gian ĐÃ DÙNG, cộng dồn), FocusTimer là
// đồng hồ ĐẾM NGƯỢC đơn giản (giống TimeDisplayer trong GameTimer gốc) -
// bắt đầu ở X phút, mỗi giây -1, hết giờ thì dừng.
// =========================================================================
class FocusTimer : public QObject
{
    Q_OBJECT
public:
    explicit FocusTimer(QObject *parent = nullptr);

    void setDistractionProcessNames(const QStringList &names) { m_distractions = names; }

    void start(int minutes);
    void stop();  // dừng sớm (user tự bấm huỷ phiên)
    bool isRunning() const { return m_running; }

    int secondsLeft() const { return m_secondsLeft; }

signals:
    void secondsLeftChanged(int secondsLeft);
    void sessionFinished();

private slots:
    void onTick();

private:
    QTimer m_timer;
    int m_secondsLeft = 0;
    bool m_running = false;
    QStringList m_distractions;

    // Tắt mọi app trong m_distractions đang chạy - gọi 1 lần lúc bắt
    // đầu phiên. KHÔNG quét liên tục trong suốt phiên (để đơn giản) -
    // nếu muốn chặn cả app mở LẠI giữa phiên, có thể gọi hàm này mỗi
    // tick, giống cách ProcessWatcher quét định kỳ.
    void closeDistractions();
};

#endif // FOCUSTIMER_H
