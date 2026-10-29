#ifndef PROCESSWATCHER_H
#define PROCESSWATCHER_H

#include <QObject>
#include <QTimer>
#include <QSet>
#include <QVector>
#include "gameentry.h"

// ====================== SO SÁNH VỚI DỰ ÁN GỐC ======================
// Trong GameTimer gốc, "gamerunner.cpp" dùng QProcess::start() để TỰ
// CHẠY game, rồi lắng nghe signal finished() của chính QProcess đó để
// biết khi nào game thoát. Cách đó CHỈ hoạt động nếu app của bạn là
// nơi KHỞI CHẠY game.
//
// Nhưng Limbus Company / Minecraft / ZZZ thường được user tự mở qua
// Steam/launcher - app của bạn không hề "start" tiến trình đó, nên
// không có QProcess nào để lắng nghe finished(). Giải pháp: ProcessWatcher
// không chạy game hộ bạn nữa, mà liên tục HỎI Windows "những tiến trình
// nào đang chạy" (bằng lệnh có sẵn "tasklist"), rồi tự so khớp xem có
// game nào trong danh sách theo dõi đang chạy hay không.
//
// Dùng lệnh "tasklist"/"taskkill" (qua QProcess) thay vì gọi thẳng
// WinAPI (CreateToolhelp32Snapshot, TerminateProcess...) vì:
//   - Đơn giản hơn nhiều cho người mới học C++, không cần link thêm
//     thư viện Windows hay học cấu trúc HANDLE/struct phức tạp.
//   - Về bản chất làm ĐÚNG những gì Task Manager làm (liệt kê +
//     tắt tiến trình) - không đọc bộ nhớ, không hook API, nên an
//     toàn với anti-cheat (xem câu trả lời trong chat).
// =========================================================================
class ProcessWatcher : public QObject
{
    Q_OBJECT
public:
    explicit ProcessWatcher(QObject *parent = nullptr);

    void setWatchedGames(const QVector<GameEntry> &games) { m_games = games; }

    void start(); // bắt đầu quét định kỳ (mỗi 3 giây)
    void stop();

    // true nếu CÓ ÍT NHẤT 1 game trong danh sách đang chạy - dùng để
    // quyết định có nên startCounting()/stopCounting() trên TimeTracker
    bool anyWatchedGameRunning() const { return !m_runningMatches.isEmpty(); }

    // Tắt NGAY LẬP TỨC mọi tiến trình game đang theo dõi (gọi khi hết giờ)
    void killAllWatchedGames();

signals:
    // Phát ra khi chuyển từ "không có game nào chạy" -> "có game chạy"
    void firstGameDetected();
    // Phát ra khi chuyển từ "đang có game chạy" -> "không còn game nào"
    void allGamesStopped();

private slots:
    void scanOnce();

private:
    QTimer m_scanTimer;
    QVector<GameEntry> m_games;

    // Tên process (chữ thường) của các game ĐANG được phát hiện là
    // chạy ở lần quét gần nhất, vd {"javaw.exe", "limbuscompany.exe"}
    QSet<QString> m_runningMatches;

    // Chạy lệnh "tasklist" và trả về danh sách tên tiến trình (chữ
    // thường, không trùng lặp) đang chạy trên máy.
    QSet<QString> currentRunningProcessNames() const;
};

#endif // PROCESSWATCHER_H
