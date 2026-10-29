#include "dailylimitdata.h"
#include <algorithm>

qint64 DailyLimitData::remainingSeconds() const
{
    qint64 budget = totalLimitSeconds + overdriveSeconds;
    qint64 remain = budget - usedSeconds;
    // std::max đảm bảo không bao giờ trả về số âm (vd nếu usedSeconds
    // lỡ vượt budget 1 chút do làm tròn thời gian).
    return std::max<qint64>(0, remain);
}
