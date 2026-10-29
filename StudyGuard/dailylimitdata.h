#ifndef DAILYLIMITDATA_H
#define DAILYLIMITDATA_H

#include <QDate>
#include <QVector>
#include "gameentry.h"

// ====================== SO SÁNH VỚI DỰ ÁN GỐC ======================
// Đây là class tương đương "savingdata.h" trong GameTimer gốc: nó CHỈ
// chứa dữ liệu (không có logic xử lý) - toàn bộ đều là các trường
// private + hàm get/set đơn giản. Việc tách riêng "dữ liệu thuần"
// khỏi "logic xử lý" (sẽ nằm ở TimeTracker) là một nguyên tắc thiết
// kế tốt: dễ test, dễ đọc, dễ đổi cách lưu (JSON, SQLite,...) sau này
// mà không đụng tới logic.
//
// Khác với SavingData gốc (chỉ có 1 QTime tổng + 1 QTime còn lại),
// ở đây mình dùng SỐ GIÂY (qint64) thay vì QTime, vì:
//   - Cộng trừ số nguyên đơn giản và không giới hạn 24h như QTime.
//   - usedSeconds là "đã dùng bao nhiêu", dễ cộng dồn hơn "còn lại
//     bao nhiêu" (leftTime) - đỡ phải trừ ngược mỗi lần tick.
// =====================================================================
class DailyLimitData
{
public:
    DailyLimitData() = default;

    // "ngày logic" hiện tại (xem logicalDateForNow() trong timetracker.h) -
    // KHÔNG phải lúc nào cũng trùng QDate::currentDate(), vì ngày logic
    // reset lúc 5h sáng chứ không phải 0h.
    QDate logicalDate;

    qint64 totalLimitSeconds = 3 * 3600;   // tổng giờ được phép / ngày (mặc định 3 tiếng)
    qint64 usedSeconds       = 0;          // đã dùng trong ngày logic hiện tại
    qint64 overdriveSeconds  = 0;          // đã cộng thêm hôm nay qua overdrive
    int    overdriveCount    = 0;          // đã overdrive bao nhiêu lần hôm nay (để giới hạn)

    QVector<GameEntry> games;

    // Giây còn lại = (tổng + overdrive) - đã dùng, không bao giờ âm
    qint64 remainingSeconds() const;
};

#endif // DAILYLIMITDATA_H
