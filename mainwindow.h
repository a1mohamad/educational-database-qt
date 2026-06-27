#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>

class QStackedWidget;
class QPushButton;
class QWidget;
class ReportDialog;
class QLabel;
class QLineEdit;
class QComboBox;
class QTableWidget;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

/**
 * @brief Main desktop window for the Educational Database application.
 *
 * MainWindow owns the primary Qt Widgets interface. It manages sidebar
 * navigation, database CRUD pages, report embedding, role-based system tools,
 * and logout flow. The class connects UI controls to file-based persistence
 * functions from the backend layer.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief Creates the main window, initializes UI state, and loads all tables.
     * @param parent Optional parent widget.
     */
    MainWindow(QWidget *parent = nullptr);

    /**
     * @brief Releases the generated Qt Designer UI object.
     */
    ~MainWindow();

    /**
     * @brief Special application exit code used to restart the login flow after logout.
     */
    static constexpr int LogoutExitCode = 100;

    /**
     * @brief Stores the authenticated user information for permission handling.
     * @param username Current logged-in username.
     * @param role Current logged-in role.
     */
    void setCurrentUser(const QString& username, const QString& role);

private:
    Ui::MainWindow *ui; ///< Qt Designer generated main-window UI.
    QStackedWidget *mainStackedWidget = nullptr; ///< Root stack for Database, Reports, and System pages.

    QWidget *databasePage = nullptr; ///< Database CRUD page.
    QWidget *reportsPage = nullptr;  ///< Reports page.
    QWidget *systemPage = nullptr;   ///< User-management and logout page.

    QPushButton *databaseNavButton = nullptr; ///< Sidebar button for the database page.
    QPushButton *reportsNavButton = nullptr;  ///< Sidebar button for the reports page.
    QPushButton *systemNavButton = nullptr;   ///< Sidebar button for the system page.
    ReportDialog *reportWidget = nullptr;     ///< Embedded report widget.

    QString currentUsername; ///< Username of the current session.
    QString currentRole;     ///< Role of the current session.

    QLabel *currentUserLabel = nullptr; ///< Displays current user and role in the System page.

    QLineEdit *systemUsernameLineEdit = nullptr; ///< Username input for user management.
    QLineEdit *systemPasswordLineEdit = nullptr; ///< Password input for user management.

    QComboBox *systemRoleComboBox = nullptr; ///< Role selector for user creation.

    QPushButton *systemAddUserButton = nullptr;            ///< Adds a new user.
    QPushButton *systemDeleteUserButton = nullptr;         ///< Deletes the selected user.
    QPushButton *systemChangePasswordButton = nullptr;     ///< Changes password for the selected user.
    QPushButton *systemRefreshUsersButton = nullptr;       ///< Reloads the user table.
    QPushButton *systemLogoutButton = nullptr;             ///< Logs out the current session.

    QTableWidget *systemUsersTable = nullptr;        ///< Displays saved application users.
    QLineEdit *systemUsersSearchLineEdit = nullptr;  ///< Filters the user table.

    /**
     * @brief Applies admin/normal role permissions to System page controls.
     */
    void applyRolePermissions();

    /** @brief Builds the System page widgets and signal connections. */
    void setupSystemPage();

    /** @brief Loads application users into the System page table. */
    void loadSystemUsers();

    /** @brief Creates a user from the System page form. */
    void addSystemUser();

    /** @brief Deletes the selected System page user. */
    void deleteSystemUser();

    /** @brief Changes the selected user's password. */
    void changeSystemUserPassword();

    /** @brief Ends the current session and restarts the login flow. */
    void logoutCurrentUser();

    /** @brief Builds the sidebar shell and embeds the main application pages. */
    void setupMainShell();

    /** @brief Shows the Database page in the main stack. */
    void showDatabasePage();

    /** @brief Shows the Reports page in the main stack. */
    void showReportsPage();

    /** @brief Shows the System page in the main stack. */
    void showSystemPage();

    /** @brief Configures the persons table headers and selection behavior. */
    void setupPersonsTable();

    /** @brief Configures the students table headers and selection behavior. */
    void setupStudentsTable();

    /** @brief Configures the teachers table headers and selection behavior. */
    void setupTeachersTable();

    /** @brief Configures the courses table headers and selection behavior. */
    void setupCoursesTable();

    /** @brief Configures the terms table headers and selection behavior. */
    void setupTermsTable();

    /** @brief Configures the term-courses table headers and selection behavior. */
    void setupTermCoursesTable();

    /** @brief Configures the grades table headers and selection behavior. */
    void setupGradesTable();

    /** @brief Loads person records into the persons table. */
    void loadPersons();

    /** @brief Adds a person record from the persons form. */
    void addPerson();

    /** @brief Updates the selected person record from the persons form. */
    void updatePerson();

    /** @brief Deletes the selected person record after dependency checks. */
    void deleteSelectedPerson();

    /** @brief Copies the selected person row into the persons form. */
    void fillPersonFormFromSelection();

    /** @brief Loads student records into the students table. */
    void loadStudents();

    /** @brief Adds a student record from the students form. */
    void addStudent();

    /** @brief Updates the selected student record from the students form. */
    void updateStudent();

    /** @brief Deletes the selected student record after dependency checks. */
    void deleteSelectedStudent();

    /** @brief Copies the selected student row into the students form. */
    void fillStudentFormFromSelection();

    /** @brief Loads teacher records into the teachers table. */
    void loadTeachers();

    /** @brief Adds a teacher record from the teachers form. */
    void addTeacher();

    /** @brief Updates the selected teacher record from the teachers form. */
    void updateTeacher();

    /** @brief Deletes the selected teacher record after dependency checks. */
    void deleteSelectedTeacher();

    /** @brief Copies the selected teacher row into the teachers form. */
    void fillTeacherFormFromSelection();

    /** @brief Loads course records into the courses table. */
    void loadCourses();

    /** @brief Adds a course record from the courses form. */
    void addCourse();

    /** @brief Updates the selected course record from the courses form. */
    void updateCourse();

    /** @brief Deletes the selected course record after dependency checks. */
    void deleteSelectedCourse();

    /** @brief Copies the selected course row into the courses form. */
    void fillCourseFormFromSelection();

    /** @brief Loads academic term records into the terms table. */
    void loadTerms();

    /** @brief Adds an academic term record from the terms form. */
    void addTerm();

    /** @brief Updates the selected academic term record from the terms form. */
    void updateTerm();

    /** @brief Deletes the selected academic term record after dependency checks. */
    void deleteSelectedTerm();

    /** @brief Copies the selected term row into the terms form. */
    void fillTermFormFromSelection();

    /** @brief Loads term-course records into the term-courses table. */
    void loadTermCourses();

    /** @brief Adds a term-course assignment from the term-courses form. */
    void addTermCourse();

    /** @brief Updates the selected term-course assignment from the form. */
    void updateTermCourse();

    /** @brief Deletes the selected term-course assignment after dependency checks. */
    void deleteSelectedTermCourse();

    /** @brief Copies the selected term-course row into the term-courses form. */
    void fillTermCourseFormFromSelection();

    /** @brief Loads grade records into the grades table. */
    void loadGrades();

    /** @brief Adds a grade record and recalculates student statistics. */
    void addGrade();

    /** @brief Updates the selected grade record and recalculates student statistics. */
    void updateGrade();

    /** @brief Deletes the selected grade record and recalculates student statistics. */
    void deleteSelectedGrade();

    /** @brief Copies the selected grade row into the grades form. */
    void fillGradeFormFromSelection();
};

#endif
