#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QString>

namespace Ui {
class LoginDialog;
}

/**
 * @brief Login dialog displayed before the main application window.
 *
 * The dialog validates user credentials through AuthManager. During first run,
 * it also supports a temporary administrator login that redirects the user to
 * FirstAdminDialog for creation of the first permanent admin account.
 */
class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Creates the login dialog and connects login controls.
     * @param parent Optional parent widget.
     */
    explicit LoginDialog(QWidget *parent = nullptr);

    /**
     * @brief Releases the generated Qt Designer UI object.
     */
    ~LoginDialog();

    /**
     * @brief Returns the username authenticated by the dialog.
     */
    QString loggedInUsername() const;

    /**
     * @brief Returns the role of the authenticated user.
     */
    QString loggedInRole() const;

    /**
     * @brief Indicates whether the accepted login was the temporary first-run admin.
     */
    bool isTemporaryAdminLogin() const;

private:
    Ui::LoginDialog *ui; ///< Qt Designer generated dialog UI.

    QString m_loggedInUsername; ///< Username accepted by the login flow.
    QString m_loggedInRole;     ///< Role accepted by the login flow.
    bool m_temporaryAdminLogin = false; ///< true only for the bootstrap admin login.

    /**
     * @brief Reads user input and validates credentials.
     */
    void handleLogin();

    /**
     * @brief Shows a success or error message inside the dialog.
     * @param message Text to display.
     * @param error true for error styling; false for success styling.
     */
    void showMessage(const QString& message, bool error = true);
};

#endif
