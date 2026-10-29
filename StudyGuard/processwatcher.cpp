#include "processwatcher.h"
#include <QProcess>
#include <QStringList>

ProcessWatcher::ProcessWatcher(QObject *parent) : QObject(parent)
{
    connect(&m_scanTimer, &QTimer::timeout, this, &ProcessWatcher::scanOnce);
    m_scanTimer.setInterval(3000); // quét mỗi 3 giây - đủ nhanh, không tốn CPU
}

void ProcessWatcher::start()
{
    m_scanTimer.start();
    scanOnce(); // quét ngay 1 lần đầu, không đợi 3 giây
}

void ProcessWatcher::stop()
{
    m_scanTimer.stop();
}

QSet<QString> ProcessWatcher::currentRunningProcessNames() const
{
    QSet<QString> names;

    QProcess p;
    // "tasklist /FO CSV /NH" -> xuất danh sách dạng CSV, không có
    // dòng tiêu đề (/NH = No Header), vd:
    //   "notepad.exe","1234","Console","1","12,345 K"
    p.start("tasklist", QStringList() << "/FO" << "CSV" << "/NH");

    if (!p.waitForStarted(2000))
        return names; // không chạy được tasklist (không phải Windows?) -> trả về rỗng

    p.waitForFinished(3000);
    QString output = QString::fromLocal8Bit(p.readAllStandardOutput());

    // Mỗi dòng là 1 tiến trình, cột đầu tiên (trong dấu "...") là tên exe
    const QStringList lines = output.split("\n", Qt::SkipEmptyParts);
    for (const QString &line : lines)
    {
        QString trimmed = line.trimmed();
        if (!trimmed.startsWith('"'))
            continue;

        // Lấy phần giữa dấu " đầu tiên và dấu " thứ hai -> đó là tên exe
        int firstQuote = trimmed.indexOf('"');
        int secondQuote = trimmed.indexOf('"', firstQuote + 1);
        if (firstQuote < 0 || secondQuote < 0)
            continue;

        QString exeName = trimmed.mid(firstQuote + 1, secondQuote - firstQuote - 1);
        names.insert(exeName.toLower());
    }

    return names;
}

void ProcessWatcher::scanOnce()
{
    QSet<QString> running = currentRunningProcessNames();

    QSet<QString> matches;
    for (const GameEntry &g : m_games)
    {
        QString pname = g.processName().toLower();
        if (!pname.isEmpty() && running.contains(pname))
            matches.insert(pname);
    }

    bool wasEmpty = m_runningMatches.isEmpty();
    bool nowEmpty = matches.isEmpty();
    m_runningMatches = matches;

    if (wasEmpty && !nowEmpty)
        emit firstGameDetected();
    else if (!wasEmpty && nowEmpty)
        emit allGamesStopped();
}

void ProcessWatcher::killAllWatchedGames()
{
    for (const QString &pname : qAsConst(m_runningMatches))
    {
        // "taskkill /IM tenfile.exe /F" -> tắt BẮT BUỘC (/F = Force)
        // mọi tiến trình có tên đó. Đây là lệnh có sẵn của Windows,
        // tương đương bấm "End Task" trong Task Manager cho từng dòng.
        QProcess::startDetached("taskkill", QStringList() << "/IM" << pname << "/F");
    }
    m_runningMatches.clear();
}
