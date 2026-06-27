/**
 * @file reportdialog.cpp
 * @brief Implements HTML report generation for educational database records.
 */
#include "reportdialog.h"
#include "ui_reportdialog.h"

#include "backend/fileIO.h"
#include "backend/structures.h"

#include <QIcon>
#include <QPushButton>

#include <vector>

/** Escapes text before inserting it into generated HTML reports. */
static QString e(const QString& text)
{
    return text.toHtmlEscaped();
}

/** Resolves a national code to a display name for report rows. */
static QString personName(
    const std::string& melli,
    const std::vector<NationalCore>& people
    )
{
    for (const NationalCore& person : people) {
        if (person.melli == melli) {
            return QString::fromStdString(person.firstName + " " + person.family);
        }
    }

    return "-";
}

/** Resolves a course identifier to its display name for report rows. */
static QString courseName(
    const std::string& courseId,
    const std::vector<Course>& courses
    )
{
    for (const Course& course : courses) {
        if (course.courseId == courseId) {
            return QString::fromStdString(course.courseName);
        }
    }

    return "-";
}

/** Resolves a course identifier to its credit count. */
static int courseCredits(
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

/** Resolves a term identifier to a combined term/year label. */
static QString termName(
    const std::string& termId,
    const std::vector<Term>& terms
    )
{
    for (const Term& term : terms) {
        if (term.termId == termId) {
            return QString::fromStdString(term.termName) +
                   " (" + QString::number(term.year) + ")";
        }
    }

    return "-";
}

/** Resolves a teacher identifier to a report-friendly teacher label. */
static QString teacherName(
    const std::string& teacherId,
    const std::vector<Teacher>& teachers,
    const std::vector<NationalCore>& people
    )
{
    for (const Teacher& teacher : teachers) {
        if (teacher.teacherId == teacherId) {
            return QString::fromStdString(teacher.teacherId) +
                   " - " +
                   personName(teacher.melli, people);
        }
    }

    return QString::fromStdString(teacherId);
}

/** Resolves a student identifier to a report-friendly student label. */
static QString studentName(
    const std::string& studentId,
    const std::vector<Student>& students,
    const std::vector<NationalCore>& people
    )
{
    for (const Student& student : students) {
        if (student.studentId == studentId) {
            return QString::fromStdString(student.studentId) +
                   " - " +
                   personName(student.melli, people);
        }
    }

    return QString::fromStdString(studentId);
}

/** Resolves a term-course identifier to a full report-friendly label. */
static QString termCourseText(
    const std::string& termCourseId,
    const std::vector<TermCourse>& termCourses,
    const std::vector<Term>& terms,
    const std::vector<Course>& courses,
    const std::vector<Teacher>& teachers,
    const std::vector<NationalCore>& people
    )
{
    for (const TermCourse& termCourse : termCourses) {
        if (termCourse.termCourseId == termCourseId) {
            return QString::fromStdString(termCourse.termCourseId) +
                   " | " +
                   termName(termCourse.termId, terms) +
                   " | " +
                   courseName(termCourse.courseId, courses) +
                   " | " +
                   teacherName(termCourse.teacherId, teachers, people);
        }
    }

    return QString::fromStdString(termCourseId);
}

/** Initializes report controls and connects generate/clear actions. */
ReportDialog::ReportDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ReportDialog)
{
    ui->setupUi(this);
    setWindowIcon(QIcon(":/app_icon.ico"));

    setWindowTitle("Reports");
    setupReportTypes();

    connect(ui->generateReportButton, &QPushButton::clicked, this, &ReportDialog::generateReport);
    connect(ui->clearReportButton, &QPushButton::clicked, this, &ReportDialog::clearReport);
}

/** Releases the generated report dialog UI. */
ReportDialog::~ReportDialog()
{
    delete ui;
}

/** Registers all supported report types in the combo box. */
void ReportDialog::setupReportTypes()
{
    ui->reportTypeComboBox->clear();

    ui->reportTypeComboBox->addItem("All Students Report", "students");
    ui->reportTypeComboBox->addItem("All Teachers Report", "teachers");
    ui->reportTypeComboBox->addItem("All Courses Report", "courses");
    ui->reportTypeComboBox->addItem("All Terms Report", "terms");
    ui->reportTypeComboBox->addItem("All Term Courses Report", "term_courses");
    ui->reportTypeComboBox->addItem("All Grades Report", "grades");
    ui->reportTypeComboBox->addItem("Student Performance Report", "student_performance");
    ui->reportTypeComboBox->addItem("Failed Students Report", "failed_students");
    ui->reportTypeComboBox->addItem("Passed Students Report", "passed_students");
}

/** Generates shared opening HTML, CSS, title, and timestamp markup. */
QString ReportDialog::pageStart(const QString& title) const
{
    return
        "<html><head><style>"
        "body{font-family:Segoe UI,Arial,sans-serif;font-size:13px;color:#111827;}"
        "h2{color:#1f2937;margin-bottom:6px;}"
        ".meta{color:#6b7280;margin-bottom:16px;}"
        "table{border-collapse:collapse;width:100%;}"
        "th{background:#f3f4f6;font-weight:700;}"
        "td,th{border:1px solid #d1d5db;padding:7px 9px;text-align:center;}"
        "tr:nth-child(even){background:#f9fafb;}"
        ".good{color:#15803d;font-weight:700;}"
        ".bad{color:#b91c1c;font-weight:700;}"
        ".empty{color:#6b7280;font-style:italic;padding:16px;border:1px solid #e5e7eb;background:#f9fafb;}"
        "</style></head><body>"
        "<h2>" + e(title) + "</h2>"
                     "<div class='meta'>Generated from current database files.</div>";
}

/** Generates shared closing HTML markup. */
QString ReportDialog::pageEnd() const
{
    return "</body></html>";
}

/** Generates consistent empty-state markup for reports without rows. */
QString ReportDialog::emptyMessage(const QString& message) const
{
    return "<div class='empty'>" + e(message) + "</div>";
}

/** Dispatches report generation based on the selected report type. */
void ReportDialog::generateReport()
{
    const QString key = ui->reportTypeComboBox->currentData().toString();
    QString output;

    if (key == "students") {
        output = allStudentsReport();
    } else if (key == "teachers") {
        output = allTeachersReport();
    } else if (key == "courses") {
        output = allCoursesReport();
    } else if (key == "terms") {
        output = allTermsReport();
    } else if (key == "term_courses") {
        output = allTermCoursesReport();
    } else if (key == "grades") {
        output = allGradesReport();
    } else if (key == "student_performance") {
        output = studentPerformanceReport();
    } else if (key == "failed_students") {
        output = failedStudentsReport();
    } else if (key == "passed_students") {
        output = passedStudentsReport();
    } else {
        output = pageStart("Report") + emptyMessage("Please select a valid report type.") + pageEnd();
    }

    ui->reportTextBrowser->setHtml(output);
}

/** Clears the rendered report output. */
void ReportDialog::clearReport()
{
    ui->reportTextBrowser->clear();
}

/** Builds a table containing all students and their calculated academic statistics. */
QString ReportDialog::allStudentsReport() const
{
    std::vector<Student> students = readStudents("database/students.txt");
    std::vector<NationalCore> people = readNationalCore("database/national_core.txt");

    QString out = pageStart("All Students Report");

    if (students.empty()) {
        return out + emptyMessage("No students found.") + pageEnd();
    }

    out += "<table><tr><th>Student ID</th><th>Melli</th><th>Full Name</th><th>Entrance Year</th><th>Passed Credits</th><th>Average</th></tr>";

    for (const Student& student : students) {
        out += "<tr>"
               "<td>" + e(QString::fromStdString(student.studentId)) + "</td>"
                                                                "<td>" + e(QString::fromStdString(student.melli)) + "</td>"
                                                            "<td>" + e(personName(student.melli, people)) + "</td>"
                                                        "<td>" + QString::number(student.entranceYear) + "</td>"
                                                         "<td>" + QString::number(student.passed) + "</td>"
                                                   "<td>" + QString::number(student.grade, 'f', 2) + "</td>"
                                                          "</tr>";
    }

    out += "</table>";
    return out + pageEnd();
}

/** Builds a table containing all teachers and their identity information. */
QString ReportDialog::allTeachersReport() const
{
    std::vector<Teacher> teachers = readTeachers("database/teachers.txt");
    std::vector<NationalCore> people = readNationalCore("database/national_core.txt");

    QString out = pageStart("All Teachers Report");

    if (teachers.empty()) {
        return out + emptyMessage("No teachers found.") + pageEnd();
    }

    out += "<table><tr><th>Teacher ID</th><th>Melli</th><th>Full Name</th><th>Hire Date</th></tr>";

    for (const Teacher& teacher : teachers) {
        out += "<tr>"
               "<td>" + e(QString::fromStdString(teacher.teacherId)) + "</td>"
                                                                "<td>" + e(QString::fromStdString(teacher.melli)) + "</td>"
                                                            "<td>" + e(personName(teacher.melli, people)) + "</td>"
                                                        "<td>" + e(QString::fromStdString(teacher.hireDate)) + "</td>"
                                                               "</tr>";
    }

    out += "</table>";
    return out + pageEnd();
}

/** Builds a table containing all available courses. */
QString ReportDialog::allCoursesReport() const
{
    std::vector<Course> courses = readCourses("database/courses.txt");

    QString out = pageStart("All Courses Report");

    if (courses.empty()) {
        return out + emptyMessage("No courses found.") + pageEnd();
    }

    out += "<table><tr><th>Course ID</th><th>Course Name</th><th>Credits</th></tr>";

    for (const Course& course : courses) {
        out += "<tr>"
               "<td>" + e(QString::fromStdString(course.courseId)) + "</td>"
                                                              "<td>" + e(QString::fromStdString(course.courseName)) + "</td>"
                                                                "<td>" + QString::number(course.credits) + "</td>"
                                                   "</tr>";
    }

    out += "</table>";
    return out + pageEnd();
}

/** Builds a table containing all academic terms. */
QString ReportDialog::allTermsReport() const
{
    std::vector<Term> terms = readTerms("database/terms.txt");

    QString out = pageStart("All Terms Report");

    if (terms.empty()) {
        return out + emptyMessage("No terms found.") + pageEnd();
    }

    out += "<table><tr><th>Term ID</th><th>Term Name</th><th>Year</th></tr>";

    for (const Term& term : terms) {
        out += "<tr>"
               "<td>" + e(QString::fromStdString(term.termId)) + "</td>"
                                                          "<td>" + e(QString::fromStdString(term.termName)) + "</td>"
                                                            "<td>" + QString::number(term.year) + "</td>"
                                              "</tr>";
    }

    out += "</table>";
    return out + pageEnd();
}

/** Builds a table containing term-course assignments with resolved names. */
QString ReportDialog::allTermCoursesReport() const
{
    std::vector<TermCourse> termCourses = readTermCourses("database/term_courses.txt");
    std::vector<Term> terms = readTerms("database/terms.txt");
    std::vector<Course> courses = readCourses("database/courses.txt");
    std::vector<Teacher> teachers = readTeachers("database/teachers.txt");
    std::vector<NationalCore> people = readNationalCore("database/national_core.txt");

    QString out = pageStart("All Term Courses Report");

    if (termCourses.empty()) {
        return out + emptyMessage("No term courses found.") + pageEnd();
    }

    out += "<table><tr><th>Term Course ID</th><th>Term</th><th>Course</th><th>Credits</th><th>Teacher</th></tr>";

    for (const TermCourse& termCourse : termCourses) {
        out += "<tr>"
               "<td>" + e(QString::fromStdString(termCourse.termCourseId)) + "</td>"
                                                                      "<td>" + e(termName(termCourse.termId, terms)) + "</td>"
                                                         "<td>" + e(courseName(termCourse.courseId, courses)) + "</td>"
                                                               "<td>" + QString::number(courseCredits(termCourse.courseId, courses)) + "</td>"
                                                                                "<td>" + e(teacherName(termCourse.teacherId, teachers, people)) + "</td>"
                                                                          "</tr>";
    }

    out += "</table>";
    return out + pageEnd();
}

/** Builds a table containing grades with resolved student and course labels. */
QString ReportDialog::allGradesReport() const
{
    std::vector<Grade> grades = readGrades("database/grades.txt");
    std::vector<Student> students = readStudents("database/students.txt");
    std::vector<TermCourse> termCourses = readTermCourses("database/term_courses.txt");
    std::vector<Term> terms = readTerms("database/terms.txt");
    std::vector<Course> courses = readCourses("database/courses.txt");
    std::vector<Teacher> teachers = readTeachers("database/teachers.txt");
    std::vector<NationalCore> people = readNationalCore("database/national_core.txt");

    QString out = pageStart("All Grades Report");

    if (grades.empty()) {
        return out + emptyMessage("No grades found.") + pageEnd();
    }

    out += "<table><tr><th>Grade ID</th><th>Student</th><th>Term Course</th><th>Grade</th><th>Passed Credits</th><th>Status</th></tr>";

    for (const Grade& grade : grades) {
        const bool passed = grade.gradeValue >= 10.0;

        out += "<tr>"
               "<td>" + e(QString::fromStdString(grade.gradeId)) + "</td>"
                                                            "<td>" + e(studentName(grade.studentId, students, people)) + "</td>"
                                                                     "<td>" + e(termCourseText(grade.termCourseId, termCourses, terms, courses, teachers, people)) + "</td>"
                                                                                                        "<td>" + QString::number(grade.gradeValue, 'f', 2) + "</td>"
                                                             "<td>" + QString::number(grade.passed) + "</td>"
                                                 "<td class='" + QString(passed ? "good" : "bad") + "'>" + QString(passed ? "Passed" : "Failed") + "</td>"
                                                                                                   "</tr>";
    }

    out += "</table>";
    return out + pageEnd();
}

/** Builds a compact student-performance table for averages and passed credits. */
QString ReportDialog::studentPerformanceReport() const
{
    std::vector<Student> students = readStudents("database/students.txt");
    std::vector<NationalCore> people = readNationalCore("database/national_core.txt");

    QString out = pageStart("Student Performance Report");

    if (students.empty()) {
        return out + emptyMessage("No students found.") + pageEnd();
    }

    out += "<table><tr><th>Student ID</th><th>Full Name</th><th>Passed Credits</th><th>Average</th><th>Status</th></tr>";

    for (const Student& student : students) {
        const bool passed = student.grade >= 10.0;

        out += "<tr>"
               "<td>" + e(QString::fromStdString(student.studentId)) + "</td>"
                                                                "<td>" + e(personName(student.melli, people)) + "</td>"
                                                        "<td>" + QString::number(student.passed) + "</td>"
                                                   "<td>" + QString::number(student.grade, 'f', 2) + "</td>"
                                                          "<td class='" + QString(passed ? "good" : "bad") + "'>" + QString(passed ? "Passed" : "Failed") + "</td>"
                                                                                                   "</tr>";
    }

    out += "</table>";
    return out + pageEnd();
}

/** Builds a report containing students below the passing average threshold. */
QString ReportDialog::failedStudentsReport() const
{
    std::vector<Student> students = readStudents("database/students.txt");
    std::vector<NationalCore> people = readNationalCore("database/national_core.txt");

    QString rows;

    for (const Student& student : students) {
        if (student.grade >= 10.0) {
            continue;
        }

        rows += "<tr>"
                "<td>" + e(QString::fromStdString(student.studentId)) + "</td>"
                                                                 "<td>" + e(personName(student.melli, people)) + "</td>"
                                                         "<td>" + QString::number(student.passed) + "</td>"
                                                    "<td class='bad'>" + QString::number(student.grade, 'f', 2) + "</td>"
                                                           "</tr>";
    }

    QString out = pageStart("Failed Students Report");

    if (rows.isEmpty()) {
        return out + emptyMessage("No failed students found.") + pageEnd();
    }

    out += "<table><tr><th>Student ID</th><th>Full Name</th><th>Passed Credits</th><th>Average</th></tr>";
    out += rows;
    out += "</table>";

    return out + pageEnd();
}

/** Builds a report containing students at or above the passing average threshold. */
QString ReportDialog::passedStudentsReport() const
{
    std::vector<Student> students = readStudents("database/students.txt");
    std::vector<NationalCore> people = readNationalCore("database/national_core.txt");

    QString rows;

    for (const Student& student : students) {
        if (student.grade < 10.0) {
            continue;
        }

        rows += "<tr>"
                "<td>" + e(QString::fromStdString(student.studentId)) + "</td>"
                                                                 "<td>" + e(personName(student.melli, people)) + "</td>"
                                                         "<td>" + QString::number(student.passed) + "</td>"
                                                    "<td class='good'>" + QString::number(student.grade, 'f', 2) + "</td>"
                                                           "</tr>";
    }

    QString out = pageStart("Passed Students Report");

    if (rows.isEmpty()) {
        return out + emptyMessage("No passed students found.") + pageEnd();
    }

    out += "<table><tr><th>Student ID</th><th>Full Name</th><th>Passed Credits</th><th>Average</th></tr>";
    out += rows;
    out += "</table>";

    return out + pageEnd();
}