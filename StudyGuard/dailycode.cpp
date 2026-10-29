#include "dailycode.h"
#include <QCryptographicHash>

DailyCode::DailyCode(QString secret)
    : m_secret(secret.isEmpty() ? QStringLiteral("StudyGuard-change-this-secret-please") : secret)
{
    // Nếu bạn muốn, có thể đổi secret mặc định ở trên thành 1 chuỗi
    // ngẫu nhiên dài của riêng bạn trước khi build app - không bắt
    // buộc để code hoạt động, nhưng giúp code không bị đoán ra nếu ai
    // đó thử các secret "phổ biến".
}

QString DailyCode::codeForDate(const QDate &logicalDate) const
{
    QString seed = logicalDate.toString("yyyy-MM-dd") + "|" + m_secret;

    QByteArray hash = QCryptographicHash::hash(seed.toUtf8(), QCryptographicHash::Sha256);

    // toHex() trả về chuỗi hex (chỉ gồm 0-9 và a-f), ví dụ "3f9a21...".
    // .toUpper() viết hoa cho đẹp mắt, .left(6) lấy 6 ký tự đầu.
    QString hex = QString(hash.toHex()).toUpper();
    return hex.left(6);
}

bool DailyCode::verify(const QString &userInput, const QDate &logicalDate) const
{
    QString correct = codeForDate(logicalDate);
    QString cleaned = userInput.trimmed().toUpper();
    return cleaned == correct;
}
