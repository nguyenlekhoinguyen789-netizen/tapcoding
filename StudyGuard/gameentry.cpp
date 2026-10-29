#include "gameentry.h"
#include <QFileInfo>

QString GameEntry::installDir() const
{
    // QFileInfo là class tiện ích của Qt để "tách" 1 đường dẫn ra
    // thành các phần: tên file, thư mục chứa nó, đuôi file...
    // Giống hệt cách gamelist.cpp gốc dùng QFileInfo để lấy fileName().
    QFileInfo fi(m_exePath);
    return fi.absolutePath(); // phần thư mục, KHÔNG gồm tên file
}
