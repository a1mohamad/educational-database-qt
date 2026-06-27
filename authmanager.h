#ifndef AUTHMANAGER_H
#define AUTHMANAGER_H

#include <QString>
#include <QVector>

/**
 * @brief Represents one application user loaded from the authentication file.
 *
 * The application stores user records in a simple pipe-separated file format:
 * username | passwordHash | role. The passwordHash field stores the SHA-256
 * hash produced by AuthManager::hashPassword(), not the plain password.
 */
struct AuthUser
{
    QString username;      ///< Unique login name used by the user.
    QString passwordHash;  ///< Stored SHA-256 password hash.
    QString role;          ///< User permission level, usually "admin" or "normal".
};

/**
 * @brief Provides file-based authentication and user-management operations.
 *
 * AuthManager centralizes all logic related to login validation, first-run
 * administrator creation, password hashing, and user-file persistence. All
 * functions are static because authentication is handled as an application-wide
 * service rather than as per-window state.
 */
class AuthManager
{
public:
    /**
     * @brief Returns the relative path of the user storage file.
     * @return The users.dat path inside the system_files directory.
     */
    static QString usersFilePath();

    /**
     * @brief Ensures that the authentication directory and user file exist.
     */
    static void ensureSystemFiles();

    /**
     * @brief Checks whether the application has no saved users yet.
     * @return true when users.dat exists but contains no valid users; otherwise false.
     */
    static bool isFirstRun();

    /**
     * @brief Reads all valid user records from users.dat.
     * @return A vector containing all parsed users.
     */
    static QVector<AuthUser> readUsers();

    /**
     * @brief Rewrites users.dat using the supplied user collection.
     * @param users Users that should be persisted.
     * @return true when the file is written successfully; otherwise false.
     */
    static bool writeUsers(const QVector<AuthUser>& users);

    /**
     * @brief Validates a username and password against the saved users.
     *
     * On a first run, a temporary admin/admin123 login is accepted only to allow
     * creation of the first real administrator account.
     *
     * @param username Username typed in the login dialog.
     * @param password Plain password typed in the login dialog.
     * @param loggedInUser Optional output parameter filled with the matched user.
     * @return true when the credentials are valid; otherwise false.
     */
    static bool validateLogin(
        const QString& username,
        const QString& password,
        AuthUser* loggedInUser = nullptr
        );

    /**
     * @brief Creates the first permanent administrator account.
     * @param username Desired administrator username.
     * @param password Desired administrator password.
     * @param message Optional output message describing success or validation errors.
     * @return true when the first administrator is created; otherwise false.
     */
    static bool createFirstAdmin(
        const QString& username,
        const QString& password,
        QString* message = nullptr
        );

    /**
     * @brief Adds a new user account.
     * @param username New account username.
     * @param password New account password.
     * @param role New account role.
     * @param message Optional output message describing success or validation errors.
     * @return true when the account is added; otherwise false.
     */
    static bool addUser(
        const QString& username,
        const QString& password,
        const QString& role,
        QString* message = nullptr
        );

    /**
     * @brief Deletes an existing user account by username.
     *
     * The implementation prevents removing the final administrator account so
     * that the application always remains manageable.
     *
     * @param username Username to delete.
     * @param message Optional output message describing success or failure.
     * @return true when the user is removed; otherwise false.
     */
    static bool deleteUser(
        const QString& username,
        QString* message = nullptr
        );

    /**
     * @brief Changes an existing user's password.
     * @param username Target username.
     * @param newPassword New plain password.
     * @param message Optional output message describing success or validation errors.
     * @return true when the password is changed; otherwise false.
     */
    static bool changePassword(
        const QString& username,
        const QString& newPassword,
        QString* message = nullptr
        );

    /**
     * @brief Creates a deterministic SHA-256 hash for a username/password pair.
     * @param username Username used as part of the hash input.
     * @param password Plain password used as part of the hash input.
     * @return Hex-encoded SHA-256 hash string.
     */
    static QString hashPassword(
        const QString& username,
        const QString& password
        );

private:
    /**
     * @brief Validates username rules before creating or updating an account.
     */
    static bool validateUsername(
        const QString& username,
        QString* message
        );

    /**
     * @brief Validates password rules before creating or updating an account.
     */
    static bool validatePassword(
        const QString& password,
        QString* message
        );

    /**
     * @brief Checks whether a username already exists in a loaded user list.
     */
    static bool usernameExists(
        const QVector<AuthUser>& users,
        const QString& username
        );

    /**
     * @brief Counts administrator accounts in a loaded user list.
     */
    static int adminCount(
        const QVector<AuthUser>& users
        );
};

#endif
