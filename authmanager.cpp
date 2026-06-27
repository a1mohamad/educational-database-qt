/**
 * @file authmanager.cpp
 * @brief Implements file-based authentication, password hashing, and user management.
 */
#include "authmanager.h"

#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QTextStream>

/** Returns the relative users.dat path used by the authentication layer. */
QString AuthManager::usersFilePath()
{
    return "system_files/users.dat";
}

/** Creates the system_files directory and users.dat file when they do not exist. */
void AuthManager::ensureSystemFiles()
{
    QDir().mkpath("system_files");

    QFile usersFile(usersFilePath());

    if (!usersFile.exists()) {
        usersFile.open(QIODevice::WriteOnly | QIODevice::Text);
        usersFile.close();
    }
}

/** Detects whether the application still needs its first permanent administrator account. */
bool AuthManager::isFirstRun()
{
    ensureSystemFiles();

    return readUsers().isEmpty();
}

/** Loads and validates user records from the pipe-separated users.dat file. */
QVector<AuthUser> AuthManager::readUsers()
{
    ensureSystemFiles();

    QVector<AuthUser> users;

    QFile file(usersFilePath());

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return users;
    }

    QTextStream in(&file);

    while (!in.atEnd()) {
        const QString line = in.readLine().trimmed();

        if (line.isEmpty()) {
            continue;
        }

        const QStringList parts = line.split("|");

        if (parts.size() != 3) {
            continue;
        }

        AuthUser user;
        user.username = parts[0].trimmed();
        user.passwordHash = parts[1].trimmed();
        user.role = parts[2].trimmed();

        if (!user.username.isEmpty() && !user.passwordHash.isEmpty() && !user.role.isEmpty()) {
            users.push_back(user);
        }
    }

    return users;
}

/** Rewrites the full user file after create, delete, or password-change operations. */
bool AuthManager::writeUsers(const QVector<AuthUser>& users)
{
    ensureSystemFiles();

    QFile file(usersFilePath());

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        return false;
    }

    QTextStream out(&file);

    for (const AuthUser& user : users) {
        out << user.username << "|"
            << user.passwordHash << "|"
            << user.role << "\n";
    }

    return true;
}

/** Hashes a normalized username and password pair using SHA-256. */
QString AuthManager::hashPassword(
    const QString& username,
    const QString& password
    )
{
    const QString normalizedUsername = username.trimmed().toLower();
    const QByteArray input = (normalizedUsername + "|" + password).toUtf8();

    return QString::fromLatin1(
        QCryptographicHash::hash(input, QCryptographicHash::Sha256).toHex()
        );
}

/** Validates credentials, including the temporary first-run admin bootstrap account. */
bool AuthManager::validateLogin(
    const QString& username,
    const QString& password,
    AuthUser* loggedInUser
    )
{
    ensureSystemFiles();

    const QString cleanUsername = username.trimmed();

    // Temporary first entrance only.
    // This user is never saved into users.dat.
    if (isFirstRun()) {
        if (cleanUsername == "admin" && password == "admin123") {
            if (loggedInUser != nullptr) {
                loggedInUser->username = "admin";
                loggedInUser->passwordHash = "";
                loggedInUser->role = "admin";
            }

            return true;
        }

        return false;
    }

    const QVector<AuthUser> users = readUsers();

    for (const AuthUser& user : users) {
        if (user.username.compare(cleanUsername, Qt::CaseInsensitive) == 0 &&
            user.passwordHash == hashPassword(user.username, password)) {

            if (loggedInUser != nullptr) {
                *loggedInUser = user;
            }

            return true;
        }
    }

    return false;
}

/** Creates the first real administrator when no saved users exist. */
bool AuthManager::createFirstAdmin(
    const QString& username,
    const QString& password,
    QString* message
    )
{
    ensureSystemFiles();

    if (!isFirstRun()) {
        if (message != nullptr) {
            *message = "First admin already exists.";
        }

        return false;
    }

    if (!validateUsername(username, message)) {
        return false;
    }

    if (!validatePassword(password, message)) {
        return false;
    }

    AuthUser admin;
    admin.username = username.trimmed();
    admin.passwordHash = hashPassword(admin.username, password);
    admin.role = "admin";

    QVector<AuthUser> users;
    users.push_back(admin);

    if (!writeUsers(users)) {
        if (message != nullptr) {
            *message = "Could not save first admin user.";
        }

        return false;
    }

    if (message != nullptr) {
        *message = "First admin account created.";
    }

    return true;
}

/** Adds a validated user account and persists the updated user list. */
bool AuthManager::addUser(
    const QString& username,
    const QString& password,
    const QString& role,
    QString* message
    )
{
    ensureSystemFiles();

    if (!validateUsername(username, message)) {
        return false;
    }

    if (!validatePassword(password, message)) {
        return false;
    }

    const QString cleanRole = role.trimmed().toLower();

    if (cleanRole != "admin" && cleanRole != "user") {
        if (message != nullptr) {
            *message = "Role must be admin or user.";
        }

        return false;
    }

    QVector<AuthUser> users = readUsers();

    if (usernameExists(users, username)) {
        if (message != nullptr) {
            *message = "Username already exists.";
        }

        return false;
    }

    AuthUser user;
    user.username = username.trimmed();
    user.passwordHash = hashPassword(user.username, password);
    user.role = cleanRole;

    users.push_back(user);

    if (!writeUsers(users)) {
        if (message != nullptr) {
            *message = "Could not save user.";
        }

        return false;
    }

    if (message != nullptr) {
        *message = "User added successfully.";
    }

    return true;
}

/** Deletes a user while protecting the final administrator account. */
bool AuthManager::deleteUser(
    const QString& username,
    QString* message
    )
{
    ensureSystemFiles();

    QVector<AuthUser> users = readUsers();
    const QString cleanUsername = username.trimmed();

    int targetIndex = -1;

    for (int i = 0; i < users.size(); ++i) {
        if (users[i].username.compare(cleanUsername, Qt::CaseInsensitive) == 0) {
            targetIndex = i;
            break;
        }
    }

    if (targetIndex < 0) {
        if (message != nullptr) {
            *message = "User not found.";
        }

        return false;
    }

    if (users[targetIndex].role == "admin" && adminCount(users) <= 1) {
        if (message != nullptr) {
            *message = "You cannot delete the last admin user.";
        }

        return false;
    }

    users.removeAt(targetIndex);

    if (!writeUsers(users)) {
        if (message != nullptr) {
            *message = "Could not delete user.";
        }

        return false;
    }

    if (message != nullptr) {
        *message = "User deleted successfully.";
    }

    return true;
}

/** Replaces the selected user password with a newly hashed password value. */
bool AuthManager::changePassword(
    const QString& username,
    const QString& newPassword,
    QString* message
    )
{
    ensureSystemFiles();

    if (!validatePassword(newPassword, message)) {
        return false;
    }

    QVector<AuthUser> users = readUsers();
    const QString cleanUsername = username.trimmed();

    for (AuthUser& user : users) {
        if (user.username.compare(cleanUsername, Qt::CaseInsensitive) == 0) {
            user.passwordHash = hashPassword(user.username, newPassword);

            if (!writeUsers(users)) {
                if (message != nullptr) {
                    *message = "Could not change password.";
                }

                return false;
            }

            if (message != nullptr) {
                *message = "Password changed successfully.";
            }

            return true;
        }
    }

    if (message != nullptr) {
        *message = "User not found.";
    }

    return false;
}

/** Checks username constraints shared by user creation and account updates. */
bool AuthManager::validateUsername(
    const QString& username,
    QString* message
    )
{
    const QString cleanUsername = username.trimmed();

    if (cleanUsername.isEmpty()) {
        if (message != nullptr) {
            *message = "Username cannot be empty.";
        }

        return false;
    }

    if (cleanUsername.length() < 3) {
        if (message != nullptr) {
            *message = "Username must be at least 3 characters.";
        }

        return false;
    }

    if (cleanUsername.contains("|") || cleanUsername.contains(" ")) {
        if (message != nullptr) {
            *message = "Username cannot contain spaces or | character.";
        }

        return false;
    }

    return true;
}

/** Checks password constraints shared by user creation and password changes. */
bool AuthManager::validatePassword(
    const QString& password,
    QString* message
    )
{
    if (password.length() < 6) {
        if (message != nullptr) {
            *message = "Password must be at least 6 characters.";
        }

        return false;
    }

    if (password.contains("|")) {
        if (message != nullptr) {
            *message = "Password cannot contain | character.";
        }

        return false;
    }

    return true;
}

/** Performs a case-insensitive duplicate username check. */
bool AuthManager::usernameExists(
    const QVector<AuthUser>& users,
    const QString& username
    )
{
    for (const AuthUser& user : users) {
        if (user.username.compare(username.trimmed(), Qt::CaseInsensitive) == 0) {
            return true;
        }
    }

    return false;
}

/** Counts saved accounts with administrator permissions. */
int AuthManager::adminCount(
    const QVector<AuthUser>& users
    )
{
    int count = 0;

    for (const AuthUser& user : users) {
        if (user.role == "admin") {
            ++count;
        }
    }

    return count;
}