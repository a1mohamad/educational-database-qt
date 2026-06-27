/**
 * @file fileIO.cpp
 * @brief Implements pipe-separated text-file serialization and database helper functions.
 */
#include "fileIO.h"
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

/** Converts a NationalCore record into one pipe-separated text line. */
string serializeCore(const NationalCore& nc) {
    return nc.melli + "|" + nc.firstName + "|" + nc.family;
}

/** Parses a pipe-separated text line into a NationalCore record. */
NationalCore deserializeCore(const string& line) {
    stringstream ss(line);
    NationalCore nc;
    string token;
    getline(ss, token, '|'); nc.melli = token;
    getline(ss, token, '|'); nc.firstName = token;
    getline(ss, token, '|'); nc.family = token;
    return nc;
}

/** Writes all NationalCore records to the given file. */
void writeNationalCore(const string& filename, const vector<NationalCore>& data) {
    ofstream out(filename);
    if (!out) { cerr << "Error: cannot open " << filename << endl; return; }
    for (const auto& rec : data) out << serializeCore(rec) << "\n";
}

/** Reads all NationalCore records from the given file. */
vector<NationalCore> readNationalCore(const string& filename) {
    vector<NationalCore> result;
    ifstream in(filename);
    if (!in) return result;
    string line;
    while (getline(in, line)) if (!line.empty()) result.push_back(deserializeCore(line));
    return result;
}

/** Converts a NationalDetails record into one pipe-separated text line. */
string serializeDetails(const NationalDetails& nd) {
    return nd.melli + "|" + nd.address + "|" + nd.postalCode + "|" + nd.telephone;
}

/** Parses a pipe-separated text line into a NationalDetails record. */
NationalDetails deserializeDetails(const string& line) {
    stringstream ss(line);
    NationalDetails nd;
    string token;
    getline(ss, token, '|'); nd.melli = token;
    getline(ss, token, '|'); nd.address = token;
    getline(ss, token, '|'); nd.postalCode = token;
    getline(ss, token, '|'); nd.telephone = token;
    return nd;
}

/** Writes all NationalDetails records to the given file. */
void writeNationalDetails(const string& filename, const vector<NationalDetails>& data) {
    ofstream out(filename);
    if (!out) { cerr << "Error: cannot open " << filename << endl; return; }
    for (const auto& rec : data) out << serializeDetails(rec) << "\n";
}

/** Reads all NationalDetails records from the given file. */
vector<NationalDetails> readNationalDetails(const string& filename) {
    vector<NationalDetails> result;
    ifstream in(filename);
    if (!in) return result;
    string line;
    while (getline(in, line)) if (!line.empty()) result.push_back(deserializeDetails(line));
    return result;
}

/** Converts a Student record into one pipe-separated text line. */
string serializeStudent(const Student& s) {
    return s.studentId + "|" + s.melli + "|" + to_string(s.entranceYear) + "|" + to_string(s.passed) + "|" + to_string(s.grade);
}

/** Parses a pipe-separated text line into a Student record. */
Student deserializeStudent(const string& line) {
    stringstream ss(line);
    Student s;
    string token;
    getline(ss, token, '|'); s.studentId = token;
    getline(ss, token, '|'); s.melli = token;
    getline(ss, token, '|'); s.entranceYear = stoi(token);
    getline(ss, token, '|'); s.passed = stoi(token);
    getline(ss, token, '|'); s.grade = stod(token);
    return s;
}

/** Writes all Student records to the given file. */
void writeStudents(const string& filename, const vector<Student>& data) {
    ofstream out(filename);
    if (!out) { cerr << "Error: cannot open " << filename << endl; return; }
    for (const auto& rec : data) out << serializeStudent(rec) << "\n";
}

/** Reads all Student records from the given file. */
vector<Student> readStudents(const string& filename) {
    vector<Student> result;
    ifstream in(filename);
    if (!in) return result;
    string line;
    while (getline(in, line)) if (!line.empty()) result.push_back(deserializeStudent(line));
    return result;
}

/** Converts a Teacher record into one pipe-separated text line. */
string serializeTeacher(const Teacher& t) {
    return t.teacherId + "|" + t.melli + "|" + t.hireDate;
}

/** Parses a pipe-separated text line into a Teacher record. */
Teacher deserializeTeacher(const string& line) {
    stringstream ss(line);
    Teacher t;
    string token;
    getline(ss, token, '|'); t.teacherId = token;
    getline(ss, token, '|'); t.melli = token;
    getline(ss, token, '|'); t.hireDate = token;
    return t;
}

/** Writes all Teacher records to the given file. */
void writeTeachers(const string& filename, const vector<Teacher>& data) {
    ofstream out(filename);
    if (!out) { cerr << "Error: cannot open " << filename << endl; return; }
    for (const auto& rec : data) out << serializeTeacher(rec) << "\n";
}

/** Reads all Teacher records from the given file. */
vector<Teacher> readTeachers(const string& filename) {
    vector<Teacher> result;
    ifstream in(filename);
    if (!in) return result;
    string line;
    while (getline(in, line)) if (!line.empty()) result.push_back(deserializeTeacher(line));
    return result;
}

/** Converts a Course record into one pipe-separated text line. */
string serializeCourse(const Course& c) {
    return c.courseId + "|" + c.courseName + "|" + to_string(c.credits);
}

/** Parses a pipe-separated text line into a Course record. */
Course deserializeCourse(const string& line) {
    stringstream ss(line);
    Course c;
    string token;
    getline(ss, token, '|'); c.courseId = token;
    getline(ss, c.courseName, '|');
    getline(ss, token, '|'); c.credits = stoi(token);
    return c;
}

/** Writes all Course records to the given file. */
void writeCourses(const string& filename, const vector<Course>& data) {
    ofstream out(filename);
    if (!out) { cerr << "Error: cannot open " << filename << endl; return; }
    for (const auto& rec : data) out << serializeCourse(rec) << "\n";
}

/** Reads all Course records from the given file. */
vector<Course> readCourses(const string& filename) {
    vector<Course> result;
    ifstream in(filename);
    if (!in) return result;
    string line;
    while (getline(in, line)) if (!line.empty()) result.push_back(deserializeCourse(line));
    return result;
}

/** Converts a Term record into one pipe-separated text line. */
string serializeTerm(const Term& t) {
    return t.termId + "|" + t.termName + "|" + to_string(t.year);
}

/** Parses a pipe-separated text line into a Term record. */
Term deserializeTerm(const string& line) {
    stringstream ss(line);
    Term t;
    string token;
    getline(ss, token, '|'); t.termId = token;
    getline(ss, t.termName, '|');
    getline(ss, token, '|'); t.year = stoi(token);
    return t;
}

/** Writes all Term records to the given file. */
void writeTerms(const string& filename, const vector<Term>& data) {
    ofstream out(filename);
    if (!out) { cerr << "Error: cannot open " << filename << endl; return; }
    for (const auto& rec : data) out << serializeTerm(rec) << "\n";
}

/** Reads all Term records from the given file. */
vector<Term> readTerms(const string& filename) {
    vector<Term> result;
    ifstream in(filename);
    if (!in) return result;
    string line;
    while (getline(in, line)) if (!line.empty()) result.push_back(deserializeTerm(line));
    return result;
}

/** Converts a TermCourse record into one pipe-separated text line. */
string serializeTermCourse(const TermCourse& tc) {
    return tc.termCourseId + "|" + tc.termId + "|" + tc.courseId + "|" + tc.teacherId;
}

/** Parses a pipe-separated text line into a TermCourse record. */
TermCourse deserializeTermCourse(const string& line) {
    stringstream ss(line);
    TermCourse tc;
    string token;
    getline(ss, token, '|'); tc.termCourseId = token;
    getline(ss, token, '|'); tc.termId = token;
    getline(ss, token, '|'); tc.courseId = token;
    getline(ss, token, '|'); tc.teacherId = token;
    return tc;
}

/** Writes all TermCourse records to the given file. */
void writeTermCourses(const string& filename, const vector<TermCourse>& data) {
    ofstream out(filename);
    if (!out) { cerr << "Error: cannot open " << filename << endl; return; }
    for (const auto& rec : data) out << serializeTermCourse(rec) << "\n";
}

/** Reads all TermCourse records from the given file. */
vector<TermCourse> readTermCourses(const string& filename) {
    vector<TermCourse> result;
    ifstream in(filename);
    if (!in) return result;
    string line;
    while (getline(in, line)) if (!line.empty()) result.push_back(deserializeTermCourse(line));
    return result;
}

/** Converts a Grade record into one pipe-separated text line. */
string serializeGrade(const Grade& g) {
    return g.gradeId + "|" + g.termCourseId + "|" + g.studentId + "|" + to_string(g.gradeValue) + "|" + to_string(g.passed);
}

/** Parses a pipe-separated text line into a Grade record. */
Grade deserializeGrade(const string& line) {
    stringstream ss(line);
    Grade g;
    string token;
    getline(ss, token, '|'); g.gradeId = token;
    getline(ss, token, '|'); g.termCourseId = token;
    getline(ss, token, '|'); g.studentId = token;
    getline(ss, token, '|'); g.gradeValue = stod(token);
    getline(ss, token, '|'); g.passed = stoi(token);
    return g;
}

/** Writes all Grade records to the given file. */
void writeGrades(const string& filename, const vector<Grade>& data) {
    ofstream out(filename);
    if (!out) { cerr << "Error: cannot open " << filename << endl; return; }
    for (const auto& rec : data) out << serializeGrade(rec) << "\n";
}

/** Reads all Grade records from the given file. */
vector<Grade> readGrades(const string& filename) {
    vector<Grade> result;
    ifstream in(filename);
    if (!in) return result;
    string line;
    while (getline(in, line)) if (!line.empty()) result.push_back(deserializeGrade(line));
    return result;
}

/** Checks whether a person exists by national code. */
bool personExists(const string& melli) {
    auto cores = readNationalCore("database/national_core.txt");
    for (const auto& p : cores) if (p.melli == melli) return true;
    return false;
}

/** Checks whether a course exists by course ID. */
bool courseExists(const string& courseId) {
    auto courses = readCourses("database/courses.txt");
    for (const auto& c : courses) if (c.courseId == courseId) return true;
    return false;
}

/** Checks whether a term exists by term ID. */
bool termExists(const string& termId) {
    auto terms = readTerms("database/terms.txt");
    for (const auto& t : terms) if (t.termId == termId) return true;
    return false;
}

/** Checks whether a term-course assignment exists by ID. */
bool termCourseExists(const string& termCourseId) {
    auto tcs = readTermCourses("database/term_courses.txt");
    for (const auto& tc : tcs) if (tc.termCourseId == termCourseId) return true;
    return false;
}

/** Checks whether a student exists by student ID. */
bool studentExists(const string& studentId) {
    auto students = readStudents("database/students.txt");
    for (const auto& s : students) if (s.studentId == studentId) return true;
    return false;
}

/** Checks whether a teacher exists by teacher ID. */
bool teacherExists(const string& teacherId) {
    auto teachers = readTeachers("database/teachers.txt");
    for (const auto& t : teachers) if (t.teacherId == teacherId) return true;
    return false;
}

/** Computes a simple average grade for a student from all grade records. */
double computeStudentAverage(const string& studentId) {
    auto grades = readGrades("database/grades.txt");
    double sum = 0;
    int count = 0;
    for (const auto& g : grades) {
        if (g.studentId == studentId && g.passed == 1) {
            sum += g.gradeValue;
            count++;
        }
    }
    if (count == 0) return 0.0;
    return sum / count;
}

/** Updates one student record with recalculated average and passed-credit values. */
void updateStudentPassedAndAverage(const string& studentId) {
    auto students = readStudents("database/students.txt");
    auto grades = readGrades("database/grades.txt");
    auto tcs = readTermCourses("database/term_courses.txt");
    auto courses = readCourses("database/courses.txt");
    int totalPassedCredits = 0;
    double sumWeighted = 0;

    for (const auto& g : grades) {
        if (g.studentId != studentId) continue;

        int credits = 0;
        for (const auto& tc : tcs) {
            if (tc.termCourseId == g.termCourseId) {
                for (const auto& c : courses) {
                    if (c.courseId == tc.courseId) {
                        credits = c.credits;
                        break;
                    }
                }
                break;
            }
        }

        if (g.passed == 1) {
            totalPassedCredits += credits;
            sumWeighted += g.gradeValue * credits;
        }
    }

    double avg = (totalPassedCredits == 0) ? 0.0 : sumWeighted / totalPassedCredits;

    for (auto& s : students) {
        if (s.studentId == studentId) {
            s.passed = totalPassedCredits;
            s.grade = avg;
            break;
        }
    }
    writeStudents("database/students.txt", students);
}