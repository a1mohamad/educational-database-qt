/**
 * @file main.cpp
 * @brief Application entry point and runtime data-file preparation.
 */
#include "mainwindow.h"
#include "logindialog.h"
#include "firstadmindialog.h"
#include "authmanager.h"

#include <QApplication>
#include <QIcon>
#include <QCoreApplication>
#include <QDialog>
#include <QDir>
#include <QFile>
#include <QStringList>

/** Ensures the runtime database directory and required text files exist beside the executable. */
static void prepareDatabaseFiles()
{
    QDir().mkpath("database");

    const QStringList databaseFiles = {
        "students.txt",
        "teachers.txt",
        "courses.txt",
        "terms.txt",
        "term_courses.txt",
        "grades.txt",
        "national_core.txt",
        "national_details.txt"
    };

    for (const QString& fileName : databaseFiles) {
        const QString filePath = "database/" + fileName;

        if (!QFile::exists(filePath)) {
            QFile file(filePath);

            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                file.close();
            }
        }
    }
}

/** Starts the Qt application, prepares storage, handles login, and opens the main window. */
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/app_icon.ico"));

    QDir::setCurrent(QCoreApplication::applicationDirPath());

    prepareDatabaseFiles();
    AuthManager::ensureSystemFiles();

    int exitCode = 0;

    do {
        LoginDialog loginDialog;

        if (loginDialog.exec() != QDialog::Accepted) {
            return 0;
        }

        QString currentUsername = loginDialog.loggedInUsername();
        QString currentRole = loginDialog.loggedInRole();

        if (loginDialog.isTemporaryAdminLogin()) {
            FirstAdminDialog firstAdminDialog;

            if (firstAdminDialog.exec() != QDialog::Accepted) {
                return 0;
            }

            currentUsername = firstAdminDialog.createdUsername();
            currentRole = "admin";
        }

        MainWindow mainWindow;
        mainWindow.setCurrentUser(currentUsername, currentRole);
        mainWindow.show();

        exitCode = a.exec();

    } while (exitCode == MainWindow::LogoutExitCode);

    return exitCode;
}