#ifndef TIMETRACKER_H
#define TIMETRACKER_H

#include <QObject>
#include <QTimer>
#include <QElapsedTimer>
#include <QDateTime>
#include "dailylimitdata.h"
#include "savemanager.h"

// ====================== ĐÂY LÀ "BỘ NÃO" CỦA APP ======================
// TimeTracker gộp vai trò của CẢ "timedisplayer.h" (đếm giờ) LẪN 1 phần
// "datamanager.h" (tính giờ còn lại, lưu/đọc) trong bản gốc, nhưng xử
// lý kỹ hơn 3 điểm mà bản gốc KHÔNG có:
//
//  1) Đếm bằng QElapsedTimer (đồng hồ "đơn điệu" của hệ điều hành,
//     KHÔNG bị ảnh hưởng nếu user chỉnh giờ hệ thống) thay vì chỉ trừ
//     dần 1 QTime như "timedisplayer.cpp" gốc làm.
//     -> bản gốc: mỗi 100ms tự trừ 100ms vào _nowTime, không quan tâm
//        thời gian THỰC đã trôi qua bao nhiêu - nếu máy bị treo/ngủ
//        đông (sleep) vài phút, _nowTime vẫn chỉ trừ đúng số tick đã
//        chạy, không phản ánh đúng thời gian thực.
//     -> ở đây: mỗi tick ta hỏi QElapsedTimer "đã trôi qua bao nhiêu
//        mili-giây thực kể từ lần tick trước", nên chính xác hơn.
//
//  2) "Ngày logic" reset lúc 5h sáng (logicalDateForNow()), không
//     phải lúc 0h như QDate::currentDate() mặc định.
//
//  3) Tự phát hiện nếu đồng hồ hệ thống bị chỉnh LÙI lại (để né reset
//     hoặc kéo dài phiên chơi) - xem detectClockRollback().
// =========================================================================
class TimeTracker : public QObject
{
    Q_OBJECT
public:
    explicit TimeTracker(SaveManager *saveManager, QObject *parent = nullptr);

    // Gọi 1 lần lúc khởi động app: đọc file save, xử lý đổi ngày logic
    void loadFromDisk();

    // Bắt đầu / dừng đếm giờ (gọi khi có game bắt đầu chạy / tất cả
    // game đã thoát) - giống connect(firstGameStarted,timer,start) ở
    // mainwindow.cpp gốc.
    void startCounting();
    void stopCounting();
    bool isCounting() const { return m_isCounting; }

    qint64 remainingSeconds() const { return m_data.remainingSeconds(); }
    qint64 totalBudgetSeconds() const { return m_data.totalLimitSeconds + m_data.overdriveSeconds; }

    const DailyLimitData &data() const { return m_data; }
    DailyLimitData &dataMutable() { return m_data; }

    // Cộng thêm thời gian khi overdrive thành công (xem overdriavedialog)
    void applyOverdrive(qint64 extraSeconds);

    void persist(); // ghi xuống đĩa ngay lập tức

    // Trả về "ngày logic" của 1 thời điểm: nếu giờ hiện tại < 5:00 sáng
    // thì vẫn tính là thuộc về ngày hôm trước. Public vì MainWindow và
    // OverdriveDialog cũng cần biết "ngày logic hôm nay" để sinh/so
    // mã 6 ký tự đúng ngày (xem dailycode.h).
    static QDate logicalDateFor(const QDateTime &when);

signals:
    // Phát ra đúng 1 LẦN khi còn 5 phút - để MainWindow hiện cảnh báo
    void fiveMinuteWarning();
    // Phát ra khi hết giờ hoàn toàn - MainWindow sẽ gọi kill tất cả game
    void limitReached();
    // Phát mỗi giây để UI cập nhật đồng hồ hiển thị
    void secondsTicked(qint64 remaining, qint64 budget);
    // Phát nếu phát hiện đồng hồ hệ thống bị chỉnh lùi (nghi gian lận)
    void suspiciousClockRollback();

private slots:
    void onTick();

private:
    SaveManager *m_saveManager;
    DailyLimitData m_data;

    QTimer m_tickTimer;          // tick mỗi 1 giây khi đang đếm
    QElapsedTimer m_elapsed;     // đo thời gian THỰC trôi qua giữa 2 tick
    bool m_isCounting = false;
    bool m_warningFired = false; // tránh phát fiveMinuteWarning() lặp lại

    QDateTime m_lastWallClockSeen; // mốc giờ hệ thống lần gần nhất ghi nhận

    // Kiểm tra: nếu ngày logic hiện tại khác ngày logic đã lưu, có 2
    // trường hợp:
    //   - Ngày mới LỚN HƠN ngày cũ -> hợp lệ, reset giới hạn.
    //   - Ngày mới NHỎ HƠN ngày cũ -> đồng hồ bị chỉnh lùi -> KHÔNG
    //     reset, giữ nguyên usedSeconds cũ, phát suspiciousClockRollback().
    void handlePossibleDayRollover();
};

#endif // TIMETRACKER_H
