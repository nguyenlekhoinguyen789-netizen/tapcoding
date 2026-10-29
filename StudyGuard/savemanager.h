#ifndef SAVEMANAGER_H
#define SAVEMANAGER_H

#include <QObject>
#include <QString>
#include "dailylimitdata.h"

// ====================== SO SÁNH VỚI DỰ ÁN GỐC ======================
// Đây là class tương đương "datasaver.h/.cpp" trong GameTimer gốc:
// chuyển đổi qua lại giữa DailyLimitData (C++ object) <-> JSON (file
// trên đĩa), y hệt cách DataSaver::save()/load() build QJsonObject
// bằng tay.
//
// Điểm MỚI so với bản gốc: SaveManager còn tính 1 "checksum" (giống
// chữ ký điện tử đơn giản) ghi kèm trong file. Mục đích: nếu user mở
// file .save bằng Notepad rồi tự sửa "usedSeconds" về 0 để gian lận,
// SaveManager sẽ PHÁT HIỆN được (checksum không khớp nữa) - xem giải
// thích trong tin nhắn chat về "lỗ hổng xoá file save".
//
// LƯU Ý: đây KHÔNG phải mã hoá, ai cũng đọc được nội dung file, chỉ là
// cách để tự phát hiện bị sửa tay (giống checksum của file tải về).
// =====================================================================
class SaveManager : public QObject
{
    Q_OBJECT
public:
    explicit SaveManager(QObject *parent = nullptr);

    // true nếu tìm thấy file save VÀ checksum hợp lệ (không bị sửa tay)
    bool hasValidSave() const;

    // true nếu file save tồn tại nhưng checksum SAI (bị sửa tay / hỏng)
    bool isTampered() const;

    DailyLimitData load() const;
    void save(const DailyLimitData &data) const;

    void setSavePath(const QString &path) { m_savePath = path; }
    void setSaveFileName(const QString &name) { m_saveFileName = name; }

private:
    QString m_savePath   = "./data";
    QString m_saveFileName = ".sgsave";

    QString fullFilePath() const;

    // Tính checksum từ nội dung dữ liệu + 1 "salt" bí mật cố định
    // trong code (xem .cpp). Vì salt nằm trong binary nên đây CHỈ
    // chặn được sửa tay bằng Notepad, KHÔNG chặn được người biết đọc
    // code - đây là giới hạn đã biết trước, không phải lỗi.
    QString computeChecksum(const DailyLimitData &data) const;
};

#endif // SAVEMANAGER_H
