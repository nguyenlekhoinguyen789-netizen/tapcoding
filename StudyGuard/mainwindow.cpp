#include "mainwindow.h"
#include "terminaltheme.h"
#include "setupdialog.h"
#include "overdrivedialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QPlainTextEdit>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QPixmap>
#include <QPainter>
#include <QDateTime>
#include <QFont>
#include <QColor>
#include <cstdlib> // std::exit

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    m_saveManager = new SaveManager(this);

    runFirstTimeSetupIfNeeded(); // có thể gọi std::exit() nếu user huỷ setup

    m_timeTracker = new TimeTracker(m_saveManager, this);
    m_timeTracker->loadFromDisk();

    m_processWatcher = new ProcessWatcher(this);
    m_processWatcher->setWatchedGames(m_timeTracker->data().games);

    m_focusTimer = new FocusTimer(this);
    // Danh sách app "gây xao nhãng" mặc định - bạn có thể đổi thành
    // app bạn hay bị phân tâm, vd thêm "discord.exe", "zalo.exe"...
    m_focusTimer->setDistractionProcessNames({"chrome.exe", "msedge.exe"});

    initUi();
    initTray();

    // ---- nối tín hiệu TimeTracker -> xử lý trên UI ----
    connect(m_timeTracker, &TimeTracker::fiveMinuteWarning, this, &MainWindow::onFiveMinuteWarning);
    connect(m_timeTracker, &TimeTracker::limitReached,      this, &MainWindow::onLimitReached);
    connect(m_timeTracker, &TimeTracker::secondsTicked,     this, &MainWindow::onSecondsTicked);
    connect(m_timeTracker, &TimeTracker::suspiciousClockRollback, this, &MainWindow::onSuspiciousClockRollback);

    // ---- nối tín hiệu ProcessWatcher -> bật/tắt đếm giờ ----
    connect(m_processWatcher, &ProcessWatcher::firstGameDetected, this, &MainWindow::onFirstGameDetected);
    connect(m_processWatcher, &ProcessWatcher::allGamesStopped,   this, &MainWindow::onAllGamesStopped);

    // ---- nối tín hiệu FocusTimer -> UI ----
    connect(m_focusTimer, &FocusTimer::secondsLeftChanged, this, &MainWindow::onFocusSecondsLeftChanged);
    connect(m_focusTimer, &FocusTimer::sessionFinished,    this, &MainWindow::onFocusSessionFinished);

    m_processWatcher->start();

    onSecondsTicked(m_timeTracker->remainingSeconds(), m_timeTracker->totalBudgetSeconds());
    refreshGameListWidget();
    logLine("StudyGuard da khoi dong. Dang theo doi " +
            QString::number(m_timeTracker->data().games.size()) + " game.");
}

void MainWindow::runFirstTimeSetupIfNeeded()
{
    if (m_saveManager->hasValidSave())
        return; // đã có dữ liệu từ trước -> không cần setup lại

    SetupDialog dialog;
    if (dialog.exec() != QDialog::Accepted)
    {
        // User đóng dialog mà chưa hoàn tất setup -> app không có dữ
        // liệu gì để chạy, nên thoát luôn. std::exit() ở đây chấp nhận
        // được vì CHƯA có gì cần dọn dẹp (chưa mở file, chưa tạo timer).
        std::exit(0);
    }

    DailyLimitData data;
    data.logicalDate       = TimeTracker::logicalDateFor(QDateTime::currentDateTime());
    data.totalLimitSeconds = dialog.chosenTotalSeconds();
    data.games              = dialog.chosenGames();

    m_saveManager->save(data);
}

void MainWindow::initUi()
{
    setWindowTitle("StudyGuard");

    // ====== ẨN NÚT [X] TRÊN THANH TIÊU ĐỀ ======
    // Qt::WindowCloseButtonHint là 1 "cờ" (flag) nói với hệ điều hành
    // "cửa sổ này có nút đóng". Bỏ cờ này đi (dùng toán tử bitwise AND
    // với PHỦ ĐỊNH của nó: "& ~cờ") thì Windows sẽ không vẽ nút [X]
    // nữa. Đây KHÁC với việc "chặn sự kiện đóng" (closeEvent) - cách
    // này ẩn hẳn nút đi, còn closeEvent bên dưới vẫn xử lý trường hợp
    // user bấm Alt+F4.
    setWindowFlags(windowFlags() & ~Qt::WindowCloseButtonHint);

    auto *central = new QWidget(this);
    setCentralWidget(central);

    m_remainingLabel = new QLabel(this);
    QFont bigFont = m_remainingLabel->font();
    bigFont.setPointSize(22);
    bigFont.setBold(true);
    m_remainingLabel->setFont(bigFont);
    m_remainingLabel->setAlignment(Qt::AlignCenter);

    m_gameListWidget = new QListWidget(this);
    m_gameListWidget->setFixedHeight(100);

    m_consoleLog = new QPlainTextEdit(this);
    m_consoleLog->setReadOnly(true);
    m_consoleLog->setFixedHeight(120);

    // --- khu vực Focus Timer ---
    m_focusMinutesBox = new QSpinBox(this);
    m_focusMinutesBox->setRange(1, 180);
    m_focusMinutesBox->setValue(25); // mặc định kiểu Pomodoro: 25 phút

    m_startFocusButton = new QPushButton("[ bat dau tap trung ]", this);
    m_stopFocusButton  = new QPushButton("[ huy phien ]", this);
    m_stopFocusButton->setEnabled(false);

    m_focusStatusLabel = new QLabel("Focus: chua bat dau", this);

    auto *focusRow = new QHBoxLayout();
    focusRow->addWidget(new QLabel("Phien tap trung:", this));
    focusRow->addWidget(m_focusMinutesBox);
    focusRow->addWidget(new QLabel("phut", this));
    focusRow->addWidget(m_startFocusButton);
    focusRow->addWidget(m_stopFocusButton);

    auto *mainLayout = new QVBoxLayout(central);
    mainLayout->addWidget(new QLabel("C:\\StudyGuard> thoi gian con lai hom nay:", this));
    mainLayout->addWidget(m_remainingLabel);
    mainLayout->addWidget(new QLabel("Game dang theo doi:", this));
    mainLayout->addWidget(m_gameListWidget);
    mainLayout->addLayout(focusRow);
    mainLayout->addWidget(m_focusStatusLabel);
    mainLayout->addWidget(new QLabel("Log:", this));
    mainLayout->addWidget(m_consoleLog);

    resize(520, 560);

    connect(m_startFocusButton, &QPushButton::clicked, this, &MainWindow::onStartFocusClicked);
    connect(m_stopFocusButton,  &QPushButton::clicked, this, &MainWindow::onStopFocusClicked);

    TerminalTheme::applyTo(this);
}

void MainWindow::initTray()
{
    // Tạo 1 icon đơn giản bằng code (hình vuông nền đen, chữ ">"
    // màu xanh lá) thay vì nhúng file ảnh qua .qrc như bản gốc -
    // để bạn không cần chuẩn bị file ảnh riêng mới chạy thử được.
    // Nếu muốn dùng icon.png riêng, bạn có thể làm lại y hệt cách
    // imageresource.qrc của GameTimer gốc làm.
    QPixmap pixmap(32, 32);
    pixmap.fill(QColor("#0C0C0C"));
    QPainter painter(&pixmap);
    painter.setPen(QColor("#33FF33"));
    QFont f = painter.font();
    f.setBold(true);
    f.setPointSize(18);
    painter.setFont(f);
    painter.drawText(pixmap.rect(), Qt::AlignCenter, ">");
    painter.end();

    m_tray = new QSystemTrayIcon(QIcon(pixmap), this);
    m_tray->setToolTip("StudyGuard");

    auto *menu = new QMenu();
    auto *showAction = new QAction("Hien cua so", this);
    auto *codeAction = new QAction("Nhap ma (Overdrive / Thoat)", this);

    connect(showAction, &QAction::triggered, this, &QMainWindow::showNormal);
    connect(codeAction, &QAction::triggered, this, &MainWindow::onRunCodeMenuTriggered);

    menu->addAction(showAction);
    menu->addAction(codeAction);
    // CỐ Ý không có action "Thoat" trực tiếp trong menu - cách DUY
    // NHẤT để thoát thật là qua "Nhap ma" -> OverdriveDialog -> chọn
    // "Thoat chuong trinh" -> gõ đúng mã.
    m_tray->setContextMenu(menu);

    connect(m_tray, &QSystemTrayIcon::activated, this, &MainWindow::onTrayActivated);

    m_tray->show();
}

void MainWindow::onTrayActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::DoubleClick)
        showNormal();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (m_allowRealClose)
    {
        event->accept();
        return;
    }

    // Giống hệt ý tưởng closeEvent() trong mainwindow.cpp gốc: không
    // cho đóng thật, chỉ ẩn xuống khay hệ thống.
    hide();
    m_tray->showMessage("StudyGuard", "Van dang chay trong khay he thong.");
    event->ignore();
}

void MainWindow::onRunCodeMenuTriggered()
{
    QDate logicalDate = TimeTracker::logicalDateFor(QDateTime::currentDateTime());

    OverdriveDialog dialog(m_dailyCode, logicalDate, this);
    if (dialog.exec() != QDialog::Accepted)
        return; // user bấm "khong" hoặc đóng dialog giữa chừng -> không làm gì

    if (dialog.resultAction() == OverdriveDialog::Action::Overdrive)
    {
        qint64 extraSeconds = static_cast<qint64>(dialog.overdriveMinutes()) * 60;
        m_timeTracker->applyOverdrive(extraSeconds);
        logLine(QString("Da overdrive them %1 phut.").arg(dialog.overdriveMinutes()));
        onSecondsTicked(m_timeTracker->remainingSeconds(), m_timeTracker->totalBudgetSeconds());
    }
    else if (dialog.resultAction() == OverdriveDialog::Action::Exit)
    {
        m_allowRealClose = true;
        m_timeTracker->persist();
        close();      // bây giờ closeEvent() sẽ accept() vì m_allowRealClose = true
        qApp->quit(); // thoát thật vòng lặp sự kiện
    }
}

void MainWindow::onFirstGameDetected()
{
    logLine("Phat hien game bat dau chay -> bat dau tinh gio.");
    m_timeTracker->startCounting();
}

void MainWindow::onAllGamesStopped()
{
    logLine("Tat ca game da dong -> dung tinh gio.");
    m_timeTracker->stopCounting();
}

void MainWindow::onFiveMinuteWarning()
{
    m_tray->showMessage("StudyGuard", "Con 5 phut truoc khi het gio hom nay!",
                         QSystemTrayIcon::Warning, 8000);
    logLine("CANH BAO: con 5 phut.");
}

void MainWindow::onLimitReached()
{
    logLine("HET GIO - dang tat tat ca game dang theo doi.");
    m_processWatcher->killAllWatchedGames();
    m_tray->showMessage("StudyGuard", "Da het gio - cac game da bi tat.",
                         QSystemTrayIcon::Critical, 8000);
}

void MainWindow::onSuspiciousClockRollback()
{
    logLine("CANH BAO: phat hien dong ho he thong bi chinh lui. Gio choi KHONG duoc reset.");
}

void MainWindow::onSecondsTicked(qint64 remaining, qint64 budget)
{
    Q_UNUSED(budget);
    int h = static_cast<int>(remaining / 3600);
    int m = static_cast<int>((remaining % 3600) / 60);
    int s = static_cast<int>(remaining % 60);
    m_remainingLabel->setText(QString("%1:%2:%3")
        .arg(h, 2, 10, QChar('0'))
        .arg(m, 2, 10, QChar('0'))
        .arg(s, 2, 10, QChar('0')));
}

void MainWindow::refreshGameListWidget()
{
    m_gameListWidget->clear();
    for (const GameEntry &g : m_timeTracker->data().games)
    {
        m_gameListWidget->addItem(g.displayName() + "  (" + g.processName() + ")");
    }
}

void MainWindow::logLine(const QString &text)
{
    QString stamp = QDateTime::currentDateTime().toString("HH:mm:ss");
    m_consoleLog->appendPlainText(QString("[%1] %2").arg(stamp, text));
}

void MainWindow::onStartFocusClicked()
{
    int minutes = m_focusMinutesBox->value();
    m_focusTimer->start(minutes);
    m_startFocusButton->setEnabled(false);
    m_stopFocusButton->setEnabled(true);
    logLine(QString("Bat dau phien tap trung %1 phut.").arg(minutes));
}

void MainWindow::onStopFocusClicked()
{
    m_focusTimer->stop();
    m_startFocusButton->setEnabled(true);
    m_stopFocusButton->setEnabled(false);
    m_focusStatusLabel->setText("Focus: da huy");
    logLine("Da huy phien tap trung.");
}

void MainWindow::onFocusSecondsLeftChanged(int secondsLeft)
{
    int m = secondsLeft / 60;
    int s = secondsLeft % 60;
    m_focusStatusLabel->setText(QString("Focus: con lai %1:%2")
        .arg(m, 2, 10, QChar('0'))
        .arg(s, 2, 10, QChar('0')));
}

void MainWindow::onFocusSessionFinished()
{
    m_startFocusButton->setEnabled(true);
    m_stopFocusButton->setEnabled(false);
    m_focusStatusLabel->setText("Focus: hoan thanh!");
    m_tray->showMessage("StudyGuard", "Phien tap trung da xong!", QSystemTrayIcon::Information, 5000);
    logLine("Phien tap trung hoan thanh.");
}
