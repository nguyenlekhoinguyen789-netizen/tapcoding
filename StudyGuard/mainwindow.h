#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QCloseEvent>

#include "savemanager.h"
#include "timetracker.h"
#include "processwatcher.h"
#include "dailycode.h"
#include "focustimer.h"

class QListWidget;
class QLabel;
class QLCDNumber;
class QPushButton;
class QSpinBox;
class QPlainTextEdit;

// ====================== BỘ NÃO CỦA APP (giống mainwindow.h gốc) ======================
// So với MainWindow trong GameTimer gốc (nối GameList + TimeDisplayer +
// DataSaver), MainWindow ở đây nối NHIỀU module hơn:
//
//   SaveManager    <-> TimeTracker (đọc/ghi + tính giờ, có chống gian lận)
//   ProcessWatcher  -> phát hiện game chạy (kể cả qua launcher)
//   DailyCode       -> sinh mã 6 ký tự/ngày
//   FocusTimer      -> đếm ngược phiên tập trung
//
// closeEvent() được override giống hệt ý tưởng của bản gốc (ẩn xuống
// khay thay vì thoát thật), nhưng thêm: nút [X] trên thanh tiêu đề bị
// ẨN LUÔN (không chỉ chặn sự kiện) bằng cách bỏ cờ Qt::WindowCloseButtonHint -
// xem initUI(). Muốn thoát thật, BẮT BUỘC phải qua OverdriveDialog với
// mã đúng (requestRealQuit()).
// ===========================================================================================
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onFiveMinuteWarning();
    void onLimitReached();
    void onSecondsTicked(qint64 remaining, qint64 budget);
    void onSuspiciousClockRollback();

    void onFirstGameDetected();
    void onAllGamesStopped();

    void onRunCodeMenuTriggered();
    void onTrayActivated(QSystemTrayIcon::ActivationReason reason);

    void onStartFocusClicked();
    void onStopFocusClicked();
    void onFocusSecondsLeftChanged(int secondsLeft);
    void onFocusSessionFinished();

private:
    SaveManager   *m_saveManager;
    TimeTracker   *m_timeTracker;
    ProcessWatcher *m_processWatcher;
    DailyCode      m_dailyCode;
    FocusTimer    *m_focusTimer;

    QSystemTrayIcon *m_tray;
    bool m_allowRealClose = false; // chỉ true khi OverdriveDialog xác nhận mã đúng + chọn Exit

    // --- UI ---
    QLabel *m_remainingLabel;
    QListWidget *m_gameListWidget;
    QPlainTextEdit *m_consoleLog;

    QSpinBox *m_focusMinutesBox;
    QPushButton *m_startFocusButton;
    QPushButton *m_stopFocusButton;
    QLabel *m_focusStatusLabel;

    void runFirstTimeSetupIfNeeded();
    void initUi();
    void initTray();
    void refreshGameListWidget();
    void logLine(const QString &text); // in 1 dòng ra "console" giả lập (giống terminal)
};

#endif // MAINWINDOW_H
