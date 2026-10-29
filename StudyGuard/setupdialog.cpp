#include "setupdialog.h"
#include "terminaltheme.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include <QFileInfo>
#include <QFile>
#include <QCryptographicHash>
#include <QMessageBox>

SetupDialog::SetupDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("StudyGuard - Thiet lap lan dau");
    buildUi();
    TerminalTheme::applyTo(this); // tô style terminal cho dialog này
}

void SetupDialog::buildUi()
{
    // ---- Mỗi dòng dưới đây tạo 1 widget trên HEAP (new) ----
    // Vì ta truyền "this" làm parent cho hầu hết widget (qua layout,
    // Qt tự set parent khi addWidget), nên KHÔNG CẦN tự delete chúng:
    // khi SetupDialog bị huỷ, Qt tự huỷ toàn bộ widget con theo - đây
    // chính là cơ chế "parent-child ownership" đã giải thích ở lần
    // trước khi đọc code GameTimer gốc.
    m_hintLabel = new QLabel(
        "C:\\> them cac game muon kiem soat, roi dat tong thoi gian/ngay.\n"
        "Gioi han se duoc tinh CHUNG cho tat ca game ban them.", this);

    m_listWidget = new QListWidget(this);

    m_addButton    = new QPushButton("[ them game ]", this);
    m_removeButton = new QPushButton("[ xoa game da chon ]", this);

    m_hourBox = new QSpinBox(this);
    m_hourBox->setRange(0, 23);
    m_hourBox->setValue(3); // mặc định 3 tiếng, giống GameTimer gốc

    m_minuteBox = new QSpinBox(this);
    m_minuteBox->setRange(0, 59);

    m_finishButton = new QPushButton("[ bat dau ]", this);

    // ---- Layout: QVBoxLayout xếp widget theo chiều dọc, giống cách
    // mainwindow.ui gốc dùng QVBoxLayout "layoutList" ----
    auto *gameButtonsRow = new QHBoxLayout();
    gameButtonsRow->addWidget(m_addButton);
    gameButtonsRow->addWidget(m_removeButton);

    auto *timeRow = new QHBoxLayout();
    timeRow->addWidget(new QLabel("Tong thoi gian/ngay:", this));
    timeRow->addWidget(m_hourBox);
    timeRow->addWidget(new QLabel("gio", this));
    timeRow->addWidget(m_minuteBox);
    timeRow->addWidget(new QLabel("phut", this));

    auto *mainLayout = new QVBoxLayout(this); // "this" -> layout này áp lên chính dialog
    mainLayout->addWidget(m_hintLabel);
    mainLayout->addWidget(m_listWidget);
    mainLayout->addLayout(gameButtonsRow);
    mainLayout->addLayout(timeRow);
    mainLayout->addWidget(m_finishButton);

    resize(480, 360);

    // connect kiểu "function pointer" (Qt5 trở lên) - an toàn hơn
    // kiểu SIGNAL()/SLOT() cũ (chuỗi ký tự) mà bản GameTimer gốc dùng,
    // vì lỗi gõ sai tên hàm sẽ bị phát hiện NGAY LÚC BIÊN DỊCH thay vì
    // chỉ lặng lẽ không hoạt động lúc chạy.
    connect(m_addButton,    &QPushButton::clicked, this, &SetupDialog::onAddGameClicked);
    connect(m_removeButton, &QPushButton::clicked, this, &SetupDialog::onRemoveGameClicked);
    connect(m_finishButton, &QPushButton::clicked, this, &SetupDialog::onFinishClicked);
}

QString SetupDialog::hashOfFile(const QString &path)
{
    QFile file(path);
    if (!file.open(QFile::ReadOnly))
        return QString();

    // QCryptographicHash có thể "nạp" dữ liệu theo từng khúc (addData)
    // thay vì đọc nguyên file vào RAM 1 lần - quan trọng với file lớn
    // (1 file game .exe có thể vài trăm MB).
    QCryptographicHash hasher(QCryptographicHash::Sha256);
    while (!file.atEnd())
    {
        hasher.addData(file.read(1024 * 1024)); // đọc từng 1MB
    }
    return QString(hasher.result().toHex());
}

void SetupDialog::onAddGameClicked()
{
    QString path = QFileDialog::getOpenFileName(
        this, "Chon file game (.exe)", "", "Games (*.exe)");

    if (path.isEmpty())
        return; // user bấm Cancel -> không thêm gì cả (khác 1 lỗi nhỏ ở bản gốc!)

    QFileInfo fi(path);
    QString processName = fi.fileName().toLower(); // vd "limbuscompany.exe"
    QString hash = hashOfFile(path);

    GameEntry entry(fi.completeBaseName(), path, processName, hash);
    m_games.append(entry);

    m_listWidget->addItem(entry.displayName() + "   (" + processName + ")");
}

void SetupDialog::onRemoveGameClicked()
{
    int row = m_listWidget->currentRow();
    if (row < 0)
        return;

    delete m_listWidget->takeItem(row); // takeItem lấy ra khỏi list nhưng
                                         // KHÔNG tự xoá object -> phải tự delete
    m_games.remove(row);
}

qint64 SetupDialog::chosenTotalSeconds() const
{
    return static_cast<qint64>(m_hourBox->value()) * 3600
         + static_cast<qint64>(m_minuteBox->value()) * 60;
}

void SetupDialog::onFinishClicked()
{
    if (m_games.isEmpty())
    {
        QMessageBox::information(this, "", "Them it nhat 1 game truoc khi bat dau.");
        return;
    }
    accept(); // đóng dialog với kết quả QDialog::Accepted - giống bấm OK
}
