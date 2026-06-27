/**
 * @file mainwindow.cpp
 * @brief Implements the main Qt Widgets shell, CRUD workflows, reports navigation, and user-management page.
 */
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "backend/fileIO.h"
#include "backend/structures.h"
#include "reportdialog.h"
#include "authmanager.h"

#include <QMenu>
#include <QMessageBox>
#include <QPoint>
#include <QIcon>

#include <QAbstractItemView>
#include <QCalendar>
#include <QComboBox>
#include <QCompleter>
#include <QDate>
#include <QDoubleSpinBox>
#include <QHeaderView>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QAbstractItemView>
#include <QComboBox>
#include <QCoreApplication>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QLineEdit>
#include <QMessageBox>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QButtonGroup>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QStackedWidget>
#include <QVBoxLayout>

#include <algorithm>
#include <string>
#include <vector>

/** Creates a table item with centered text for consistent table formatting. */
static QTableWidgetItem* makeCenteredItem(const QString& text)
{
    QTableWidgetItem* item = new QTableWidgetItem(text);
    item->setTextAlignment(Qt::AlignCenter);
    return item;
}

/** Detects the pipe delimiter because text files use | as the field separator. */
static bool containsPipe(const QString& text)
{
    return text.contains('|');
}

/** Assigns a style role property used by the Qt stylesheet. */
static void setButtonRole(QPushButton* button, const QString& role)
{
    if (button != nullptr) {
        button->setProperty("role", role);
    }
}

/** Makes a combo box searchable while preventing custom values from being inserted. */
static void makeComboBoxSearchable(QComboBox* comboBox, const QString& placeholder)
{
    comboBox->setEditable(true);
    comboBox->setInsertPolicy(QComboBox::NoInsert);

    if (comboBox->lineEdit() != nullptr) {
        comboBox->lineEdit()->setPlaceholderText(placeholder);
        comboBox->lineEdit()->clear();
    }

    QCompleter* completer = comboBox->completer();

    if (completer != nullptr) {
        completer->setCompletionMode(QCompleter::PopupCompletion);
        completer->setFilterMode(Qt::MatchContains);
        completer->setCaseSensitivity(Qt::CaseInsensitive);
    }

    comboBox->setCurrentIndex(-1);
}

/** Clears a searchable combo box back to its placeholder state. */
static void resetComboBoxPlaceholder(QComboBox* comboBox)
{
    comboBox->setCurrentIndex(-1);

    if (comboBox->lineEdit() != nullptr) {
        comboBox->lineEdit()->clear();
    }
}

/** Selects a combo-box item by stored data instead of visible text. */
static bool setComboBoxByData(QComboBox* comboBox, const QString& value)
{
    for (int index = 0; index < comboBox->count(); ++index) {
        if (comboBox->itemData(index).toString() == value) {
            comboBox->setCurrentIndex(index);
            return true;
        }
    }

    comboBox->setCurrentIndex(-1);

    if (comboBox->lineEdit() != nullptr) {
        comboBox->lineEdit()->clear();
    }

    return false;
}

/** Filters table rows by searching all visible columns case-insensitively. */
static void filterTable(QTableWidget* table, const QString& query)
{
    const QString normalizedQuery = query.trimmed();

    for (int row = 0; row < table->rowCount(); ++row) {
        bool matched = normalizedQuery.isEmpty();

        for (int column = 0; column < table->columnCount() && !matched; ++column) {
            QTableWidgetItem* item = table->item(row, column);

            if (item != nullptr && item->text().contains(normalizedQuery, Qt::CaseInsensitive)) {
                matched = true;
            }
        }

        table->setRowHidden(row, !matched);
    }
}

/** Returns an item from the selected table row or nullptr when nothing is selected. */
static QTableWidgetItem* currentRowItem(QTableWidget* table, int column)
{
    const int row = table->currentRow();

    if (row < 0) {
        return nullptr;
    }

    return table->item(row, column);
}

/** Parses a Jalali date string and falls back to today when input is invalid. */
static QDate parseJalaliDateOrToday(const QString& text)
{
    const QDate parsedDate = QDate::fromString(
        text,
        "yyyy/MM/dd",
        QCalendar(QCalendar::System::Jalali)
        );

    if (parsedDate.isValid()) {
        return parsedDate;
    }

    return QDate::currentDate();
}

/** Validates that the combo-box text matches one of the existing selectable items. */
static bool comboBoxHasValidSelection(QComboBox* comboBox)
{
    const int index = comboBox->currentIndex();

    if (index < 0) {
        return false;
    }

    return comboBox->currentText() == comboBox->itemText(index);
}

/** Finds the credit count for a course identifier. */
static int findCourseCreditsByCourseId(
    const std::string& courseId,
    const std::vector<Course>& courses
    )
{
    for (const Course& course : courses) {
        if (course.courseId == courseId) {
            return course.credits;
        }
    }

    return 0;
}

/** Resolves a term-course identifier to the underlying course credit count. */
static int findCourseCreditsByTermCourseId(
    const std::string& termCourseId,
    const std::vector<TermCourse>& termCourses,
    const std::vector<Course>& courses
    )
{
    for (const TermCourse& termCourse : termCourses) {
        if (termCourse.termCourseId == termCourseId) {
            return findCourseCreditsByCourseId(termCourse.courseId, courses);
        }
    }

    return 0;
}

/** Recomputes grade pass values, total passed credits, and weighted student averages. */
static void recalculateStudentStatistics()
{
    std::vector<Student> students = readStudents("database/students.txt");
    std::vector<Grade> grades = readGrades("database/grades.txt");
    std::vector<TermCourse> termCourses = readTermCourses("database/term_courses.txt");
    std::vector<Course> courses = readCourses("database/courses.txt");

    for (Grade& grade : grades) {
        const int credits = findCourseCreditsByTermCourseId(
            grade.termCourseId,
            termCourses,
            courses
            );

        if (grade.gradeValue >= 10.0) {
            grade.passed = credits;
        } else {
            grade.passed = 0;
        }
    }

    for (Student& student : students) {
        int passedCredits = 0;
        int totalCreditsForAverage = 0;
        double weightedSum = 0.0;

        for (const Grade& grade : grades) {
            if (grade.studentId != student.studentId) {
                continue;
            }

            int credits = findCourseCreditsByTermCourseId(
                grade.termCourseId,
                termCourses,
                courses
                );

            if (credits <= 0) {
                credits = 1;
            }

            passedCredits += grade.passed;
            weightedSum += grade.gradeValue * credits;
            totalCreditsForAverage += credits;
        }

        student.passed = passedCredits;

        if (totalCreditsForAverage > 0) {
            student.grade = weightedSum / totalCreditsForAverage;
        } else {
            student.grade = 0.0;
        }
    }

    writeGrades("database/grades.txt", grades);
    writeStudents("database/students.txt", students);
}

/** Resolves a national code to the display name stored in the people list. */
static QString getPersonNameByMelli(
    const std::string& melli,
    const std::vector<NationalCore>& people
    )
{
    for (const NationalCore& person : people) {
        if (person.melli == melli) {
            return QString::fromStdString(person.firstName + " " + person.family);
        }
    }

    return "Unknown";
}

/** Builds the main UI, connects all CRUD actions, configures tables, and loads initial data. */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowIcon(QIcon(":/app_icon.ico"));
    setButtonRole(ui->addPersonButton, "primary");
    setButtonRole(ui->addStudentButton, "primary");
    setButtonRole(ui->addTeacherButton, "primary");
    setButtonRole(ui->addCourseButton, "primary");
    setButtonRole(ui->addTermButton, "primary");
    setButtonRole(ui->addTermCourseButton, "primary");
    setButtonRole(ui->addGradeButton, "primary");

    setButtonRole(ui->updatePersonButton, "success");
    setButtonRole(ui->updateStudentButton, "success");
    setButtonRole(ui->updateTeacherButton, "success");
    setButtonRole(ui->updateCourseButton, "success");
    setButtonRole(ui->updateTermButton, "success");
    setButtonRole(ui->updateTermCourseButton, "success");
    setButtonRole(ui->updateGradeButton, "success");

    setButtonRole(ui->deletePersonButton, "danger");
    setButtonRole(ui->deleteStudentButton, "danger");
    setButtonRole(ui->deleteTeacherButton, "danger");
    setButtonRole(ui->deleteCourseButton, "danger");
    setButtonRole(ui->deleteTermButton, "danger");
    setButtonRole(ui->deleteTermCourseButton, "danger");
    setButtonRole(ui->deleteGradeButton, "danger");

    setButtonRole(ui->refreshPersonsButton, "secondary");
    setButtonRole(ui->refreshStudentsButton, "secondary");
    setButtonRole(ui->refreshTeachersButton, "secondary");
    setButtonRole(ui->refreshCoursesButton, "secondary");
    setButtonRole(ui->refreshTermsButton, "secondary");
    setButtonRole(ui->refreshTermCoursesButton, "secondary");
    setButtonRole(ui->refreshGradesButton, "secondary");
    setupMainShell();
    setMinimumSize(1050, 680);
    resize(1050, 680);

    ui->tabWidget->setTabText(0, "Persons");
    ui->tabWidget->setTabText(1, "Students");
    ui->tabWidget->setTabText(2, "Teachers");
    ui->tabWidget->setTabText(3, "Courses");
    ui->tabWidget->setTabText(4, "Terms");
    ui->tabWidget->setTabText(5, "Term Courses");
    ui->tabWidget->setTabText(6, "Grades");

    while (ui->tabWidget->count() > 7) {
        ui->tabWidget->removeTab(7);
    }

    ui->personMelliEdit->setPlaceholderText("Melli");
    ui->personFirstNameEdit->setPlaceholderText("First Name");
    ui->personFamilyEdit->setPlaceholderText("Family");
    ui->personAddressEdit->setPlaceholderText("Address");
    ui->personPostalCodeEdit->setPlaceholderText("Postal Code");
    ui->personTelephoneEdit->setPlaceholderText("Telephone");

    ui->studentIdEdit->setPlaceholderText("Student ID");
    ui->nationalIdEdit->setPlaceholderText("National ID");

    ui->entranceYearSpinBox->setMinimum(1300);
    ui->entranceYearSpinBox->setMaximum(1500);
    ui->entranceYearSpinBox->setValue(1404);

    ui->teacherIdEdit->setPlaceholderText("Teacher ID");
    ui->teacherMelliEdit->setPlaceholderText("Melli");

    ui->teacherHireDateEdit->setCalendar(QCalendar(QCalendar::System::Jalali));
    ui->teacherHireDateEdit->setDisplayFormat("yyyy/MM/dd");
    ui->teacherHireDateEdit->setCalendarPopup(true);
    ui->teacherHireDateEdit->setDate(QDate::currentDate());

    ui->courseIdEdit->setPlaceholderText("Course ID");
    ui->courseNameEdit->setPlaceholderText("Course Name");

    ui->courseCreditsSpinBox->setMinimum(1);
    ui->courseCreditsSpinBox->setMaximum(4);
    ui->courseCreditsSpinBox->setValue(3);

    ui->termIdEdit->setPlaceholderText("Term ID");
    ui->termNameEdit->setPlaceholderText("Term Name");

    ui->termYearSpinBox->setMinimum(1370);
    ui->termYearSpinBox->setMaximum(1430);
    ui->termYearSpinBox->setValue(1404);

    ui->termCourseIdEdit->setPlaceholderText("Term Course ID");

    ui->gradeIdEdit->setPlaceholderText("Grade ID");

    ui->gradeValueDoubleSpinBox->setMinimum(0.0);
    ui->gradeValueDoubleSpinBox->setMaximum(20.0);
    ui->gradeValueDoubleSpinBox->setDecimals(2);
    ui->gradeValueDoubleSpinBox->setSingleStep(0.25);
    ui->gradeValueDoubleSpinBox->setValue(10.0);

    makeComboBoxSearchable(ui->termCourseTermComboBox, "Select Term");
    makeComboBoxSearchable(ui->termCourseCourseComboBox, "Select Course");
    makeComboBoxSearchable(ui->termCourseTeacherComboBox, "Select Teacher");
    makeComboBoxSearchable(ui->gradeStudentComboBox, "Select Student");
    makeComboBoxSearchable(ui->gradeTermCourseComboBox, "Select Term Course");

    ui->personsSearchEdit->setPlaceholderText("Search...");
    ui->studentsSearchEdit->setPlaceholderText("Search...");
    ui->teachersSearchEdit->setPlaceholderText("Search...");
    ui->coursesSearchEdit->setPlaceholderText("Search...");
    ui->termsSearchEdit->setPlaceholderText("Search...");
    ui->termCoursesSearchEdit->setPlaceholderText("Search...");
    ui->gradesSearchEdit->setPlaceholderText("Search...");

    setupPersonsTable();
    setupStudentsTable();
    setupTeachersTable();
    setupCoursesTable();
    setupTermsTable();
    setupTermCoursesTable();
    setupGradesTable();

    connect(ui->addPersonButton, &QPushButton::clicked, this, &MainWindow::addPerson);
    connect(ui->refreshPersonsButton, &QPushButton::clicked, this, &MainWindow::loadPersons);
    connect(ui->deletePersonButton, &QPushButton::clicked, this, &MainWindow::deleteSelectedPerson);

    connect(ui->addStudentButton, &QPushButton::clicked, this, &MainWindow::addStudent);
    connect(ui->refreshStudentsButton, &QPushButton::clicked, this, &MainWindow::loadStudents);
    connect(ui->deleteStudentButton, &QPushButton::clicked, this, &MainWindow::deleteSelectedStudent);

    connect(ui->addTeacherButton, &QPushButton::clicked, this, &MainWindow::addTeacher);
    connect(ui->refreshTeachersButton, &QPushButton::clicked, this, &MainWindow::loadTeachers);
    connect(ui->deleteTeacherButton, &QPushButton::clicked, this, &MainWindow::deleteSelectedTeacher);

    connect(ui->addCourseButton, &QPushButton::clicked, this, &MainWindow::addCourse);
    connect(ui->refreshCoursesButton, &QPushButton::clicked, this, &MainWindow::loadCourses);
    connect(ui->deleteCourseButton, &QPushButton::clicked, this, &MainWindow::deleteSelectedCourse);

    connect(ui->addTermButton, &QPushButton::clicked, this, &MainWindow::addTerm);
    connect(ui->refreshTermsButton, &QPushButton::clicked, this, &MainWindow::loadTerms);
    connect(ui->deleteTermButton, &QPushButton::clicked, this, &MainWindow::deleteSelectedTerm);

    connect(ui->addTermCourseButton, &QPushButton::clicked, this, &MainWindow::addTermCourse);
    connect(ui->refreshTermCoursesButton, &QPushButton::clicked, this, &MainWindow::loadTermCourses);
    connect(ui->deleteTermCourseButton, &QPushButton::clicked, this, &MainWindow::deleteSelectedTermCourse);

    connect(ui->addGradeButton, &QPushButton::clicked, this, &MainWindow::addGrade);
    connect(ui->refreshGradesButton, &QPushButton::clicked, this, &MainWindow::loadGrades);
    connect(ui->deleteGradeButton, &QPushButton::clicked, this, &MainWindow::deleteSelectedGrade);

    connect(ui->updatePersonButton, &QPushButton::clicked, this, &MainWindow::updatePerson);
    connect(ui->updateStudentButton, &QPushButton::clicked, this, &MainWindow::updateStudent);
    connect(ui->updateTeacherButton, &QPushButton::clicked, this, &MainWindow::updateTeacher);
    connect(ui->updateCourseButton, &QPushButton::clicked, this, &MainWindow::updateCourse);
    connect(ui->updateTermButton, &QPushButton::clicked, this, &MainWindow::updateTerm);
    connect(ui->updateTermCourseButton, &QPushButton::clicked, this, &MainWindow::updateTermCourse);
    connect(ui->updateGradeButton, &QPushButton::clicked, this, &MainWindow::updateGrade);

    connect(ui->personsSearchEdit, &QLineEdit::textChanged, this, [this](const QString& text) {
        filterTable(ui->personsTable, text);
    });

    connect(ui->studentsSearchEdit, &QLineEdit::textChanged, this, [this](const QString& text) {
        filterTable(ui->studentsTable, text);
    });

    connect(ui->teachersSearchEdit, &QLineEdit::textChanged, this, [this](const QString& text) {
        filterTable(ui->teachersTable, text);
    });

    connect(ui->coursesSearchEdit, &QLineEdit::textChanged, this, [this](const QString& text) {
        filterTable(ui->coursesTable, text);
    });

    connect(ui->termsSearchEdit, &QLineEdit::textChanged, this, [this](const QString& text) {
        filterTable(ui->termsTable, text);
    });

    connect(ui->termCoursesSearchEdit, &QLineEdit::textChanged, this, [this](const QString& text) {
        filterTable(ui->termCoursesTable, text);
    });

    connect(ui->gradesSearchEdit, &QLineEdit::textChanged, this, [this](const QString& text) {
        filterTable(ui->gradesTable, text);
    });

    connect(ui->personsTable, &QTableWidget::itemSelectionChanged, this, &MainWindow::fillPersonFormFromSelection);
    connect(ui->studentsTable, &QTableWidget::itemSelectionChanged, this, &MainWindow::fillStudentFormFromSelection);
    connect(ui->teachersTable, &QTableWidget::itemSelectionChanged, this, &MainWindow::fillTeacherFormFromSelection);
    connect(ui->coursesTable, &QTableWidget::itemSelectionChanged, this, &MainWindow::fillCourseFormFromSelection);
    connect(ui->termsTable, &QTableWidget::itemSelectionChanged, this, &MainWindow::fillTermFormFromSelection);
    connect(ui->termCoursesTable, &QTableWidget::itemSelectionChanged, this, &MainWindow::fillTermCourseFormFromSelection);
    connect(ui->gradesTable, &QTableWidget::itemSelectionChanged, this, &MainWindow::fillGradeFormFromSelection);

    loadPersons();
    loadStudents();
    loadTeachers();
    loadCourses();
    loadTerms();
    loadTermCourses();
    loadGrades();
    ui->tabWidget->setCurrentIndex(0);
}

/** Releases the generated main-window UI. */
MainWindow::~MainWindow()
{
    delete ui;
}

/** Configures columns and selection behavior for the persons table. */
void MainWindow::setupPersonsTable()
{
    ui->personsTable->setColumnCount(6);

    ui->personsTable->setHorizontalHeaderLabels({
        "Melli",
        "First Name",
        "Family",
        "Address",
        "Postal Code",
        "Telephone"
    });

    ui->personsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->personsTable->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
    ui->personsTable->verticalHeader()->setVisible(false);

    ui->personsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->personsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->personsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

/** Configures columns and selection behavior for the students table. */
void MainWindow::setupStudentsTable()
{
    ui->studentsTable->setColumnCount(5);

    ui->studentsTable->setHorizontalHeaderLabels({
        "Student ID",
        "National ID",
        "Entrance Year",
        "Passed Credits",
        "Average"
    });

    ui->studentsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->studentsTable->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
    ui->studentsTable->verticalHeader()->setVisible(false);

    ui->studentsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->studentsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->studentsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

/** Configures columns and selection behavior for the teachers table. */
void MainWindow::setupTeachersTable()
{
    ui->teachersTable->setColumnCount(3);

    ui->teachersTable->setHorizontalHeaderLabels({
        "Teacher ID",
        "Melli",
        "Hire Date"
    });

    ui->teachersTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->teachersTable->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
    ui->teachersTable->verticalHeader()->setVisible(false);

    ui->teachersTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->teachersTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->teachersTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

/** Configures columns and selection behavior for the courses table. */
void MainWindow::setupCoursesTable()
{
    ui->coursesTable->setColumnCount(3);

    ui->coursesTable->setHorizontalHeaderLabels({
        "Course ID",
        "Course Name",
        "Credits"
    });

    ui->coursesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->coursesTable->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
    ui->coursesTable->verticalHeader()->setVisible(false);

    ui->coursesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->coursesTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->coursesTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

/** Configures columns and selection behavior for the terms table. */
void MainWindow::setupTermsTable()
{
    ui->termsTable->setColumnCount(3);

    ui->termsTable->setHorizontalHeaderLabels({
        "Term ID",
        "Term Name",
        "Year"
    });

    ui->termsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->termsTable->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
    ui->termsTable->verticalHeader()->setVisible(false);

    ui->termsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->termsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->termsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

/** Configures columns and selection behavior for the term-courses table. */
void MainWindow::setupTermCoursesTable()
{
    ui->termCoursesTable->setColumnCount(4);

    ui->termCoursesTable->setHorizontalHeaderLabels({
        "Term Course ID",
        "Term ID",
        "Course ID",
        "Teacher ID"
    });

    ui->termCoursesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->termCoursesTable->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
    ui->termCoursesTable->verticalHeader()->setVisible(false);

    ui->termCoursesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->termCoursesTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->termCoursesTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
}


/** Configures columns and selection behavior for the grades table. */
void MainWindow::setupGradesTable()
{
    ui->gradesTable->setColumnCount(5);

    ui->gradesTable->setHorizontalHeaderLabels({
        "Grade ID",
        "Student ID",
        "Term Course ID",
        "Grade",
        "Passed Credits"
    });

    ui->gradesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->gradesTable->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
    ui->gradesTable->verticalHeader()->setVisible(false);

    ui->gradesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->gradesTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->gradesTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
}



/** Reads person and detail files and displays merged person records. */
void MainWindow::loadPersons()
{
    std::vector<NationalCore> cores = readNationalCore("database/national_core.txt");
    std::vector<NationalDetails> details = readNationalDetails("database/national_details.txt");

    ui->personsTable->setRowCount(static_cast<int>(cores.size()));

    for (int row = 0; row < static_cast<int>(cores.size()); ++row) {
        const NationalCore& core = cores[row];

        QString address = "";
        QString postalCode = "";
        QString telephone = "";

        for (const NationalDetails& detail : details) {
            if (detail.melli == core.melli) {
                address = QString::fromStdString(detail.address);
                postalCode = QString::fromStdString(detail.postalCode);
                telephone = QString::fromStdString(detail.telephone);
                break;
            }
        }

        ui->personsTable->setItem(row, 0, makeCenteredItem(QString::fromStdString(core.melli)));
        ui->personsTable->setItem(row, 1, makeCenteredItem(QString::fromStdString(core.firstName)));
        ui->personsTable->setItem(row, 2, makeCenteredItem(QString::fromStdString(core.family)));
        ui->personsTable->setItem(row, 3, makeCenteredItem(address));
        ui->personsTable->setItem(row, 4, makeCenteredItem(postalCode));
        ui->personsTable->setItem(row, 5, makeCenteredItem(telephone));
    }

    filterTable(ui->personsTable, ui->personsSearchEdit->text());
}

/** Validates the person form and appends a new identity/detail record pair. */
void MainWindow::addPerson()
{
    const QString melli = ui->personMelliEdit->text().trimmed();
    const QString firstName = ui->personFirstNameEdit->text().trimmed();
    const QString family = ui->personFamilyEdit->text().trimmed();
    const QString address = ui->personAddressEdit->text().trimmed();
    const QString postalCode = ui->personPostalCodeEdit->text().trimmed();
    const QString telephone = ui->personTelephoneEdit->text().trimmed();

    if (
        melli.isEmpty() ||
        firstName.isEmpty() ||
        family.isEmpty() ||
        address.isEmpty() ||
        postalCode.isEmpty() ||
        telephone.isEmpty()
        ) {
        QMessageBox::warning(this, "Missing Data", "All person fields are required.");
        return;
    }

    if (
        containsPipe(melli) ||
        containsPipe(firstName) ||
        containsPipe(family) ||
        containsPipe(address) ||
        containsPipe(postalCode) ||
        containsPipe(telephone)
        ) {
        QMessageBox::warning(this, "Invalid Character", "The character | is not allowed because it is used by the database format.");
        return;
    }

    if (personExists(melli.toStdString())) {
        QMessageBox::warning(this, "Duplicate Person", "A person with this Melli already exists.");
        return;
    }

    std::vector<NationalCore> cores = readNationalCore("database/national_core.txt");
    std::vector<NationalDetails> details = readNationalDetails("database/national_details.txt");

    NationalCore core;
    core.melli = melli.toStdString();
    core.firstName = firstName.toStdString();
    core.family = family.toStdString();

    NationalDetails detail;
    detail.melli = melli.toStdString();
    detail.address = address.toStdString();
    detail.postalCode = postalCode.toStdString();
    detail.telephone = telephone.toStdString();

    cores.push_back(core);
    details.push_back(detail);

    writeNationalCore("database/national_core.txt", cores);
    writeNationalDetails("database/national_details.txt", details);

    ui->personMelliEdit->clear();
    ui->personFirstNameEdit->clear();
    ui->personFamilyEdit->clear();
    ui->personAddressEdit->clear();
    ui->personPostalCodeEdit->clear();
    ui->personTelephoneEdit->clear();

    loadPersons();

    QMessageBox::information(this, "Success", "Person added successfully.");
}

/** Deletes the selected person only when no student or teacher depends on it. */
void MainWindow::deleteSelectedPerson()
{
    const int selectedRow = ui->personsTable->currentRow();

    if (selectedRow < 0) {
        QMessageBox::warning(this, "No Selection", "Please select a person row first.");
        return;
    }

    QTableWidgetItem* melliItem = ui->personsTable->item(selectedRow, 0);

    if (melliItem == nullptr) {
        QMessageBox::warning(this, "Invalid Selection", "Selected row does not contain a valid Melli.");
        return;
    }

    const QString melli = melliItem->text();

    std::vector<Student> students = readStudents("database/students.txt");

    const bool usedByStudent = std::any_of(
        students.begin(),
        students.end(),
        [&melli](const Student& student) {
            return student.melli == melli.toStdString();
        }
        );

    if (usedByStudent) {
        QMessageBox::warning(this, "Cannot Delete", "This person is registered as a student. Delete the student first.");
        return;
    }

    std::vector<Teacher> teachers = readTeachers("database/teachers.txt");

    const bool usedByTeacher = std::any_of(
        teachers.begin(),
        teachers.end(),
        [&melli](const Teacher& teacher) {
            return teacher.melli == melli.toStdString();
        }
        );

    if (usedByTeacher) {
        QMessageBox::warning(this, "Cannot Delete", "This person is registered as a teacher. Delete the teacher first.");
        return;
    }

    const QMessageBox::StandardButton answer = QMessageBox::question(
        this,
        "Delete Person",
        "Are you sure you want to delete person " + melli + "?"
        );

    if (answer != QMessageBox::Yes) {
        return;
    }

    std::vector<NationalCore> cores = readNationalCore("database/national_core.txt");
    std::vector<NationalDetails> details = readNationalDetails("database/national_details.txt");

    cores.erase(
        std::remove_if(
            cores.begin(),
            cores.end(),
            [&melli](const NationalCore& core) {
                return core.melli == melli.toStdString();
            }
            ),
        cores.end()
        );

    details.erase(
        std::remove_if(
            details.begin(),
            details.end(),
            [&melli](const NationalDetails& detail) {
                return detail.melli == melli.toStdString();
            }
            ),
        details.end()
        );

    writeNationalCore("database/national_core.txt", cores);
    writeNationalDetails("database/national_details.txt", details);

    loadPersons();

    QMessageBox::information(this, "Deleted", "Person deleted successfully.");
}

/** Reads students and related person names into the students table. */
void MainWindow::loadStudents()
{
    std::vector<Student> students = readStudents("database/students.txt");

    ui->studentsTable->setRowCount(static_cast<int>(students.size()));

    for (int row = 0; row < static_cast<int>(students.size()); ++row) {
        const Student& student = students[row];

        ui->studentsTable->setItem(row, 0, makeCenteredItem(QString::fromStdString(student.studentId)));
        ui->studentsTable->setItem(row, 1, makeCenteredItem(QString::fromStdString(student.melli)));
        ui->studentsTable->setItem(row, 2, makeCenteredItem(QString::number(student.entranceYear)));
        ui->studentsTable->setItem(row, 3, makeCenteredItem(QString::number(student.passed)));
        ui->studentsTable->setItem(row, 4, makeCenteredItem(QString::number(student.grade, 'f', 2)));
    }

    filterTable(ui->studentsTable, ui->studentsSearchEdit->text());
}

/** Validates and appends a new student record linked to an existing person. */
void MainWindow::addStudent()
{
    const QString studentId = ui->studentIdEdit->text().trimmed();
    const QString nationalId = ui->nationalIdEdit->text().trimmed();
    const int entranceYear = ui->entranceYearSpinBox->value();

    if (studentId.isEmpty() || nationalId.isEmpty()) {
        QMessageBox::warning(this, "Missing Data", "Student ID and National ID are required.");
        return;
    }

    if (containsPipe(studentId) || containsPipe(nationalId)) {
        QMessageBox::warning(this, "Invalid Character", "The character | is not allowed because it is used by the database format.");
        return;
    }

    std::vector<Student> students = readStudents("database/students.txt");

    const bool duplicateStudentId = std::any_of(
        students.begin(),
        students.end(),
        [&studentId](const Student& student) {
            return student.studentId == studentId.toStdString();
        }
        );

    if (duplicateStudentId) {
        QMessageBox::warning(this, "Duplicate Student", "A student with this Student ID already exists.");
        return;
    }

    if (!personExists(nationalId.toStdString())) {
        QMessageBox::warning(this, "Person Not Found", "This National ID does not exist. Please add the person first.");
        return;
    }

    const bool melliAlreadyStudent = std::any_of(
        students.begin(),
        students.end(),
        [&nationalId](const Student& student) {
            return student.melli == nationalId.toStdString();
        }
        );

    if (melliAlreadyStudent) {
        QMessageBox::warning(this, "Duplicate National ID", "This National ID is already registered as a student.");
        return;
    }

    Student student;
    student.studentId = studentId.toStdString();
    student.melli = nationalId.toStdString();
    student.entranceYear = entranceYear;
    student.passed = 0;
    student.grade = 0.0;

    students.push_back(student);

    writeStudents("database/students.txt", students);

    ui->studentIdEdit->clear();
    ui->nationalIdEdit->clear();
    ui->entranceYearSpinBox->setValue(1404);

    loadStudents();
    loadGrades();

    QMessageBox::information(this, "Success", "Student added successfully.");
}

/** Deletes the selected student only when no grade records depend on it. */
void MainWindow::deleteSelectedStudent()
{
    const int selectedRow = ui->studentsTable->currentRow();

    if (selectedRow < 0) {
        QMessageBox::warning(this, "No Selection", "Please select a student row first.");
        return;
    }

    QTableWidgetItem* idItem = ui->studentsTable->item(selectedRow, 0);

    if (idItem == nullptr) {
        QMessageBox::warning(this, "Invalid Selection", "Selected row does not contain a valid Student ID.");
        return;
    }

    const QString studentId = idItem->text();

    std::vector<Grade> grades = readGrades("database/grades.txt");

    const bool usedByGrade = std::any_of(
        grades.begin(),
        grades.end(),
        [&studentId](const Grade& grade) {
            return grade.studentId == studentId.toStdString();
        }
        );

    if (usedByGrade) {
        QMessageBox::warning(
            this,
            "Cannot Delete",
            "This student has grades. Delete those grades first."
            );
        return;
    }

    const QMessageBox::StandardButton answer = QMessageBox::question(
        this,
        "Delete Student",
        "Are you sure you want to delete student " + studentId + "?"
        );

    if (answer != QMessageBox::Yes) {
        return;
    }

    std::vector<Student> students = readStudents("database/students.txt");

    students.erase(
        std::remove_if(
            students.begin(),
            students.end(),
            [&studentId](const Student& student) {
                return student.studentId == studentId.toStdString();
            }
            ),
        students.end()
        );

    writeStudents("database/students.txt", students);

    loadStudents();
    loadGrades();

    QMessageBox::information(this, "Deleted", "Student deleted successfully.");
}

/** Reads teachers and related person names into the teachers table. */
void MainWindow::loadTeachers()
{
    std::vector<Teacher> teachers = readTeachers("database/teachers.txt");

    ui->teachersTable->setRowCount(static_cast<int>(teachers.size()));

    for (int row = 0; row < static_cast<int>(teachers.size()); ++row) {
        const Teacher& teacher = teachers[row];

        ui->teachersTable->setItem(row, 0, makeCenteredItem(QString::fromStdString(teacher.teacherId)));
        ui->teachersTable->setItem(row, 1, makeCenteredItem(QString::fromStdString(teacher.melli)));
        ui->teachersTable->setItem(row, 2, makeCenteredItem(QString::fromStdString(teacher.hireDate)));
    }

    filterTable(ui->teachersTable, ui->teachersSearchEdit->text());
}

/** Validates and appends a new teacher record linked to an existing person. */
void MainWindow::addTeacher()
{
    const QString teacherId = ui->teacherIdEdit->text().trimmed();
    const QString melli = ui->teacherMelliEdit->text().trimmed();
    const QString hireDate = ui->teacherHireDateEdit->text();

    if (teacherId.isEmpty() || melli.isEmpty() || hireDate.isEmpty()) {
        QMessageBox::warning(this, "Missing Data", "Teacher ID, Melli, and Hire Date are required.");
        return;
    }

    if (containsPipe(teacherId) || containsPipe(melli) || containsPipe(hireDate)) {
        QMessageBox::warning(this, "Invalid Character", "The character | is not allowed because it is used by the database format.");
        return;
    }

    std::vector<Teacher> teachers = readTeachers("database/teachers.txt");

    const bool duplicateTeacherId = std::any_of(
        teachers.begin(),
        teachers.end(),
        [&teacherId](const Teacher& teacher) {
            return teacher.teacherId == teacherId.toStdString();
        }
        );

    if (duplicateTeacherId) {
        QMessageBox::warning(this, "Duplicate Teacher", "A teacher with this Teacher ID already exists.");
        return;
    }

    if (!personExists(melli.toStdString())) {
        QMessageBox::warning(this, "Person Not Found", "This Melli does not exist. Please add the person first.");
        return;
    }

    Teacher teacher;
    teacher.teacherId = teacherId.toStdString();
    teacher.melli = melli.toStdString();
    teacher.hireDate = hireDate.toStdString();

    teachers.push_back(teacher);

    writeTeachers("database/teachers.txt", teachers);

    ui->teacherIdEdit->clear();
    ui->teacherMelliEdit->clear();
    ui->teacherHireDateEdit->setDate(QDate::currentDate());

    loadTeachers();

    QMessageBox::information(this, "Success", "Teacher added successfully.");
}

/** Deletes the selected teacher only when no term-course records depend on it. */
void MainWindow::deleteSelectedTeacher()
{
    const int selectedRow = ui->teachersTable->currentRow();

    if (selectedRow < 0) {
        QMessageBox::warning(this, "No Selection", "Please select a teacher row first.");
        return;
    }

    QTableWidgetItem* idItem = ui->teachersTable->item(selectedRow, 0);

    if (idItem == nullptr) {
        QMessageBox::warning(this, "Invalid Selection", "Selected row does not contain a valid Teacher ID.");
        return;
    }

    const QString teacherId = idItem->text();

    std::vector<TermCourse> termCourses = readTermCourses("database/term_courses.txt");

    const bool usedByTermCourse = std::any_of(
        termCourses.begin(),
        termCourses.end(),
        [&teacherId](const TermCourse& termCourse) {
            return termCourse.teacherId == teacherId.toStdString();
        }
        );

    if (usedByTermCourse) {
        QMessageBox::warning(this, "Cannot Delete", "This teacher is used in a term course. Delete that term course first.");
        return;
    }

    const QMessageBox::StandardButton answer = QMessageBox::question(
        this,
        "Delete Teacher",
        "Are you sure you want to delete teacher " + teacherId + "?"
        );

    if (answer != QMessageBox::Yes) {
        return;
    }

    std::vector<Teacher> teachers = readTeachers("database/teachers.txt");

    teachers.erase(
        std::remove_if(
            teachers.begin(),
            teachers.end(),
            [&teacherId](const Teacher& teacher) {
                return teacher.teacherId == teacherId.toStdString();
            }
            ),
        teachers.end()
        );

    writeTeachers("database/teachers.txt", teachers);

    loadTeachers();

    QMessageBox::information(this, "Deleted", "Teacher deleted successfully.");
}

/** Reads course records into the courses table. */
void MainWindow::loadCourses()
{
    std::vector<Course> courses = readCourses("database/courses.txt");

    ui->coursesTable->setRowCount(static_cast<int>(courses.size()));

    for (int row = 0; row < static_cast<int>(courses.size()); ++row) {
        const Course& course = courses[row];

        ui->coursesTable->setItem(row, 0, makeCenteredItem(QString::fromStdString(course.courseId)));
        ui->coursesTable->setItem(row, 1, makeCenteredItem(QString::fromStdString(course.courseName)));
        ui->coursesTable->setItem(row, 2, makeCenteredItem(QString::number(course.credits)));
    }

    filterTable(ui->coursesTable, ui->coursesSearchEdit->text());
}

/** Validates and appends a new course record. */
void MainWindow::addCourse()
{
    const QString courseId = ui->courseIdEdit->text().trimmed();
    const QString courseName = ui->courseNameEdit->text().trimmed();
    const int credits = ui->courseCreditsSpinBox->value();

    if (courseId.isEmpty() || courseName.isEmpty()) {
        QMessageBox::warning(this, "Missing Data", "Course ID and Course Name are required.");
        return;
    }

    if (containsPipe(courseId) || containsPipe(courseName)) {
        QMessageBox::warning(this, "Invalid Character", "The character | is not allowed because it is used by the database format.");
        return;
    }

    if (credits < 1 || credits > 4) {
        QMessageBox::warning(this, "Invalid Credits", "Course credits must be between 1 and 4.");
        return;
    }

    std::vector<Course> courses = readCourses("database/courses.txt");

    const bool duplicateCourseId = std::any_of(
        courses.begin(),
        courses.end(),
        [&courseId](const Course& course) {
            return course.courseId == courseId.toStdString();
        }
        );

    if (duplicateCourseId) {
        QMessageBox::warning(this, "Duplicate Course", "A course with this Course ID already exists.");
        return;
    }

    Course course;
    course.courseId = courseId.toStdString();
    course.courseName = courseName.toStdString();
    course.credits = credits;

    courses.push_back(course);

    writeCourses("database/courses.txt", courses);

    ui->courseIdEdit->clear();
    ui->courseNameEdit->clear();
    ui->courseCreditsSpinBox->setValue(3);

    loadCourses();

    QMessageBox::information(this, "Success", "Course added successfully.");
}

/** Deletes the selected course only when no term-course records depend on it. */
void MainWindow::deleteSelectedCourse()
{
    const int selectedRow = ui->coursesTable->currentRow();

    if (selectedRow < 0) {
        QMessageBox::warning(this, "No Selection", "Please select a course row first.");
        return;
    }

    QTableWidgetItem* idItem = ui->coursesTable->item(selectedRow, 0);

    if (idItem == nullptr) {
        QMessageBox::warning(this, "Invalid Selection", "Selected row does not contain a valid Course ID.");
        return;
    }

    const QString courseId = idItem->text();

    std::vector<TermCourse> termCourses = readTermCourses("database/term_courses.txt");

    const bool usedByTermCourse = std::any_of(
        termCourses.begin(),
        termCourses.end(),
        [&courseId](const TermCourse& termCourse) {
            return termCourse.courseId == courseId.toStdString();
        }
        );

    if (usedByTermCourse) {
        QMessageBox::warning(this, "Cannot Delete", "This course is used in a term course. Delete that term course first.");
        return;
    }

    const QMessageBox::StandardButton answer = QMessageBox::question(
        this,
        "Delete Course",
        "Are you sure you want to delete course " + courseId + "?"
        );

    if (answer != QMessageBox::Yes) {
        return;
    }

    std::vector<Course> courses = readCourses("database/courses.txt");

    courses.erase(
        std::remove_if(
            courses.begin(),
            courses.end(),
            [&courseId](const Course& course) {
                return course.courseId == courseId.toStdString();
            }
            ),
        courses.end()
        );

    writeCourses("database/courses.txt", courses);

    loadCourses();

    QMessageBox::information(this, "Deleted", "Course deleted successfully.");
}

/** Reads academic term records into the terms table. */
void MainWindow::loadTerms()
{
    std::vector<Term> terms = readTerms("database/terms.txt");

    ui->termsTable->setRowCount(static_cast<int>(terms.size()));

    for (int row = 0; row < static_cast<int>(terms.size()); ++row) {
        const Term& term = terms[row];

        ui->termsTable->setItem(
            row,
            0,
            makeCenteredItem(QString::fromStdString(term.termId))
            );

        ui->termsTable->setItem(
            row,
            1,
            makeCenteredItem(QString::fromStdString(term.termName))
            );

        ui->termsTable->setItem(
            row,
            2,
            makeCenteredItem(QString::number(term.year))
            );
    }

    filterTable(ui->termsTable, ui->termsSearchEdit->text());
}

/** Validates and appends a new academic term record. */
void MainWindow::addTerm()
{
    const QString termId = ui->termIdEdit->text().trimmed();
    const QString termName = ui->termNameEdit->text().trimmed();
    const int year = ui->termYearSpinBox->value();

    if (termId.isEmpty() || termName.isEmpty()) {
        QMessageBox::warning(
            this,
            "Missing Data",
            "Term ID and Term Name are required."
            );
        return;
    }

    if (containsPipe(termId) || containsPipe(termName)) {
        QMessageBox::warning(
            this,
            "Invalid Character",
            "The character | is not allowed because it is used by the database format."
            );
        return;
    }

    if (year < 1370 || year > 1430) {
        QMessageBox::warning(
            this,
            "Invalid Year",
            "Term year must be between 1370 and 1430."
            );
        return;
    }

    std::vector<Term> terms = readTerms("database/terms.txt");

    const bool duplicateTermId = std::any_of(
        terms.begin(),
        terms.end(),
        [&termId](const Term& term) {
            return term.termId == termId.toStdString();
        }
        );

    if (duplicateTermId) {
        QMessageBox::warning(
            this,
            "Duplicate Term",
            "A term with this Term ID already exists."
            );
        return;
    }

    Term term;
    term.termId = termId.toStdString();
    term.termName = termName.toStdString();
    term.year = year;

    terms.push_back(term);

    writeTerms("database/terms.txt", terms);

    ui->termIdEdit->clear();
    ui->termNameEdit->clear();
    ui->termYearSpinBox->setValue(1404);

    loadTerms();

    QMessageBox::information(
        this,
        "Success",
        "Term added successfully."
        );
}

/** Deletes the selected term only when no term-course records depend on it. */
void MainWindow::deleteSelectedTerm()
{
    const int selectedRow = ui->termsTable->currentRow();

    if (selectedRow < 0) {
        QMessageBox::warning(
            this,
            "No Selection",
            "Please select a term row first."
            );
        return;
    }

    QTableWidgetItem* idItem = ui->termsTable->item(selectedRow, 0);

    if (idItem == nullptr) {
        QMessageBox::warning(
            this,
            "Invalid Selection",
            "Selected row does not contain a valid Term ID."
            );
        return;
    }

    const QString termId = idItem->text();

    std::vector<TermCourse> termCourses =
        readTermCourses("database/term_courses.txt");

    const bool usedByTermCourse = std::any_of(
        termCourses.begin(),
        termCourses.end(),
        [&termId](const TermCourse& termCourse) {
            return termCourse.termId == termId.toStdString();
        }
        );

    if (usedByTermCourse) {
        QMessageBox::warning(
            this,
            "Cannot Delete",
            "This term is used in a term course. Delete that term course first."
            );
        return;
    }

    const QMessageBox::StandardButton answer = QMessageBox::question(
        this,
        "Delete Term",
        "Are you sure you want to delete term " + termId + "?"
        );

    if (answer != QMessageBox::Yes) {
        return;
    }

    std::vector<Term> terms = readTerms("database/terms.txt");

    terms.erase(
        std::remove_if(
            terms.begin(),
            terms.end(),
            [&termId](const Term& term) {
                return term.termId == termId.toStdString();
            }
            ),
        terms.end()
        );

    writeTerms("database/terms.txt", terms);

    loadTerms();

    QMessageBox::information(
        this,
        "Deleted",
        "Term deleted successfully."
        );
}

/** Reads term-course assignments and displays resolved term, course, and teacher names. */
void MainWindow::loadTermCourses()
{
    std::vector<Term> terms = readTerms("database/terms.txt");
    std::vector<Course> courses = readCourses("database/courses.txt");
    std::vector<Teacher> teachers = readTeachers("database/teachers.txt");
    std::vector<NationalCore> people = readNationalCore("database/national_core.txt");

    ui->termCourseTermComboBox->clear();
    ui->termCourseCourseComboBox->clear();
    ui->termCourseTeacherComboBox->clear();

    for (const Term& term : terms) {
        const QString termId = QString::fromStdString(term.termId);
        const QString termName = QString::fromStdString(term.termName);

        ui->termCourseTermComboBox->addItem(
            termId + " - " + termName + " (" + QString::number(term.year) + ")",
            termId
            );
    }

    for (const Course& course : courses) {
        const QString courseId = QString::fromStdString(course.courseId);
        const QString courseName = QString::fromStdString(course.courseName);

        ui->termCourseCourseComboBox->addItem(
            courseId + " - " + courseName,
            courseId
            );
    }

    for (const Teacher& teacher : teachers) {
        const QString teacherId = QString::fromStdString(teacher.teacherId);
        const QString teacherName = getPersonNameByMelli(teacher.melli, people);

        ui->termCourseTeacherComboBox->addItem(
            teacherName + " - " + teacherId,
            teacherId
            );
    }

    std::vector<TermCourse> termCourses =
        readTermCourses("database/term_courses.txt");

    ui->termCoursesTable->setRowCount(static_cast<int>(termCourses.size()));

    for (int row = 0; row < static_cast<int>(termCourses.size()); ++row) {
        const TermCourse& termCourse = termCourses[row];

        ui->termCoursesTable->setItem(
            row,
            0,
            makeCenteredItem(QString::fromStdString(termCourse.termCourseId))
            );

        ui->termCoursesTable->setItem(
            row,
            1,
            makeCenteredItem(QString::fromStdString(termCourse.termId))
            );

        ui->termCoursesTable->setItem(
            row,
            2,
            makeCenteredItem(QString::fromStdString(termCourse.courseId))
            );

        ui->termCoursesTable->setItem(
            row,
            3,
            makeCenteredItem(QString::fromStdString(termCourse.teacherId))
            );
    }

    resetComboBoxPlaceholder(ui->termCourseTermComboBox);
    resetComboBoxPlaceholder(ui->termCourseCourseComboBox);
    resetComboBoxPlaceholder(ui->termCourseTeacherComboBox);

    filterTable(ui->termCoursesTable, ui->termCoursesSearchEdit->text());
}

/** Validates selected term/course/teacher values and creates a term-course assignment. */
void MainWindow::addTermCourse()
{
    const QString termCourseId = ui->termCourseIdEdit->text().trimmed();

    if (termCourseId.isEmpty()) {
        QMessageBox::warning(
            this,
            "Missing Data",
            "Term Course ID is required."
            );
        return;
    }

    if (containsPipe(termCourseId)) {
        QMessageBox::warning(
            this,
            "Invalid Character",
            "The character | is not allowed because it is used by the database format."
            );
        return;
    }

    if (ui->termCourseTermComboBox->count() == 0) {
        QMessageBox::warning(
            this,
            "No Term",
            "Please add at least one term first."
            );
        return;
    }

    if (ui->termCourseCourseComboBox->count() == 0) {
        QMessageBox::warning(
            this,
            "No Course",
            "Please add at least one course first."
            );
        return;
    }

    if (ui->termCourseTeacherComboBox->count() == 0) {
        QMessageBox::warning(
            this,
            "No Teacher",
            "Please add at least one teacher first."
            );
        return;
    }

    if (!comboBoxHasValidSelection(ui->termCourseTermComboBox)) {
        QMessageBox::warning(
            this,
            "Invalid Term",
            "Please select a valid term from the list."
            );
        return;
    }

    if (!comboBoxHasValidSelection(ui->termCourseCourseComboBox)) {
        QMessageBox::warning(
            this,
            "Invalid Course",
            "Please select a valid course from the list."
            );
        return;
    }

    if (!comboBoxHasValidSelection(ui->termCourseTeacherComboBox)) {
        QMessageBox::warning(
            this,
            "Invalid Teacher",
            "Please select a valid teacher from the list."
            );
        return;
    }

    const QString termId =
        ui->termCourseTermComboBox->currentData().toString();

    const QString courseId =
        ui->termCourseCourseComboBox->currentData().toString();

    const QString teacherId =
        ui->termCourseTeacherComboBox->currentData().toString();

    std::vector<TermCourse> termCourses =
        readTermCourses("database/term_courses.txt");

    const bool duplicateTermCourseId = std::any_of(
        termCourses.begin(),
        termCourses.end(),
        [&termCourseId](const TermCourse& termCourse) {
            return termCourse.termCourseId == termCourseId.toStdString();
        }
        );

    if (duplicateTermCourseId) {
        QMessageBox::warning(
            this,
            "Duplicate Term Course",
            "A term course with this ID already exists."
            );
        return;
    }

    TermCourse termCourse;
    termCourse.termCourseId = termCourseId.toStdString();
    termCourse.termId = termId.toStdString();
    termCourse.courseId = courseId.toStdString();
    termCourse.teacherId = teacherId.toStdString();

    termCourses.push_back(termCourse);

    writeTermCourses("database/term_courses.txt", termCourses);

    ui->termCourseIdEdit->clear();

    loadTermCourses();
    loadGrades();

    QMessageBox::information(
        this,
        "Success",
        "Term course added successfully."
        );
}

/** Deletes the selected term-course only when no grade records depend on it. */
void MainWindow::deleteSelectedTermCourse()
{
    const int selectedRow = ui->termCoursesTable->currentRow();

    if (selectedRow < 0) {
        QMessageBox::warning(
            this,
            "No Selection",
            "Please select a term course row first."
            );
        return;
    }

    QTableWidgetItem* idItem = ui->termCoursesTable->item(selectedRow, 0);

    if (idItem == nullptr) {
        QMessageBox::warning(
            this,
            "Invalid Selection",
            "Selected row does not contain a valid Term Course ID."
            );
        return;
    }

    const QString termCourseId = idItem->text();

    std::vector<Grade> grades =
        readGrades("database/grades.txt");

    const bool usedByGrade = std::any_of(
        grades.begin(),
        grades.end(),
        [&termCourseId](const Grade& grade) {
            return grade.termCourseId == termCourseId.toStdString();
        }
        );

    if (usedByGrade) {
        QMessageBox::warning(
            this,
            "Cannot Delete",
            "This term course has grades. Delete those grades first."
            );
        return;
    }

    const QMessageBox::StandardButton answer = QMessageBox::question(
        this,
        "Delete Term Course",
        "Are you sure you want to delete term course " + termCourseId + "?"
        );

    if (answer != QMessageBox::Yes) {
        return;
    }

    std::vector<TermCourse> termCourses =
        readTermCourses("database/term_courses.txt");

    termCourses.erase(
        std::remove_if(
            termCourses.begin(),
            termCourses.end(),
            [&termCourseId](const TermCourse& termCourse) {
                return termCourse.termCourseId == termCourseId.toStdString();
            }
            ),
        termCourses.end()
        );

    writeTermCourses("database/term_courses.txt", termCourses);

    loadTermCourses();
    loadGrades();

    QMessageBox::information(
        this,
        "Deleted",
        "Term course deleted successfully."
        );
}

/** Reads grade records and displays resolved student and term-course labels. */
void MainWindow::loadGrades()
{
    std::vector<Student> students = readStudents("database/students.txt");
    std::vector<TermCourse> termCourses = readTermCourses("database/term_courses.txt");
    std::vector<Grade> grades = readGrades("database/grades.txt");
    std::vector<NationalCore> people = readNationalCore("database/national_core.txt");

    ui->gradeStudentComboBox->clear();
    ui->gradeTermCourseComboBox->clear();

    for (const Student& student : students) {
        const QString studentName = getPersonNameByMelli(student.melli, people);

        ui->gradeStudentComboBox->addItem(
            studentName + " - " + QString::fromStdString(student.studentId),
            QString::fromStdString(student.studentId)
            );
    }

    for (const TermCourse& termCourse : termCourses) {
        const QString termCourseId = QString::fromStdString(termCourse.termCourseId);
        const QString termId = QString::fromStdString(termCourse.termId);
        const QString courseId = QString::fromStdString(termCourse.courseId);
        const QString teacherId = QString::fromStdString(termCourse.teacherId);

        ui->gradeTermCourseComboBox->addItem(
            termCourseId + " - Term: " + termId +
                " | Course: " + courseId +
                " | Teacher: " + teacherId,
            termCourseId
            );
    }

    ui->gradesTable->setRowCount(static_cast<int>(grades.size()));

    for (int row = 0; row < static_cast<int>(grades.size()); ++row) {
        const Grade& grade = grades[row];

        ui->gradesTable->setItem(
            row,
            0,
            makeCenteredItem(QString::fromStdString(grade.gradeId))
            );

        ui->gradesTable->setItem(
            row,
            1,
            makeCenteredItem(QString::fromStdString(grade.studentId))
            );

        ui->gradesTable->setItem(
            row,
            2,
            makeCenteredItem(QString::fromStdString(grade.termCourseId))
            );

        ui->gradesTable->setItem(
            row,
            3,
            makeCenteredItem(QString::number(grade.gradeValue, 'f', 2))
            );

        ui->gradesTable->setItem(
            row,
            4,
            makeCenteredItem(QString::number(grade.passed))
            );
    }

    resetComboBoxPlaceholder(ui->gradeStudentComboBox);
    resetComboBoxPlaceholder(ui->gradeTermCourseComboBox);

    filterTable(ui->gradesTable, ui->gradesSearchEdit->text());
}

/** Adds a grade, calculates passed credits, and refreshes student statistics. */
void MainWindow::addGrade()
{
    const QString gradeId = ui->gradeIdEdit->text().trimmed();
    const double gradeValue = ui->gradeValueDoubleSpinBox->value();

    if (gradeId.isEmpty()) {
        QMessageBox::warning(
            this,
            "Missing Data",
            "Grade ID is required."
            );
        return;
    }

    if (containsPipe(gradeId)) {
        QMessageBox::warning(
            this,
            "Invalid Character",
            "The character | is not allowed because it is used by the database format."
            );
        return;
    }

    if (ui->gradeStudentComboBox->count() == 0) {
        QMessageBox::warning(
            this,
            "No Student",
            "Please add at least one student first."
            );
        return;
    }

    if (ui->gradeTermCourseComboBox->count() == 0) {
        QMessageBox::warning(
            this,
            "No Term Course",
            "Please add at least one term course first."
            );
        return;
    }

    if (!comboBoxHasValidSelection(ui->gradeStudentComboBox)) {
        QMessageBox::warning(
            this,
            "Invalid Student",
            "Please select a valid student from the list."
            );
        return;
    }

    if (!comboBoxHasValidSelection(ui->gradeTermCourseComboBox)) {
        QMessageBox::warning(
            this,
            "Invalid Term Course",
            "Please select a valid term course from the list."
            );
        return;
    }

    const QString studentId =
        ui->gradeStudentComboBox->currentData().toString();

    const QString termCourseId =
        ui->gradeTermCourseComboBox->currentData().toString();

    std::vector<Grade> grades = readGrades("database/grades.txt");

    const bool duplicateGradeId = std::any_of(
        grades.begin(),
        grades.end(),
        [&gradeId](const Grade& grade) {
            return grade.gradeId == gradeId.toStdString();
        }
        );

    if (duplicateGradeId) {
        QMessageBox::warning(
            this,
            "Duplicate Grade",
            "A grade with this Grade ID already exists."
            );
        return;
    }

    const bool duplicateStudentTermCourse = std::any_of(
        grades.begin(),
        grades.end(),
        [&studentId, &termCourseId](const Grade& grade) {
            return grade.studentId == studentId.toStdString() &&
                   grade.termCourseId == termCourseId.toStdString();
        }
        );

    if (duplicateStudentTermCourse) {
        QMessageBox::warning(
            this,
            "Duplicate Grade",
            "This student already has a grade for this term course."
            );
        return;
    }

    std::vector<TermCourse> termCourses =
        readTermCourses("database/term_courses.txt");

    std::vector<Course> courses =
        readCourses("database/courses.txt");

    const int credits = findCourseCreditsByTermCourseId(
        termCourseId.toStdString(),
        termCourses,
        courses
        );

    Grade grade;
    grade.gradeId = gradeId.toStdString();
    grade.termCourseId = termCourseId.toStdString();
    grade.studentId = studentId.toStdString();
    grade.gradeValue = gradeValue;

    if (gradeValue >= 10.0) {
        grade.passed = credits;
    } else {
        grade.passed = 0;
    }

    grades.push_back(grade);

    writeGrades("database/grades.txt", grades);

    recalculateStudentStatistics();

    ui->gradeIdEdit->clear();
    ui->gradeValueDoubleSpinBox->setValue(10.0);

    loadGrades();
    loadStudents();

    QMessageBox::information(
        this,
        "Success",
        "Grade added successfully."
        );
}

/** Deletes the selected grade and refreshes student statistics. */
void MainWindow::deleteSelectedGrade()
{
    const int selectedRow = ui->gradesTable->currentRow();

    if (selectedRow < 0) {
        QMessageBox::warning(
            this,
            "No Selection",
            "Please select a grade row first."
            );
        return;
    }

    QTableWidgetItem* idItem = ui->gradesTable->item(selectedRow, 0);

    if (idItem == nullptr) {
        QMessageBox::warning(
            this,
            "Invalid Selection",
            "Selected row does not contain a valid Grade ID."
            );
        return;
    }

    const QString gradeId = idItem->text();

    const QMessageBox::StandardButton answer = QMessageBox::question(
        this,
        "Delete Grade",
        "Are you sure you want to delete grade " + gradeId + "?"
        );

    if (answer != QMessageBox::Yes) {
        return;
    }

    std::vector<Grade> grades = readGrades("database/grades.txt");

    grades.erase(
        std::remove_if(
            grades.begin(),
            grades.end(),
            [&gradeId](const Grade& grade) {
                return grade.gradeId == gradeId.toStdString();
            }
            ),
        grades.end()
        );

    writeGrades("database/grades.txt", grades);

    recalculateStudentStatistics();

    loadGrades();
    loadStudents();

    QMessageBox::information(
        this,
        "Deleted",
        "Grade deleted successfully."
        );
}

/** Copies the selected person row into the editable form fields. */
void MainWindow::fillPersonFormFromSelection()
{
    QTableWidgetItem* melliItem = currentRowItem(ui->personsTable, 0);
    QTableWidgetItem* firstNameItem = currentRowItem(ui->personsTable, 1);
    QTableWidgetItem* familyItem = currentRowItem(ui->personsTable, 2);
    QTableWidgetItem* addressItem = currentRowItem(ui->personsTable, 3);
    QTableWidgetItem* postalCodeItem = currentRowItem(ui->personsTable, 4);
    QTableWidgetItem* telephoneItem = currentRowItem(ui->personsTable, 5);

    if (melliItem == nullptr) {
        return;
    }

    ui->personMelliEdit->setText(melliItem->text());
    ui->personFirstNameEdit->setText(firstNameItem != nullptr ? firstNameItem->text() : "");
    ui->personFamilyEdit->setText(familyItem != nullptr ? familyItem->text() : "");
    ui->personAddressEdit->setText(addressItem != nullptr ? addressItem->text() : "");
    ui->personPostalCodeEdit->setText(postalCodeItem != nullptr ? postalCodeItem->text() : "");
    ui->personTelephoneEdit->setText(telephoneItem != nullptr ? telephoneItem->text() : "");
}

/** Copies the selected student row into the editable form fields. */
void MainWindow::fillStudentFormFromSelection()
{
    QTableWidgetItem* studentIdItem = currentRowItem(ui->studentsTable, 0);
    QTableWidgetItem* melliItem = currentRowItem(ui->studentsTable, 1);
    QTableWidgetItem* entranceYearItem = currentRowItem(ui->studentsTable, 2);

    if (studentIdItem == nullptr) {
        return;
    }

    ui->studentIdEdit->setText(studentIdItem->text());
    ui->nationalIdEdit->setText(melliItem != nullptr ? melliItem->text() : "");

    if (entranceYearItem != nullptr) {
        ui->entranceYearSpinBox->setValue(entranceYearItem->text().toInt());
    }
}

/** Copies the selected teacher row into the editable form fields. */
void MainWindow::fillTeacherFormFromSelection()
{
    QTableWidgetItem* teacherIdItem = currentRowItem(ui->teachersTable, 0);
    QTableWidgetItem* melliItem = currentRowItem(ui->teachersTable, 1);
    QTableWidgetItem* hireDateItem = currentRowItem(ui->teachersTable, 2);

    if (teacherIdItem == nullptr) {
        return;
    }

    ui->teacherIdEdit->setText(teacherIdItem->text());
    ui->teacherMelliEdit->setText(melliItem != nullptr ? melliItem->text() : "");

    if (hireDateItem != nullptr) {
        ui->teacherHireDateEdit->setDate(parseJalaliDateOrToday(hireDateItem->text()));
    }
}

/** Copies the selected course row into the editable form fields. */
void MainWindow::fillCourseFormFromSelection()
{
    QTableWidgetItem* courseIdItem = currentRowItem(ui->coursesTable, 0);
    QTableWidgetItem* courseNameItem = currentRowItem(ui->coursesTable, 1);
    QTableWidgetItem* creditsItem = currentRowItem(ui->coursesTable, 2);

    if (courseIdItem == nullptr) {
        return;
    }

    ui->courseIdEdit->setText(courseIdItem->text());
    ui->courseNameEdit->setText(courseNameItem != nullptr ? courseNameItem->text() : "");

    if (creditsItem != nullptr) {
        ui->courseCreditsSpinBox->setValue(creditsItem->text().toInt());
    }
}

/** Copies the selected term row into the editable form fields. */
void MainWindow::fillTermFormFromSelection()
{
    QTableWidgetItem* termIdItem = currentRowItem(ui->termsTable, 0);
    QTableWidgetItem* termNameItem = currentRowItem(ui->termsTable, 1);
    QTableWidgetItem* yearItem = currentRowItem(ui->termsTable, 2);

    if (termIdItem == nullptr) {
        return;
    }

    ui->termIdEdit->setText(termIdItem->text());
    ui->termNameEdit->setText(termNameItem != nullptr ? termNameItem->text() : "");

    if (yearItem != nullptr) {
        ui->termYearSpinBox->setValue(yearItem->text().toInt());
    }
}

/** Copies the selected term-course row into the editable form fields. */
void MainWindow::fillTermCourseFormFromSelection()
{
    QTableWidgetItem* termCourseIdItem = currentRowItem(ui->termCoursesTable, 0);
    QTableWidgetItem* termIdItem = currentRowItem(ui->termCoursesTable, 1);
    QTableWidgetItem* courseIdItem = currentRowItem(ui->termCoursesTable, 2);
    QTableWidgetItem* teacherIdItem = currentRowItem(ui->termCoursesTable, 3);

    if (termCourseIdItem == nullptr) {
        return;
    }

    ui->termCourseIdEdit->setText(termCourseIdItem->text());

    if (termIdItem != nullptr) {
        setComboBoxByData(ui->termCourseTermComboBox, termIdItem->text());
    }

    if (courseIdItem != nullptr) {
        setComboBoxByData(ui->termCourseCourseComboBox, courseIdItem->text());
    }

    if (teacherIdItem != nullptr) {
        setComboBoxByData(ui->termCourseTeacherComboBox, teacherIdItem->text());
    }
}

/** Copies the selected grade row into the editable form fields. */
void MainWindow::fillGradeFormFromSelection()
{
    QTableWidgetItem* gradeIdItem = currentRowItem(ui->gradesTable, 0);
    QTableWidgetItem* studentIdItem = currentRowItem(ui->gradesTable, 1);
    QTableWidgetItem* termCourseIdItem = currentRowItem(ui->gradesTable, 2);
    QTableWidgetItem* gradeValueItem = currentRowItem(ui->gradesTable, 3);

    if (gradeIdItem == nullptr) {
        return;
    }

    ui->gradeIdEdit->setText(gradeIdItem->text());

    if (studentIdItem != nullptr) {
        setComboBoxByData(ui->gradeStudentComboBox, studentIdItem->text());
    }

    if (termCourseIdItem != nullptr) {
        setComboBoxByData(ui->gradeTermCourseComboBox, termCourseIdItem->text());
    }

    if (gradeValueItem != nullptr) {
        ui->gradeValueDoubleSpinBox->setValue(gradeValueItem->text().toDouble());
    }
}

/** Replaces the selected person and detail records with the current form values. */
void MainWindow::updatePerson()
{
    QTableWidgetItem* selectedMelliItem = currentRowItem(ui->personsTable, 0);

    if (selectedMelliItem == nullptr) {
        QMessageBox::warning(this, "No Selection", "Please select a person row first.");
        return;
    }

    const QString selectedMelli = selectedMelliItem->text();
    const QString melli = ui->personMelliEdit->text().trimmed();
    const QString firstName = ui->personFirstNameEdit->text().trimmed();
    const QString family = ui->personFamilyEdit->text().trimmed();
    const QString address = ui->personAddressEdit->text().trimmed();
    const QString postalCode = ui->personPostalCodeEdit->text().trimmed();
    const QString telephone = ui->personTelephoneEdit->text().trimmed();

    if (melli != selectedMelli) {
        QMessageBox::warning(this, "Cannot Change ID", "Melli cannot be changed. Delete and add a new person if needed.");
        return;
    }

    if (firstName.isEmpty() || family.isEmpty() || address.isEmpty() || postalCode.isEmpty() || telephone.isEmpty()) {
        QMessageBox::warning(this, "Missing Data", "All person fields are required.");
        return;
    }

    if (containsPipe(firstName) || containsPipe(family) || containsPipe(address) || containsPipe(postalCode) || containsPipe(telephone)) {
        QMessageBox::warning(this, "Invalid Character", "The character | is not allowed because it is used by the database format.");
        return;
    }

    std::vector<NationalCore> cores = readNationalCore("database/national_core.txt");
    std::vector<NationalDetails> details = readNationalDetails("database/national_details.txt");

    for (NationalCore& core : cores) {
        if (core.melli == selectedMelli.toStdString()) {
            core.firstName = firstName.toStdString();
            core.family = family.toStdString();
            break;
        }
    }

    bool detailFound = false;

    for (NationalDetails& detail : details) {
        if (detail.melli == selectedMelli.toStdString()) {
            detail.address = address.toStdString();
            detail.postalCode = postalCode.toStdString();
            detail.telephone = telephone.toStdString();
            detailFound = true;
            break;
        }
    }

    if (!detailFound) {
        NationalDetails detail;
        detail.melli = selectedMelli.toStdString();
        detail.address = address.toStdString();
        detail.postalCode = postalCode.toStdString();
        detail.telephone = telephone.toStdString();
        details.push_back(detail);
    }

    writeNationalCore("database/national_core.txt", cores);
    writeNationalDetails("database/national_details.txt", details);

    loadPersons();
    loadTeachers();
    loadStudents();
    loadGrades();

    QMessageBox::information(this, "Updated", "Person updated successfully.");
}

/** Replaces the selected student record with the current form values. */
void MainWindow::updateStudent()
{
    QTableWidgetItem* selectedIdItem = currentRowItem(ui->studentsTable, 0);

    if (selectedIdItem == nullptr) {
        QMessageBox::warning(this, "No Selection", "Please select a student row first.");
        return;
    }

    const QString selectedStudentId = selectedIdItem->text();
    const QString studentId = ui->studentIdEdit->text().trimmed();
    const QString melli = ui->nationalIdEdit->text().trimmed();
    const int entranceYear = ui->entranceYearSpinBox->value();

    if (studentId != selectedStudentId) {
        QMessageBox::warning(this, "Cannot Change ID", "Student ID cannot be changed. Delete and add a new student if needed.");
        return;
    }

    if (melli.isEmpty()) {
        QMessageBox::warning(this, "Missing Data", "National ID is required.");
        return;
    }

    if (containsPipe(melli)) {
        QMessageBox::warning(this, "Invalid Character", "The character | is not allowed because it is used by the database format.");
        return;
    }

    if (!personExists(melli.toStdString())) {
        QMessageBox::warning(this, "Person Not Found", "This National ID does not exist. Please add the person first.");
        return;
    }

    std::vector<Student> students = readStudents("database/students.txt");

    const bool duplicateMelli = std::any_of(
        students.begin(),
        students.end(),
        [&melli, &selectedStudentId](const Student& student) {
            return student.studentId != selectedStudentId.toStdString() &&
                   student.melli == melli.toStdString();
        }
        );

    if (duplicateMelli) {
        QMessageBox::warning(this, "Duplicate National ID", "This National ID is already registered as another student.");
        return;
    }

    for (Student& student : students) {
        if (student.studentId == selectedStudentId.toStdString()) {
            student.melli = melli.toStdString();
            student.entranceYear = entranceYear;
            break;
        }
    }

    writeStudents("database/students.txt", students);
    recalculateStudentStatistics();

    loadStudents();
    loadGrades();

    QMessageBox::information(this, "Updated", "Student updated successfully.");
}

/** Replaces the selected teacher record with the current form values. */
void MainWindow::updateTeacher()
{
    QTableWidgetItem* selectedIdItem = currentRowItem(ui->teachersTable, 0);

    if (selectedIdItem == nullptr) {
        QMessageBox::warning(this, "No Selection", "Please select a teacher row first.");
        return;
    }

    const QString selectedTeacherId = selectedIdItem->text();
    const QString teacherId = ui->teacherIdEdit->text().trimmed();
    const QString melli = ui->teacherMelliEdit->text().trimmed();
    const QString hireDate = ui->teacherHireDateEdit->text();

    if (teacherId != selectedTeacherId) {
        QMessageBox::warning(this, "Cannot Change ID", "Teacher ID cannot be changed. Delete and add a new teacher if needed.");
        return;
    }

    if (melli.isEmpty() || hireDate.isEmpty()) {
        QMessageBox::warning(this, "Missing Data", "Melli and Hire Date are required.");
        return;
    }

    if (containsPipe(melli) || containsPipe(hireDate)) {
        QMessageBox::warning(this, "Invalid Character", "The character | is not allowed because it is used by the database format.");
        return;
    }

    if (!personExists(melli.toStdString())) {
        QMessageBox::warning(this, "Person Not Found", "This Melli does not exist. Please add the person first.");
        return;
    }

    std::vector<Teacher> teachers = readTeachers("database/teachers.txt");

    for (Teacher& teacher : teachers) {
        if (teacher.teacherId == selectedTeacherId.toStdString()) {
            teacher.melli = melli.toStdString();
            teacher.hireDate = hireDate.toStdString();
            break;
        }
    }

    writeTeachers("database/teachers.txt", teachers);

    loadTeachers();
    loadTermCourses();
    loadGrades();

    QMessageBox::information(this, "Updated", "Teacher updated successfully.");
}

/** Replaces the selected course record with the current form values. */
void MainWindow::updateCourse()
{
    QTableWidgetItem* selectedIdItem = currentRowItem(ui->coursesTable, 0);

    if (selectedIdItem == nullptr) {
        QMessageBox::warning(this, "No Selection", "Please select a course row first.");
        return;
    }

    const QString selectedCourseId = selectedIdItem->text();
    const QString courseId = ui->courseIdEdit->text().trimmed();
    const QString courseName = ui->courseNameEdit->text().trimmed();
    const int credits = ui->courseCreditsSpinBox->value();

    if (courseId != selectedCourseId) {
        QMessageBox::warning(this, "Cannot Change ID", "Course ID cannot be changed. Delete and add a new course if needed.");
        return;
    }

    if (courseName.isEmpty()) {
        QMessageBox::warning(this, "Missing Data", "Course Name is required.");
        return;
    }

    if (containsPipe(courseName)) {
        QMessageBox::warning(this, "Invalid Character", "The character | is not allowed because it is used by the database format.");
        return;
    }

    if (credits < 1 || credits > 4) {
        QMessageBox::warning(this, "Invalid Credits", "Course credits must be between 1 and 4.");
        return;
    }

    std::vector<Course> courses = readCourses("database/courses.txt");

    for (Course& course : courses) {
        if (course.courseId == selectedCourseId.toStdString()) {
            course.courseName = courseName.toStdString();
            course.credits = credits;
            break;
        }
    }

    writeCourses("database/courses.txt", courses);
    recalculateStudentStatistics();

    loadCourses();
    loadTermCourses();
    loadGrades();
    loadStudents();

    QMessageBox::information(this, "Updated", "Course updated successfully.");
}

/** Replaces the selected academic term record with the current form values. */
void MainWindow::updateTerm()
{
    QTableWidgetItem* selectedIdItem = currentRowItem(ui->termsTable, 0);

    if (selectedIdItem == nullptr) {
        QMessageBox::warning(this, "No Selection", "Please select a term row first.");
        return;
    }

    const QString selectedTermId = selectedIdItem->text();
    const QString termId = ui->termIdEdit->text().trimmed();
    const QString termName = ui->termNameEdit->text().trimmed();
    const int year = ui->termYearSpinBox->value();

    if (termId != selectedTermId) {
        QMessageBox::warning(this, "Cannot Change ID", "Term ID cannot be changed. Delete and add a new term if needed.");
        return;
    }

    if (termName.isEmpty()) {
        QMessageBox::warning(this, "Missing Data", "Term Name is required.");
        return;
    }

    if (containsPipe(termName)) {
        QMessageBox::warning(this, "Invalid Character", "The character | is not allowed because it is used by the database format.");
        return;
    }

    if (year < 1370 || year > 1430) {
        QMessageBox::warning(this, "Invalid Year", "Term year must be between 1370 and 1430.");
        return;
    }

    std::vector<Term> terms = readTerms("database/terms.txt");

    for (Term& term : terms) {
        if (term.termId == selectedTermId.toStdString()) {
            term.termName = termName.toStdString();
            term.year = year;
            break;
        }
    }

    writeTerms("database/terms.txt", terms);

    loadTerms();
    loadTermCourses();
    loadGrades();

    QMessageBox::information(this, "Updated", "Term updated successfully.");
}

/** Replaces the selected term-course assignment with the current form values. */
void MainWindow::updateTermCourse()
{
    QTableWidgetItem* selectedIdItem = currentRowItem(ui->termCoursesTable, 0);

    if (selectedIdItem == nullptr) {
        QMessageBox::warning(this, "No Selection", "Please select a term course row first.");
        return;
    }

    const QString selectedTermCourseId = selectedIdItem->text();
    const QString termCourseId = ui->termCourseIdEdit->text().trimmed();

    if (termCourseId != selectedTermCourseId) {
        QMessageBox::warning(this, "Cannot Change ID", "Term Course ID cannot be changed. Delete and add a new term course if needed.");
        return;
    }

    if (!comboBoxHasValidSelection(ui->termCourseTermComboBox)) {
        QMessageBox::warning(this, "Invalid Term", "Please select a valid term from the list.");
        return;
    }

    if (!comboBoxHasValidSelection(ui->termCourseCourseComboBox)) {
        QMessageBox::warning(this, "Invalid Course", "Please select a valid course from the list.");
        return;
    }

    if (!comboBoxHasValidSelection(ui->termCourseTeacherComboBox)) {
        QMessageBox::warning(this, "Invalid Teacher", "Please select a valid teacher from the list.");
        return;
    }

    const QString termId = ui->termCourseTermComboBox->currentData().toString();
    const QString courseId = ui->termCourseCourseComboBox->currentData().toString();
    const QString teacherId = ui->termCourseTeacherComboBox->currentData().toString();

    std::vector<TermCourse> termCourses = readTermCourses("database/term_courses.txt");

    for (TermCourse& termCourse : termCourses) {
        if (termCourse.termCourseId == selectedTermCourseId.toStdString()) {
            termCourse.termId = termId.toStdString();
            termCourse.courseId = courseId.toStdString();
            termCourse.teacherId = teacherId.toStdString();
            break;
        }
    }

    writeTermCourses("database/term_courses.txt", termCourses);
    recalculateStudentStatistics();

    loadTermCourses();
    loadGrades();
    loadStudents();

    QMessageBox::information(this, "Updated", "Term course updated successfully.");
}

/** Replaces the selected grade, recalculates pass status, and refreshes student statistics. */
void MainWindow::updateGrade()
{
    QTableWidgetItem* selectedIdItem = currentRowItem(ui->gradesTable, 0);

    if (selectedIdItem == nullptr) {
        QMessageBox::warning(this, "No Selection", "Please select a grade row first.");
        return;
    }

    const QString selectedGradeId = selectedIdItem->text();
    const QString gradeId = ui->gradeIdEdit->text().trimmed();
    const double gradeValue = ui->gradeValueDoubleSpinBox->value();

    if (gradeId != selectedGradeId) {
        QMessageBox::warning(this, "Cannot Change ID", "Grade ID cannot be changed. Delete and add a new grade if needed.");
        return;
    }

    if (!comboBoxHasValidSelection(ui->gradeStudentComboBox)) {
        QMessageBox::warning(this, "Invalid Student", "Please select a valid student from the list.");
        return;
    }

    if (!comboBoxHasValidSelection(ui->gradeTermCourseComboBox)) {
        QMessageBox::warning(this, "Invalid Term Course", "Please select a valid term course from the list.");
        return;
    }

    const QString studentId = ui->gradeStudentComboBox->currentData().toString();
    const QString termCourseId = ui->gradeTermCourseComboBox->currentData().toString();

    std::vector<Grade> grades = readGrades("database/grades.txt");

    const bool duplicateStudentTermCourse = std::any_of(
        grades.begin(),
        grades.end(),
        [&selectedGradeId, &studentId, &termCourseId](const Grade& grade) {
            return grade.gradeId != selectedGradeId.toStdString() &&
                   grade.studentId == studentId.toStdString() &&
                   grade.termCourseId == termCourseId.toStdString();
        }
        );

    if (duplicateStudentTermCourse) {
        QMessageBox::warning(this, "Duplicate Grade", "This student already has a grade for this term course.");
        return;
    }

    std::vector<TermCourse> termCourses = readTermCourses("database/term_courses.txt");
    std::vector<Course> courses = readCourses("database/courses.txt");

    const int credits = findCourseCreditsByTermCourseId(
        termCourseId.toStdString(),
        termCourses,
        courses
        );

    for (Grade& grade : grades) {
        if (grade.gradeId == selectedGradeId.toStdString()) {
            grade.studentId = studentId.toStdString();
            grade.termCourseId = termCourseId.toStdString();
            grade.gradeValue = gradeValue;
            grade.passed = gradeValue >= 10.0 ? credits : 0;
            break;
        }
    }

    writeGrades("database/grades.txt", grades);
    recalculateStudentStatistics();

    loadGrades();
    loadStudents();

    QMessageBox::information(this, "Updated", "Grade updated successfully.");
}

/** Creates the sidebar navigation and wraps Database, Reports, and System pages in one shell. */
void MainWindow::setupMainShell()
{
    QWidget *central = ui->centralwidget;

    if (central->layout() != nullptr) {
        delete central->layout();
    }

    QHBoxLayout *rootLayout = new QHBoxLayout(central);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    QFrame *sidebarFrame = new QFrame(central);
    sidebarFrame->setMinimumWidth(140);
    sidebarFrame->setMaximumWidth(160);
    sidebarFrame->setObjectName("sidebarFrame");

    QVBoxLayout *sidebarLayout = new QVBoxLayout(sidebarFrame);
    sidebarLayout->setContentsMargins(12, 16, 12, 16);
    sidebarLayout->setSpacing(10);

    QLabel *appTitleLabel = new QLabel("Educational\nDatabase", sidebarFrame);
    appTitleLabel->setAlignment(Qt::AlignCenter);

    databaseNavButton = new QPushButton("Database", sidebarFrame);
    reportsNavButton = new QPushButton("Reports", sidebarFrame);
    systemNavButton = new QPushButton("Users", sidebarFrame);

    databaseNavButton->setCheckable(true);
    reportsNavButton->setCheckable(true);
    systemNavButton->setCheckable(true);

    QButtonGroup *navGroup = new QButtonGroup(this);
    navGroup->setExclusive(true);
    navGroup->addButton(databaseNavButton);
    navGroup->addButton(reportsNavButton);
    navGroup->addButton(systemNavButton);

    sidebarLayout->addWidget(appTitleLabel);
    sidebarLayout->addSpacing(20);
    sidebarLayout->addWidget(databaseNavButton);
    sidebarLayout->addWidget(reportsNavButton);
    sidebarLayout->addWidget(systemNavButton);

    sidebarLayout->addStretch();

    QPushButton *helpButton = new QPushButton("Help", sidebarFrame);
    helpButton->setObjectName("helpButton");
    sidebarLayout->addWidget(helpButton);

    mainStackedWidget = new QStackedWidget(central);

    databasePage = new QWidget(mainStackedWidget);
    reportsPage = new QWidget(mainStackedWidget);
    systemPage = new QWidget(mainStackedWidget);

    QVBoxLayout *databaseLayout = new QVBoxLayout(databasePage);
    databaseLayout->setContentsMargins(4, 4, 4, 4);

    ui->tabWidget->setParent(databasePage);
    databaseLayout->addWidget(ui->tabWidget);

    QVBoxLayout *reportsLayout = new QVBoxLayout(reportsPage);
    reportsLayout->setContentsMargins(12, 12, 12, 12);
    reportsLayout->setSpacing(0);

    reportWidget = new ReportDialog(reportsPage);
    reportWidget->setWindowFlags(Qt::Widget);
    reportWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    reportsLayout->addWidget(reportWidget);

    setupSystemPage();

    mainStackedWidget->addWidget(databasePage);
    mainStackedWidget->addWidget(reportsPage);
    mainStackedWidget->addWidget(systemPage);

    rootLayout->addWidget(sidebarFrame);
    rootLayout->addWidget(mainStackedWidget, 1);

    connect(
        databaseNavButton,
        &QPushButton::clicked,
        this,
        &MainWindow::showDatabasePage
        );

    connect(
        reportsNavButton,
        &QPushButton::clicked,
        this,
        &MainWindow::showReportsPage
        );

    connect(
        systemNavButton,
        &QPushButton::clicked,
        this,
        &MainWindow::showSystemPage
        );

    connect(helpButton, &QPushButton::clicked, this, [this, helpButton]() {
        QMenu helpMenu(this);

        QAction *userGuideAction = helpMenu.addAction("User Guide");
        QAction *aboutAction = helpMenu.addAction("About");

        QAction *selectedAction = helpMenu.exec(
            helpButton->mapToGlobal(QPoint(0, helpButton->height()))
            );

        if (selectedAction == userGuideAction) {
            QMessageBox::information(
                this,
                "User Guide",
                "Admin:\n"
                "- Can add, update, and delete database records.\n"
                "- Can manage users.\n\n"
                "User:\n"
                "- Can view and search database records.\n"
                "- Can view reports/results.\n"
                "- Can view and search users.\n\n"
                "First entrance:\n"
                "- Temporary login is admin / admin123.\n"
                "- After first login, create a real admin account."
                );
        } else if (selectedAction == aboutAction) {
            QMessageBox::information(
                this,
                "About",
                "Educational Database\n"
                "Version 1.0\n\n"
                "Qt Widgets desktop application."
                );
        }
    });

    central->setStyleSheet(
        /* Sidebar */
        "QFrame#sidebarFrame {"
        "    background-color: #f8fafc;"
        "    border-right: 1px solid #d1d5db;"
        "}"

        "QFrame#sidebarFrame QPushButton {"
        "    background-color: transparent;"
        "    color: #111827;"
        "    border: none;"
        "    border-radius: 8px;"
        "    padding: 10px 12px;"
        "    min-height: 30px;"
        "    text-align: left;"
        "}"

        "QFrame#sidebarFrame QPushButton:hover {"
        "    background-color: #e5e7eb;"
        "}"

        "QFrame#sidebarFrame QPushButton:checked {"
        "    background-color: #dbeafe;"
        "    color: #111827;"
        "    font-weight: bold;"
        "}"

        /* Default buttons */
        "QPushButton {"
        "    background-color: #ffffff;"
        "    color: #111827;"
        "    border: 1px solid #cbd5e1;"
        "    border-radius: 7px;"
        "    padding: 6px 14px;"
        "    min-height: 24px;"
        "}"

        "QPushButton:hover {"
        "    background-color: #f8fafc;"
        "    border-color: #94a3b8;"
        "}"

        "QPushButton:pressed {"
        "    background-color: #e2e8f0;"
        "}"

        /* Add buttons */
        "QPushButton[role=\"primary\"] {"
        "    background-color: #2563eb;"
        "    color: white;"
        "    border: 1px solid #1d4ed8;"
        "    font-weight: 600;"
        "}"

        "QPushButton[role=\"primary\"]:hover {"
        "    background-color: #1d4ed8;"
        "}"

        "QPushButton[role=\"primary\"]:pressed {"
        "    background-color: #1e40af;"
        "}"

        /* Update buttons */
        "QPushButton[role=\"success\"] {"
        "    background-color: #16a34a;"
        "    color: white;"
        "    border: 1px solid #15803d;"
        "    font-weight: 600;"
        "}"

        "QPushButton[role=\"success\"]:hover {"
        "    background-color: #15803d;"
        "}"

        "QPushButton[role=\"success\"]:pressed {"
        "    background-color: #166534;"
        "}"

        /* Delete buttons */
        "QPushButton[role=\"danger\"] {"
        "    background-color: #dc2626;"
        "    color: white;"
        "    border: 1px solid #b91c1c;"
        "    font-weight: 600;"
        "}"

        "QPushButton[role=\"danger\"]:hover {"
        "    background-color: #b91c1c;"
        "}"

        "QPushButton[role=\"danger\"]:pressed {"
        "    background-color: #991b1b;"
        "}"

        /* Refresh buttons */
        "QPushButton[role=\"secondary\"] {"
        "    background-color: #f1f5f9;"
        "    color: #111827;"
        "    border: 1px solid #94a3b8;"
        "    font-weight: 500;"
        "}"

        "QPushButton[role=\"secondary\"]:hover {"
        "    background-color: #e2e8f0;"
        "}"

        "QPushButton[role=\"secondary\"]:pressed {"
        "    background-color: #cbd5e1;"
        "}"

        /* Inputs */
        "QLineEdit, QComboBox, QSpinBox, QDoubleSpinBox, QDateEdit {"
        "    background-color: #ffffff;"
        "    color: #111827;"
        "    border: 1px solid #d1d5db;"
        "    border-radius: 7px;"
        "    padding: 5px 8px;"
        "    min-height: 24px;"
        "}"

        "QLineEdit:focus, QComboBox:focus, QSpinBox:focus, QDoubleSpinBox:focus, QDateEdit:focus {"
        "    border: 1px solid #2563eb;"
        "}"

        /* Tables */
        "QTableWidget {"
        "    background-color: #ffffff;"
        "    border: 1px solid #e5e7eb;"
        "    gridline-color: #e5e7eb;"
        "    selection-background-color: #dbeafe;"
        "    selection-color: #111827;"
        "}"

        "QHeaderView::section {"
        "    background-color: #f8fafc;"
        "    color: #111827;"
        "    border: 1px solid #e5e7eb;"
        "    padding: 6px;"
        "    font-weight: bold;"
        "}"
        );
    databaseNavButton->setChecked(true);
    mainStackedWidget->setCurrentWidget(databasePage);
}

/** Activates the Database page and updates sidebar button state. */
void MainWindow::showDatabasePage()
{
    mainStackedWidget->setCurrentWidget(databasePage);
    databaseNavButton->setChecked(true);
}

/** Activates the Reports page and updates sidebar button state. */
void MainWindow::showReportsPage()
{
    mainStackedWidget->setCurrentWidget(reportsPage);
    reportsNavButton->setChecked(true);
}

/** Activates the System page and updates sidebar button state. */
void MainWindow::showSystemPage()
{
    mainStackedWidget->setCurrentWidget(systemPage);
    systemNavButton->setChecked(true);
}

/** Stores current session details and refreshes role-based UI permissions. */
void MainWindow::setCurrentUser(const QString& username, const QString& role)
{
    currentUsername = username;
    currentRole = role;

    if (currentUserLabel != nullptr) {
        currentUserLabel->setText(
            "Current user: " + currentUsername + " (" + currentRole + ")"
            );
    }

    const bool isAdmin = currentRole == "admin";

    if (systemUsernameLineEdit != nullptr) {
        systemUsernameLineEdit->setEnabled(isAdmin);
    }

    if (systemPasswordLineEdit != nullptr) {
        systemPasswordLineEdit->setEnabled(isAdmin);
    }

    if (systemRoleComboBox != nullptr) {
        systemRoleComboBox->setEnabled(isAdmin);
    }

    if (systemAddUserButton != nullptr) {
        systemAddUserButton->setEnabled(isAdmin);
    }

    if (systemDeleteUserButton != nullptr) {
        systemDeleteUserButton->setEnabled(isAdmin);
    }

    if (systemChangePasswordButton != nullptr) {
        systemChangePasswordButton->setEnabled(isAdmin);
    }

    loadSystemUsers();
    applyRolePermissions();
}

/** Builds the user-management page inside the main application shell. */
void MainWindow::setupSystemPage()
{
    if (systemPage->layout() != nullptr) {
        delete systemPage->layout();
    }

    QVBoxLayout *mainLayout = new QVBoxLayout(systemPage);
    mainLayout->setContentsMargins(18, 18, 18, 18);
    mainLayout->setSpacing(14);

    currentUserLabel = new QLabel("Current user: -", systemPage);
    currentUserLabel->setStyleSheet("font-weight: bold; font-size: 14px;");

    QGroupBox *usersGroup = new QGroupBox("Users Management", systemPage);

    QVBoxLayout *groupLayout = new QVBoxLayout(usersGroup);
    groupLayout->setSpacing(10);

    QGridLayout *formLayout = new QGridLayout();
    formLayout->setHorizontalSpacing(12);
    formLayout->setVerticalSpacing(10);

    systemUsernameLineEdit = new QLineEdit(usersGroup);
    systemPasswordLineEdit = new QLineEdit(usersGroup);
    systemRoleComboBox = new QComboBox(usersGroup);

    systemUsernameLineEdit->setPlaceholderText("Username");
    systemPasswordLineEdit->setPlaceholderText("Password / New Password");
    systemPasswordLineEdit->setEchoMode(QLineEdit::Password);

    systemRoleComboBox->addItem("admin");
    systemRoleComboBox->addItem("user");

    formLayout->addWidget(systemUsernameLineEdit, 0, 0);
    formLayout->addWidget(systemPasswordLineEdit, 0, 1);
    formLayout->addWidget(systemRoleComboBox, 0, 2);

    groupLayout->addLayout(formLayout);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(10);

    systemAddUserButton = new QPushButton("Add User", usersGroup);
    systemChangePasswordButton = new QPushButton("Change Password", usersGroup);
    systemDeleteUserButton = new QPushButton("Delete User", usersGroup);
    systemRefreshUsersButton = new QPushButton("Refresh", usersGroup);
    systemLogoutButton = new QPushButton("Logout", usersGroup);

    buttonLayout->addWidget(systemAddUserButton);
    buttonLayout->addWidget(systemChangePasswordButton);
    buttonLayout->addWidget(systemDeleteUserButton);
    buttonLayout->addWidget(systemRefreshUsersButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(systemLogoutButton);

    groupLayout->addLayout(buttonLayout);
    QHBoxLayout *searchLayout = new QHBoxLayout();

    systemUsersSearchLineEdit = new QLineEdit(usersGroup);
    systemUsersSearchLineEdit->setPlaceholderText("Search users...");
    systemUsersSearchLineEdit->setMinimumHeight(32);
    systemUsersSearchLineEdit->setMaximumWidth(380);

    searchLayout->addStretch();
    searchLayout->addWidget(systemUsersSearchLineEdit);
    searchLayout->addStretch();

    groupLayout->addLayout(searchLayout);

    systemUsersTable = new QTableWidget(usersGroup);
    systemUsersTable->setColumnCount(2);
    systemUsersTable->setHorizontalHeaderLabels({"Username", "Role"});
    systemUsersTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    systemUsersTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    systemUsersTable->setSelectionMode(QAbstractItemView::SingleSelection);
    systemUsersTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    groupLayout->addWidget(systemUsersTable, 1);

    mainLayout->addWidget(currentUserLabel);
    mainLayout->addWidget(usersGroup, 1);

    connect(systemAddUserButton, &QPushButton::clicked, this, &MainWindow::addSystemUser);
    connect(systemDeleteUserButton, &QPushButton::clicked, this, &MainWindow::deleteSystemUser);
    connect(systemChangePasswordButton, &QPushButton::clicked, this, &MainWindow::changeSystemUserPassword);
    connect(systemRefreshUsersButton, &QPushButton::clicked, this, &MainWindow::loadSystemUsers);
    connect(systemLogoutButton, &QPushButton::clicked, this, &MainWindow::logoutCurrentUser);
    connect(
        systemUsersSearchLineEdit,
        &QLineEdit::textChanged,
        this,
        &MainWindow::loadSystemUsers
        );

    connect(systemUsersTable, &QTableWidget::cellClicked, this, [this](int row, int) {
        if (row < 0) {
            return;
        }

        QTableWidgetItem *usernameItem = systemUsersTable->item(row, 0);
        QTableWidgetItem *roleItem = systemUsersTable->item(row, 1);

        if (usernameItem != nullptr) {
            systemUsernameLineEdit->setText(usernameItem->text());
        }

        if (roleItem != nullptr) {
            const int roleIndex = systemRoleComboBox->findText(roleItem->text());

            if (roleIndex >= 0) {
                systemRoleComboBox->setCurrentIndex(roleIndex);
            }
        }

        systemPasswordLineEdit->clear();
    });

    loadSystemUsers();
}

/** Loads saved users into the System page table. */
void MainWindow::loadSystemUsers()
{
    if (systemUsersTable == nullptr) {
        return;
    }

    const QVector<AuthUser> users = AuthManager::readUsers();

    QString searchText;

    if (systemUsersSearchLineEdit != nullptr) {
        searchText = systemUsersSearchLineEdit->text().trimmed().toLower();
    }

    systemUsersTable->setRowCount(0);

    for (const AuthUser& user : users) {
        const QString username = user.username;
        const QString role = user.role;

        if (!searchText.isEmpty()) {
            const bool matches =
                username.toLower().contains(searchText) ||
                role.toLower().contains(searchText);

            if (!matches) {
                continue;
            }
        }

        const int row = systemUsersTable->rowCount();
        systemUsersTable->insertRow(row);

        systemUsersTable->setItem(row, 0, new QTableWidgetItem(username));
        systemUsersTable->setItem(row, 1, new QTableWidgetItem(role));
    }
}
/** Adds a new application user from the System page form. */
void MainWindow::addSystemUser()
{
    if (currentRole != "admin") {
        QMessageBox::warning(this, "Permission Denied", "Only admin users can add users.");
        return;
    }

    QString message;

    const bool success = AuthManager::addUser(
        systemUsernameLineEdit->text(),
        systemPasswordLineEdit->text(),
        systemRoleComboBox->currentText(),
        &message
        );

    if (!success) {
        QMessageBox::warning(this, "Add User", message);
        return;
    }

    QMessageBox::information(this, "Add User", message);

    systemUsernameLineEdit->clear();
    systemPasswordLineEdit->clear();
    systemRoleComboBox->setCurrentText("user");

    loadSystemUsers();
}

/** Deletes the selected application user after confirmation. */
void MainWindow::deleteSystemUser()
{
    if (currentRole != "admin") {
        QMessageBox::warning(this, "Permission Denied", "Only admin users can delete users.");
        return;
    }

    const QString username = systemUsernameLineEdit->text().trimmed();

    if (username.isEmpty()) {
        QMessageBox::warning(this, "Delete User", "Select or enter a username first.");
        return;
    }

    if (username.compare(currentUsername, Qt::CaseInsensitive) == 0) {
        QMessageBox::warning(this, "Delete User", "You cannot delete the currently logged-in user.");
        return;
    }

    const QMessageBox::StandardButton answer = QMessageBox::question(
        this,
        "Delete User",
        "Delete user '" + username + "'?"
        );

    if (answer != QMessageBox::Yes) {
        return;
    }

    QString message;

    if (!AuthManager::deleteUser(username, &message)) {
        QMessageBox::warning(this, "Delete User", message);
        return;
    }

    QMessageBox::information(this, "Delete User", message);

    systemUsernameLineEdit->clear();
    systemPasswordLineEdit->clear();

    loadSystemUsers();
}

/** Changes the selected user's password using the System page form. */
void MainWindow::changeSystemUserPassword()
{
    if (currentRole != "admin") {
        QMessageBox::warning(this, "Permission Denied", "Only admin users can change passwords.");
        return;
    }

    const QString username = systemUsernameLineEdit->text().trimmed();
    const QString newPassword = systemPasswordLineEdit->text();

    if (username.isEmpty()) {
        QMessageBox::warning(this, "Change Password", "Select or enter a username first.");
        return;
    }

    QString message;

    if (!AuthManager::changePassword(username, newPassword, &message)) {
        QMessageBox::warning(this, "Change Password", message);
        return;
    }

    QMessageBox::information(this, "Change Password", message);

    systemPasswordLineEdit->clear();

    loadSystemUsers();
}

/** Closes the current main window and requests the login loop to restart. */
void MainWindow::logoutCurrentUser()
{
    const QMessageBox::StandardButton answer = QMessageBox::question(
        this,
        "Logout",
        "Do you want to logout?"
        );

    if (answer != QMessageBox::Yes) {
        return;
    }

    QCoreApplication::exit(LogoutExitCode);
}

/** Enables or disables user-management controls according to the current role. */
void MainWindow::applyRolePermissions()
{
    const bool isAdmin = currentRole == "admin";

    // =========================
    // Database mutation buttons
    // =========================

    ui->addPersonButton->setEnabled(isAdmin);
    ui->updatePersonButton->setEnabled(isAdmin);
    ui->deletePersonButton->setEnabled(isAdmin);

    ui->addStudentButton->setEnabled(isAdmin);
    ui->updateStudentButton->setEnabled(isAdmin);
    ui->deleteStudentButton->setEnabled(isAdmin);

    ui->addTeacherButton->setEnabled(isAdmin);
    ui->updateTeacherButton->setEnabled(isAdmin);
    ui->deleteTeacherButton->setEnabled(isAdmin);

    ui->addCourseButton->setEnabled(isAdmin);
    ui->updateCourseButton->setEnabled(isAdmin);
    ui->deleteCourseButton->setEnabled(isAdmin);

    ui->addTermButton->setEnabled(isAdmin);
    ui->updateTermButton->setEnabled(isAdmin);
    ui->deleteTermButton->setEnabled(isAdmin);

    ui->addTermCourseButton->setEnabled(isAdmin);
    ui->updateTermCourseButton->setEnabled(isAdmin);
    ui->deleteTermCourseButton->setEnabled(isAdmin);

    ui->addGradeButton->setEnabled(isAdmin);
    ui->updateGradeButton->setEnabled(isAdmin);
    ui->deleteGradeButton->setEnabled(isAdmin);

    // Refresh and search stay enabled for everyone.

    // =========================
    // Users page mutation controls
    // =========================

    if (systemUsernameLineEdit != nullptr) {
        systemUsernameLineEdit->setEnabled(isAdmin);
    }

    if (systemPasswordLineEdit != nullptr) {
        systemPasswordLineEdit->setEnabled(isAdmin);
    }

    if (systemRoleComboBox != nullptr) {
        systemRoleComboBox->setEnabled(isAdmin);
    }

    if (systemAddUserButton != nullptr) {
        systemAddUserButton->setEnabled(isAdmin);
    }

    if (systemDeleteUserButton != nullptr) {
        systemDeleteUserButton->setEnabled(isAdmin);
    }

    if (systemChangePasswordButton != nullptr) {
        systemChangePasswordButton->setEnabled(isAdmin);
    }

    // User search, refresh, table view, and logout stay enabled.
    if (systemUsersSearchLineEdit != nullptr) {
        systemUsersSearchLineEdit->setEnabled(true);
    }

    if (systemRefreshUsersButton != nullptr) {
        systemRefreshUsersButton->setEnabled(true);
    }

    if (systemLogoutButton != nullptr) {
        systemLogoutButton->setEnabled(true);
    }

    if (systemUsersTable != nullptr) {
        systemUsersTable->setEnabled(true);
    }
}