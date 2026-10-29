#include "focustimer.h"
#include <QProcess>

FocusTimer::FocusTimer(QObject *parent) : QObject(parent)
{
    connect(&m_timer, &QTimer::timeout, this, &FocusTimer::onTick);
    m_timer.setInterval(1000);
}

void FocusTimer::start(int minutes)
{
    m_secondsLeft = minutes * 60;
    m_running = true;
    closeDistractions();
    m_timer.start();
    emit secondsLeftChanged(m_secondsLeft);
}

void FocusTimer::stop()
{
    m_running = false;
    m_timer.stop();
}

void FocusTimer::onTick()
{
    m_secondsLeft -= 1;
    emit secondsLeftChanged(m_secondsLeft);

    // Dọn lại distraction mỗi tick (mỗi giây) để chặn cả trường hợp
    // user mở lại app bị chặn giữa phiên. "taskkill" chạy rất nhanh
    // (vài mili-giây) nên gọi mỗi giây không gây giật lag.
    closeDistractions();

    if (m_secondsLeft <= 0)
    {
        stop();
        emit sessionFinished();
    }
}

void FocusTimer::closeDistractions()
{
    for (const QString &procName : m_distractions)
    {
        if (procName.trimmed().isEmpty())
            continue;
        QProcess::startDetached("taskkill", QStringList() << "/IM" << procName << "/F");
    }
}
