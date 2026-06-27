#ifndef FIRSTADMINDIALOG_H
#define FIRSTADMINDIALOG_H

#include <QDialog>
#include <QString>

namespace Ui {
class FirstAdminDialog;
}

/**
 * @brief Dialog used to create the first permanent administrator account.
 *
 * This dialog appears after the temporary first-run admin login succeeds. It
 * forces the user to create a real administrator before entering the main
 * application window.
 */
class FirstAdminDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Creates the first-admin dialog and initializes its signal handlers.
     * @param parent Optional parent widget.
     */
    explicit FirstAdminDialog(QWidget *parent = nullptr);

    /**
     * @brief Releases the generated Qt Designer UI object.
     */
    ~FirstAdminDialog();

    /**
     * @brief Returns the username created by this dialog.
     * @return The accepted administrator username.
     */
    QString createdUsername() const;

private:
    Ui::FirstAdminDialog *ui; ///< Qt Designer generated dialog UI.

    QString m_createdUsername; ///< Stores the administrator username after successful creation.

    /**
     * @brief Attempts to validate and create the first administrator account.
     */
    void createAdmin();

    /**
     * @brief Shows a success or error message inside the dialog.
     * @param message Text to display.
     * @param error true for error styling; false for success styling.
     */
    void showMessage(const QString& message, bool error = true);
};

#endif
