#include "overdrivedialog.h"
#include "terminaltheme.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QRadioButton>
#include <QSpinBox>
#include <QTimer>
#include <QMessageBox>
#include <QFont>

OverdriveDialog::OverdriveDialog(const DailyCode &codeProvider,
                                  const QDate &logicalDate,
                                  QWidget *parent)
    : QDialog(parent),
      m_codeProvider(codeProvider),
      m_logicalDate(logicalDate)
{
    setWindowTitle("StudyGuard - Overdrive / Thoat");
    setModal(true); // chặn tương tác với cửa sổ chính khi dialog này mở
    buildUi();
    TerminalTheme::applyTo(this);
    showStepConfirm();
}

void OverdriveDialog::buildUi()
{
    m_stepLabel = new QLabel(this);
    m_stepLabel->setWordWrap(true);

    m_yesButton = new QPushButton("[ co, toi chac chan ]", this);
    m_noButton  = new QPushButton("[ khong ]", this);

    m_receiveCodeButton = new QPushButton("[ nhan ma ]", this);
    m_receiveCodeButton->setEnabled(false);

    m_codeLabel = new QLabel(this);
    QFont bigMono = m_codeLabel->font();
    bigMono.setPointSize(28);
    bigMono.setBold(true);
    m_codeLabel->setFont(bigMono);
    m_codeLabel->setAlignment(Qt::AlignCenter);

    m_overdriveRadio = new QRadioButton("Overdrive them thoi gian", this);
    m_exitRadio      = new QRadioButton("Thoat chuong trinh", this);
    m_overdriveRadio->setChecked(true);

    m_overdriveMinutesBox = new QSpinBox(this);
    m_overdriveMinutesBox->setRange(1, 180);
    m_overdriveMinutesBox->setValue(15);

    m_retypeCodeEdit = new QLineEdit(this);
    m_retypeCodeEdit->setPlaceholderText("Go lai 6 ky tu ma vua hien o tren");
    m_retypeCodeEdit->setMaxLength(6);

    m_applyButton = new QPushButton("[ xac nhan ]", this);

    auto *confirmRow = new QHBoxLayout();
    confirmRow->addWidget(m_yesButton);
    confirmRow->addWidget(m_noButton);

    auto *overdriveRow = new QHBoxLayout();
    overdriveRow->addWidget(m_overdriveRadio);
    overdriveRow->addWidget(m_overdriveMinutesBox);
    overdriveRow->addWidget(new QLabel("phut", this));

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(m_stepLabel);
    layout->addLayout(confirmRow);
    layout->addWidget(m_receiveCodeButton);
    layout->addWidget(m_codeLabel);
    layout->addLayout(overdriveRow);
    layout->addWidget(m_exitRadio);
    layout->addWidget(m_retypeCodeEdit);
    layout->addWidget(m_applyButton);

    resize(420, 380);

    connect(m_yesButton, &QPushButton::clicked, this, &OverdriveDialog::onConfirmYes);
    connect(m_noButton,  &QPushButton::clicked, this, &OverdriveDialog::onConfirmNo);
    connect(m_receiveCodeButton, &QPushButton::clicked, this, &OverdriveDialog::onReceiveCodeClicked);
    connect(m_applyButton, &QPushButton::clicked, this, &OverdriveDialog::onApplyClicked);

    m_countdownTimer = new QTimer(this);
    m_countdownTimer->setInterval(1000);
    connect(m_countdownTimer, &QTimer::timeout, this, &OverdriveDialog::onCountdownTick);
}

// ---------------------------------------------------------------------
// Mỗi hàm showStepXxx() chỉ làm 1 việc: ẨN HẾT mọi widget, rồi HIỆN lại
// đúng những widget cần cho bước đó. Đây là cách đơn giản để làm "wizard
// nhiều bước" trong 1 dialog mà không cần QStackedWidget phức tạp hơn.
// ---------------------------------------------------------------------
void OverdriveDialog::showStepConfirm()
{
    m_stepLabel->setText(
        "C:\\> Ban co chac chan muon lay ma overdrive/thoat khong?\n"
        "Hay suy nghi ky truoc khi tiep tuc.");

    m_yesButton->setVisible(true);
    m_noButton->setVisible(true);

    m_receiveCodeButton->setVisible(false);
    m_codeLabel->setVisible(false);
    m_overdriveRadio->setVisible(false);
    m_exitRadio->setVisible(false);
    m_overdriveMinutesBox->setVisible(false);
    m_retypeCodeEdit->setVisible(false);
    m_applyButton->setVisible(false);
}

void OverdriveDialog::showStepCountdown()
{
    m_yesButton->setVisible(false);
    m_noButton->setVisible(false);

    m_secondsLeft = 5;
    m_stepLabel->setText(QString("C:\\> Dang cho... (%1)").arg(m_secondsLeft));

    m_receiveCodeButton->setVisible(true);
    m_receiveCodeButton->setEnabled(false);

    m_countdownTimer->start();
}

void OverdriveDialog::showStepCodeAndAction()
{
    m_receiveCodeButton->setVisible(false);

    m_revealedCode = m_codeProvider.codeForDate(m_logicalDate);

    m_stepLabel->setText("C:\\> Ma cua hom nay la:");
    m_codeLabel->setText(m_revealedCode);
    m_codeLabel->setVisible(true);

    m_overdriveRadio->setVisible(true);
    m_exitRadio->setVisible(true);
    m_overdriveMinutesBox->setVisible(true);
    m_retypeCodeEdit->setVisible(true);
    m_applyButton->setVisible(true);
}

void OverdriveDialog::onConfirmYes()
{
    showStepCountdown();
}

void OverdriveDialog::onConfirmNo()
{
    m_resultAction = Action::None;
    reject(); // đóng dialog, kết quả QDialog::Rejected - không làm gì cả
}

void OverdriveDialog::onCountdownTick()
{
    m_secondsLeft -= 1;
    if (m_secondsLeft <= 0)
    {
        m_countdownTimer->stop();
        m_stepLabel->setText("C:\\> San sang.");
        m_receiveCodeButton->setEnabled(true);
        return;
    }
    m_stepLabel->setText(QString("C:\\> Dang cho... (%1)").arg(m_secondsLeft));
}

void OverdriveDialog::onReceiveCodeClicked()
{
    showStepCodeAndAction();
}

void OverdriveDialog::onApplyClicked()
{
    // So sánh mã user gõ lại với mã đã hiện - việc "gõ lại" này KHÔNG
    // kiểm tra thêm được gì về mặt bảo mật (user vừa đọc code 2 dòng
    // phía trên), tác dụng của nó thuần tuý là MA SÁT - buộc user phải
    // đọc kỹ + chủ động gõ lại, thay vì bấm 1 nút theo phản xạ.
    QString typed = m_retypeCodeEdit->text().trimmed().toUpper();
    if (typed != m_revealedCode)
    {
        QMessageBox::warning(this, "", "Ma go lai khong khop. Thu lai.");
        return;
    }

    m_resultAction = m_overdriveRadio->isChecked() ? Action::Overdrive : Action::Exit;
    accept();
}

int OverdriveDialog::overdriveMinutes() const
{
    return m_overdriveMinutesBox->value();
}
