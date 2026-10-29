#ifndef GAMEENTRY_H
#define GAMEENTRY_H

#include <QString>
#include <utility>

// ====================== SO SÁNH VỚI DỰ ÁN GỐC ======================
// Trong GameTimer gốc, "gamelist.h" chỉ lưu QFileInfo (đường dẫn file)
// cho mỗi game, vì giả định user LUÔN chạy game trực tiếp từ app.
//
// Nhưng Limbus Company / Minecraft / ZZZ thường chạy qua launcher
// (Steam, HoYoPlay, Minecraft Launcher...) - tiến trình thật sự chạy
// game KHÔNG PHẢI là file bạn chọn lúc "Add Game". Vì vậy GameEntry
// ở đây lưu nhiều thông tin hơn để ProcessWatcher (file processwatcher.h)
// có thể nhận diện đúng tiến trình game thật đang chạy.
// =====================================================================
class GameEntry
{
public:
    GameEntry() = default;

    GameEntry(QString displayName,
              QString exePath,
              QString processName,
              QString sha256Hash)
        : m_displayName(std::move(displayName)),
          m_exePath(std::move(exePath)),
          m_processName(std::move(processName)),
          m_sha256Hash(std::move(sha256Hash))
    {
    }

    // --- getter: hàm chỉ đọc, không sửa dữ liệu, nên khai báo "const" ---
    QString displayName() const { return m_displayName; }
    QString exePath()     const { return m_exePath; }
    QString processName() const { return m_processName; } // vd: "javaw.exe"
    QString sha256Hash()  const { return m_sha256Hash; }

    void setDisplayName(const QString &v) { m_displayName = v; }

    // Thư mục chứa file exe, dùng để so khớp "tiến trình đang chạy có
    // nằm trong thư mục cài đặt của game này không" - hữu ích khi
    // processName chung chung (vd nhiều app Java cùng tên javaw.exe).
    QString installDir() const;

private:
    QString m_displayName;
    QString m_exePath;
    QString m_processName;
    QString m_sha256Hash;
};

#endif // GAMEENTRY_H
