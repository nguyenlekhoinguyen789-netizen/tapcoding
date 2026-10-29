#include "savemanager.h"

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCryptographicHash>

// "Salt" bí mật - chỉ dùng để tính checksum chống sửa tay, không phải
// để bảo mật thật sự (xem giải thích trong .h). Bạn nên đổi chuỗi này
// thành chuỗi ngẫu nhiên của riêng bạn trước khi build.
static const QString kChecksumSalt = "StudyGuard-2026-change-me";

SaveManager::SaveManager(QObject *parent) : QObject(parent)
{
}

QString SaveManager::fullFilePath() const
{
    QDir dir(m_savePath);
    return dir.absoluteFilePath(m_saveFileName);
}

// Hàm phụ (không phải thành viên class) để build mảng JSON cho danh
// sách game - giống hệt cách buildGamesJsonArray() làm trong
// datasaver.cpp gốc, chỉ là giờ mỗi phần tử là 1 object (4 trường)
// thay vì 1 chuỗi string.
static QJsonArray buildGamesArray(const QVector<GameEntry> &games)
{
    QJsonArray arr;
    for (const GameEntry &g : games)
    {
        QJsonObject o;
        o["displayName"]  = g.displayName();
        o["exePath"]      = g.exePath();
        o["processName"]  = g.processName();
        o["sha256"]       = g.sha256Hash();
        arr.append(o);
    }
    return arr;
}

static QVector<GameEntry> gamesFromArray(const QJsonArray &arr)
{
    QVector<GameEntry> games;
    for (const QJsonValue &v : arr)
    {
        QJsonObject o = v.toObject();
        games.append(GameEntry(
            o["displayName"].toString(),
            o["exePath"].toString(),
            o["processName"].toString(),
            o["sha256"].toString()
        ));
    }
    return games;
}

QString SaveManager::computeChecksum(const DailyLimitData &data) const
{
    // Nối tất cả các trường quan trọng thành 1 chuỗi, kèm salt bí mật,
    // rồi băm bằng SHA-256. Chỉ cần đổi 1 ký tự trong dữ liệu gốc là
    // checksum ra kết quả khác hẳn - đó là tính chất của hàm băm.
    QString raw = QString("%1|%2|%3|%4|%5|%6")
            .arg(data.logicalDate.toString("yyyy-MM-dd"))
            .arg(data.totalLimitSeconds)
            .arg(data.usedSeconds)
            .arg(data.overdriveSeconds)
            .arg(data.overdriveCount)
            .arg(kChecksumSalt);

    QByteArray hash = QCryptographicHash::hash(raw.toUtf8(), QCryptographicHash::Sha256);
    return QString(hash.toHex());
}

bool SaveManager::hasValidSave() const
{
    if (!QFile::exists(fullFilePath()))
        return false;
    return !isTampered();
}

bool SaveManager::isTampered() const
{
    QFile file(fullFilePath());
    if (!file.exists())
        return false; // file không tồn tại thì không gọi là "bị sửa", chỉ là chưa có

    if (!file.open(QFile::ReadOnly | QFile::Text))
        return true; // không mở được file coi như hỏng/đáng ngờ

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &err);
    file.close();

    if (err.error != QJsonParseError::NoError)
        return true; // JSON hỏng -> chắc chắn bị sửa tay hoặc hỏng file

    QJsonObject root = doc.object();
    QString savedChecksum = root["checksum"].toString();

    // Build lại 1 DailyLimitData tạm từ JSON rồi tính checksum, so
    // sánh với checksum đã lưu trong file.
    DailyLimitData tmp;
    tmp.logicalDate        = QDate::fromString(root["logicalDate"].toString(), "yyyy-MM-dd");
    tmp.totalLimitSeconds  = root["totalLimitSeconds"].toVariant().toLongLong();
    tmp.usedSeconds        = root["usedSeconds"].toVariant().toLongLong();
    tmp.overdriveSeconds   = root["overdriveSeconds"].toVariant().toLongLong();
    tmp.overdriveCount     = root["overdriveCount"].toInt();

    return computeChecksum(tmp) != savedChecksum;
}

DailyLimitData SaveManager::load() const
{
    DailyLimitData data;

    QFile file(fullFilePath());
    if (!file.open(QFile::ReadOnly | QFile::Text))
        return data; // trả về data mặc định nếu không mở được

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &err);
    file.close();
    if (err.error != QJsonParseError::NoError)
        return data;

    QJsonObject root = doc.object();
    data.logicalDate       = QDate::fromString(root["logicalDate"].toString(), "yyyy-MM-dd");
    data.totalLimitSeconds = root["totalLimitSeconds"].toVariant().toLongLong();
    data.usedSeconds       = root["usedSeconds"].toVariant().toLongLong();
    data.overdriveSeconds  = root["overdriveSeconds"].toVariant().toLongLong();
    data.overdriveCount    = root["overdriveCount"].toInt();
    data.games              = gamesFromArray(root["games"].toArray());

    // NẾU checksum sai (bị sửa tay), ta KHÔNG tin usedSeconds trong
    // file nữa - coi như đã dùng HẾT giới hạn hôm nay, để không cho
    // gian lận bằng cách tự sửa file. Đây là lựa chọn "thà chặn nhầm
    // còn hơn bỏ sót".
    QString savedChecksum = root["checksum"].toString();
    if (computeChecksum(data) != savedChecksum)
    {
        data.usedSeconds = data.totalLimitSeconds + data.overdriveSeconds;
    }

    return data;
}

void SaveManager::save(const DailyLimitData &data) const
{
    QDir dir(m_savePath);
    if (!dir.exists())
        dir.mkpath(".");

    QJsonObject root;
    root["logicalDate"]       = data.logicalDate.toString("yyyy-MM-dd");
    root["totalLimitSeconds"] = data.totalLimitSeconds;
    root["usedSeconds"]       = data.usedSeconds;
    root["overdriveSeconds"]  = data.overdriveSeconds;
    root["overdriveCount"]    = data.overdriveCount;
    root["games"]             = buildGamesArray(data.games);
    root["checksum"]          = computeChecksum(data);

    QFile file(dir.absoluteFilePath(m_saveFileName));
    if (!file.open(QFile::WriteOnly | QFile::Text))
        return;

    QJsonDocument doc(root);
    file.write(doc.toJson());
    file.close();
}
