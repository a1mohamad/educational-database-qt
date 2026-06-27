#ifndef REPORTDIALOG_H
#define REPORTDIALOG_H

#include <QDialog>
#include <QString>

namespace Ui {
class ReportDialog;
}

/**
 * @brief Generates HTML-based reports from the educational database files.
 *
 * ReportDialog is embedded in the main application shell and provides summary
 * reports for students, teachers, courses, terms, term courses, grades, and
 * pass/fail performance views.
 */
class ReportDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Creates the report widget and initializes report options.
     * @param parent Optional parent widget.
     */
    explicit ReportDialog(QWidget *parent = nullptr);

    /**
     * @brief Releases the generated Qt Designer UI object.
     */
    ~ReportDialog();

private:
    Ui::ReportDialog *ui; ///< Qt Designer generated report UI.

    /**
     * @brief Fills the report-type combo box with supported reports.
     */
    void setupReportTypes();

    /**
     * @brief Generates the selected report and displays it in the report view.
     */
    void generateReport();

    /**
     * @brief Clears the current report output area.
     */
    void clearReport();

    /**
     * @brief Builds the opening HTML and CSS used by all reports.
     */
    QString pageStart(const QString& title) const;

    /**
     * @brief Builds the closing HTML used by all reports.
     */
    QString pageEnd() const;

    /**
     * @brief Builds a consistent empty-state message for reports with no data.
     */
    QString emptyMessage(const QString& message) const;

    /** @brief Builds the complete students report. */
    QString allStudentsReport() const;

    /** @brief Builds the complete teachers report. */
    QString allTeachersReport() const;

    /** @brief Builds the complete courses report. */
    QString allCoursesReport() const;

    /** @brief Builds the complete academic terms report. */
    QString allTermsReport() const;

    /** @brief Builds the complete term-course assignments report. */
    QString allTermCoursesReport() const;

    /** @brief Builds the complete grades report. */
    QString allGradesReport() const;

    /** @brief Builds a student performance report with averages and passed credits. */
    QString studentPerformanceReport() const;

    /** @brief Builds a report for students whose average grade is below the passing threshold. */
    QString failedStudentsReport() const;

    /** @brief Builds a report for students whose average grade meets the passing threshold. */
    QString passedStudentsReport() const;
};

#endif