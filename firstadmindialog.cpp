/**
 * @file firstadmindialog.cpp
 * @brief Implements the first-run administrator creation dialog.
 */
#include "firstadmindialog.h"
#include "ui_firstadmindialog.h"

#include "authmanager.h"
#include <QIcon>
#include <QLineEdit>
#include <QPushButton>

/** Initializes the first-admin dialog and connects the create button. */
FirstAdminDialog::FirstAdminDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FirstAdminDialog)
{
    ui->setupUi(this);
    setWindowIcon(QIcon(":/app_icon.ico"));

    setWindowTitle("First Admin Setup");

    ui->instructionLabel->setText("Create your first real admin account.");
    ui->messageLabel->clear();

    ui->passwordLineEdit->setEchoMode(QLineEdit::Password);
    ui->confirmPasswordLineEdit->setEchoMode(QLineEdit::Password);

    connect(ui->createAdminButton, &QPushButton::clicked, this, &FirstAdminDialog::createAdmin);
    connect(ui->cancelButton, &QPushButton::clicked, this, &FirstAdminDialog::reject);

    connect(ui->confirmPasswordLineEdit, &QLineEdit::returnPressed, this, &FirstAdminDialog::createAdmin);
}

/** Releases the generated dialog UI. */
FirstAdminDialog::~FirstAdminDialog()
{
    delete ui;
}

/** Returns the administrator username created during this dialog session. */
QString FirstAdminDialog::createdUsername() const
{
    return m_createdUsername;
}

/** Validates form input and delegates first-admin creation to AuthManager. */
void FirstAdminDialog::createAdmin()
{
    const QString username = ui->usernameLineEdit->text().trimmed();
    const QString password = ui->passwordLineEdit->text();
    const QString confirmPassword = ui->confirmPasswordLineEdit->text();

    if (password != confirmPassword) {
        showMessage("Passwords do not match.", true);
        ui->confirmPasswordLineEdit->clear();
        ui->confirmPasswordLineEdit->setFocus();
        return;
    }

    QString message;

    if (!AuthManager::createFirstAdmin(username, password, &message)) {
        showMessage(message, true);
        return;
    }

    m_createdUsername = username;

    accept();
}

/** Displays a colored success or error message in the dialog. */
void FirstAdminDialog::showMessage(const QString& message, bool error)
{
    ui->messageLabel->setText(message);

    if (error) {
        ui->messageLabel->setStyleSheet("color: #dc2626;");
    } else {
        ui->messageLabel->setStyleSheet("color: #16a34a;");
    }
}