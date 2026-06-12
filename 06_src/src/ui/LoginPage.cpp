#include "LoginPage.h"
#include "service/AuthService.h"
#include <QMessageBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDebug>

LoginPage::LoginPage(QSqlDatabase &db, QWidget *parent)
    : QWidget(parent), m_db(db)
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setAlignment(Qt::AlignCenter);

    auto *titleLabel = new QLabel(QStringLiteral("硬件产品全生命周期管理系统 (HPLM)"));
    QFont titleFont;
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);
    mainLayout->addSpacing(30);

    auto *formLayout = new QFormLayout();
    formLayout->setLabelAlignment(Qt::AlignRight);

    m_employeeNoEdit = new QLineEdit();
    m_employeeNoEdit->setPlaceholderText(QStringLiteral("请输入工号（如 EMP001）"));
    m_employeeNoEdit->setMinimumWidth(280);
    formLayout->addRow(QStringLiteral("工号："), m_employeeNoEdit);

    m_passwordEdit = new QLineEdit();
    m_passwordEdit->setPlaceholderText(QStringLiteral("请输入密码"));
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setMinimumWidth(280);
    formLayout->addRow(QStringLiteral("密码："), m_passwordEdit);

    mainLayout->addLayout(formLayout);
    mainLayout->addSpacing(20);

    auto *btnLayout = new QHBoxLayout();
    btnLayout->setAlignment(Qt::AlignCenter);

    auto *loginBtn = new QPushButton(QStringLiteral("登录"));
    loginBtn->setDefault(true);
    loginBtn->setMinimumWidth(100);
    btnLayout->addWidget(loginBtn);

    auto *registerBtn = new QPushButton(QStringLiteral("注册"));
    registerBtn->setMinimumWidth(100);
    btnLayout->addWidget(registerBtn);

    mainLayout->addLayout(btnLayout);
    mainLayout->addSpacing(10);

    m_statusLabel = new QLabel();
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setStyleSheet("color: red;");
    mainLayout->addWidget(m_statusLabel);

    connect(loginBtn, &QPushButton::clicked, this, &LoginPage::onLoginClicked);
    connect(registerBtn, &QPushButton::clicked, this, &LoginPage::onRegisterClicked);
    connect(m_passwordEdit, &QLineEdit::returnPressed, this, &LoginPage::onLoginClicked);

    setMinimumSize(400, 350);
}

void LoginPage::onLoginClicked()
{
    QString no = m_employeeNoEdit->text().trimmed();
    QString pwd = m_passwordEdit->text();

    if (no.isEmpty() || pwd.isEmpty()) {
        m_statusLabel->setText(QStringLiteral("工号和密码不能为空"));
        return;
    }

    AuthService auth(m_db);
    Employee emp = auth.login(no, pwd);

    if (emp.employeeId == 0) {
        m_statusLabel->setText(QStringLiteral("工号或密码错误，请重试"));
        m_passwordEdit->clear();
        m_passwordEdit->setFocus();
        return;
    }

    m_statusLabel->setStyleSheet("color: green;");
    m_statusLabel->setText(QStringLiteral("登录成功！欢迎 %1").arg(emp.name));

    emit loginSucceeded(emp);
}

void LoginPage::onRegisterClicked()
{
    QDialog dlg(this);
    dlg.setWindowTitle(QStringLiteral("注册新员工"));
    auto *layout = new QFormLayout(&dlg);

    auto *noEdit   = new QLineEdit();
    auto *nameEdit = new QLineEdit();
    auto *pwdEdit  = new QLineEdit();
    pwdEdit->setEchoMode(QLineEdit::Password);
    auto *posEdit  = new QLineEdit();
    auto *phoneEdit = new QLineEdit();
    auto *emailEdit = new QLineEdit();

    layout->addRow(QStringLiteral("工号："), noEdit);
    layout->addRow(QStringLiteral("姓名："), nameEdit);
    layout->addRow(QStringLiteral("密码："), pwdEdit);
    layout->addRow(QStringLiteral("职位："), posEdit);
    layout->addRow(QStringLiteral("电话："), phoneEdit);
    layout->addRow(QStringLiteral("邮箱："), emailEdit);

    auto *bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(bb, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    layout->addRow(bb);

    if (dlg.exec() != QDialog::Accepted) return;

    Employee e;
    e.employeeNo = noEdit->text().trimmed();
    e.name       = nameEdit->text().trimmed();
    e.password   = pwdEdit->text();
    e.position   = posEdit->text().trimmed();
    e.phone      = phoneEdit->text().trimmed();
    e.email      = emailEdit->text().trimmed();

    AuthService auth(m_db);
    if (auth.registerEmployee(e)) {
        QMessageBox::information(this, QStringLiteral("注册成功"), QStringLiteral("注册成功，请使用工号和密码登录"));
        m_employeeNoEdit->setText(e.employeeNo);
        m_passwordEdit->setFocus();
    } else {
        QMessageBox::warning(this, QStringLiteral("注册失败"), QStringLiteral("注册失败，工号可能已存在"));
    }
}
