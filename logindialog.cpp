/**
 * @file logindialog.cpp
 * @brief Implements the login dialog and first-run temporary admin flow.
 */
#include "logindialog.h"
#include "ui_logindialog.h"

#include "authmanager.h"

#include <QIcon>
#include <QLineEdit>
#include <QPushButton>

/** Initializes the login dialog and configures first-run helper text. */
LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    setWindowIcon(QIcon(":/app_icon.ico"));

    setWindowTitle("Login");

    ui->passwordLineEdit->setEchoMode(QLineEdit::Password);
    ui->messageLabel->clear();

    if (AuthManager::isFirstRun()) {
        ui->instructionLabel->setText("First entrance: login with the temporary admin account.");
        showMessage("Username: admin    Password: admin123", false);
    } else {
        ui->instructionLabel->setText("Please enter your username and password.");
    }

    connect(ui->loginButton, &QPushButton::clicked, this, &LoginDialog::handleLogin);
    connect(ui->cancelButton, &QPushButton::clicked, this, &LoginDialog::reject);

    connect(ui->passwordLineEdit, &QLineEdit::returnPressed, this, &LoginDialog::handleLogin);
    connect(ui->usernameLineEdit, &QLineEdit::returnPressed, this, &LoginDialog::handleLogin);
}

/** Releases the generated dialog UI. */
LoginDialog::~LoginDialog()
{
    delete ui;
}

/** Returns the username accepted by the login process. */
QString LoginDialog::loggedInUsername() const
{
    return m_loggedInUsername;
}

/** Returns the role accepted by the login process. */
QString LoginDialog::loggedInRole() const
{
    return m_loggedInRole;
}

/** Reports whether the accepted login used the temporary first-run admin account. */
bool LoginDialog::isTemporaryAdminLogin() const
{
    return m_temporaryAdminLogin;
}

/** Reads credentials, validates them through AuthManager, and accepts the dialog on success. */
void LoginDialog::handleLogin()
{
    const QString username = ui->usernameLineEdit->text().trimmed();
    const QString password = ui->passwordLineEdit->text();

    AuthUser user;

    if (!AuthManager::validateLogin(username, password, &user)) {
        showMessage("Invalid username or password.", true);
        ui->passwordLineEdit->clear();
        ui->passwordLineEdit->setFocus();
        return;
    }

    m_loggedInUsername = user.username;
    m_loggedInRole = user.role;
    m_temporaryAdminLogin = AuthManager::isFirstRun();

    accept();
}

/** Displays a colored success or error message in the dialog. */
void LoginDialog::showMessage(const QString& message, bool error)
{
    ui->messageLabel->setText(message);

    if (error) {
        ui->messageLabel->setStyleSheet("color: #dc2626;");
    } else {
        ui->messageLabel->setStyleSheet("color: #2563eb;");
    }
}