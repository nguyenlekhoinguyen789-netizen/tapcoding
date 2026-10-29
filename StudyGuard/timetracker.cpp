#include "timetracker.h"
#include <QDateTime>
#include <QTime>
#include <algorithm>

TimeTracker::TimeTracker(SaveManager *saveManager, QObject *parent)
    : QObject(parent),
      m_saveManager(saveManager)
{
    // "connect" là trái tim của Qt: nối tín hiệu timeout() của
    // m_tickTimer tới slot onTick() của chính object này ("this").
    // Mỗi khi m_tickTimer kêu, onTick() sẽ tự động được gọi.
    connect(&m_tickTimer, &QTimer::timeout, this, &TimeTracker::onTick);
    m_tickTimer.setInterval(1000); // 1000ms = 1 giây/lần
}

QDate TimeTracker::logicalDateFor(const QDateTime &when)
{
    static const QTime kResetTime(5, 0, 0); // mốc reset: 5:00:00 sáng
    if (when.time() < kResetTime)
        return when.date().addDays(-1);
    return when.date();
}

void TimeTracker::loadFromDisk()
{
    if (m_saveManager->hasValidSave())
    {
        m_data = m_saveManager->load();
    }
    else
    {
        // Lần đầu chạy app, hoặc file bị tampered (SaveManager::load()
        // đã tự xử lý trường hợp tampered bằng cách khoá usedSeconds).
        m_data = DailyLimitData();
        m_data.logicalDate = logicalDateFor(QDateTime::currentDateTime());
    }

    m_lastWallClockSeen = QDateTime::currentDateTime();
    handlePossibleDayRollover();
    persist();
}

void TimeTracker::handlePossibleDayRollover()
{
    QDateTime now = QDateTime::currentDateTime();
    QDate todayLogical = logicalDateFor(now);

    if (todayLogical == m_data.logicalDate)
        return; // vẫn cùng 1 ngày logic, không có gì để làm

    if (todayLogical > m_data.logicalDate)
    {
        // Thời gian trôi tới (bình thường) -> sang ngày logic mới
        // thật sự -> reset giới hạn.
        m_data.logicalDate      = todayLogical;
        m_data.usedSeconds      = 0;
        m_data.overdriveSeconds = 0;
        m_data.overdriveCount   = 0;
        m_warningFired           = false;
    }
    else
    {
        // todayLogical < ngày đã lưu -> đồng hồ hệ thống bị LÙI lại.
        // KHÔNG reset gì cả, giữ nguyên usedSeconds cũ, chỉ cảnh báo.
        emit suspiciousClockRollback();
    }
}

void TimeTracker::startCounting()
{
    if (m_isCounting)
        return;
    m_isCounting = true;
    m_elapsed.start(); // bắt đầu đo thời gian thực từ bây giờ
    m_tickTimer.start();
}

void TimeTracker::stopCounting()
{
    m_isCounting = false;
    m_tickTimer.stop();
    persist();
}

void TimeTracker::onTick()
{
    // Thay vì "giả định đã trôi qua đúng 1000ms" (như timedisplayer.cpp
    // gốc làm), ta HỎI LẠI m_elapsed xem THỰC SỰ đã trôi qua bao nhiêu
    // mili-giây. Nếu máy bị lag/ngủ đông, số này có thể là 3000ms,
    // 60000ms... và ta cộng dồn đúng số giây thực đó, không phải luôn
    // luôn +1.
    qint64 elapsedMs = m_elapsed.restart(); // đọc rồi RESET mốc đo về 0
    qint64 elapsedSec = elapsedMs / 1000;
    if (elapsedSec < 1)
        elapsedSec = 1; // tối thiểu +1s mỗi tick, tránh đứng yên nếu lỡ đọc ra 0

    handlePossibleDayRollover();

    m_data.usedSeconds += elapsedSec;

    qint64 remain = m_data.remainingSeconds();
    qint64 budget = totalBudgetSeconds();

    emit secondsTicked(remain, budget);

    if (!m_warningFired && remain <= 5 * 60 && remain > 0)
    {
        m_warningFired = true;
        emit fiveMinuteWarning();
    }

    if (remain <= 0)
    {
        stopCounting();
        emit limitReached();
    }

    // Lưu định kỳ (mỗi tick = mỗi giây có hơi nhiều, nhưng file JSON
    // rất nhỏ nên không đáng lo - xem lại câu trả lời về "chiếm ổ đĩa"
    // trong chat: ghi đè 1 file vài trăm byte liên tục KHÔNG làm phình
    // ổ đĩa, khác hẳn với việc APPEND log liên tục).
    persist();
}

void TimeTracker::applyOverdrive(qint64 extraSeconds)
{
    m_data.overdriveSeconds += extraSeconds;
    m_data.overdriveCount   += 1;
    m_warningFired = false; // cho phép cảnh báo 5 phút lại khi gần hết budget mới
    persist();
}

void TimeTracker::persist()
{
    m_saveManager->save(m_data);
}
