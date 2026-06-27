#ifndef FILEIO_H
#define FILEIO_H

#include "structures.h"
#include <string>
#include <vector>

/**
 * @file fileIO.h
 * @brief Declares file-based persistence helpers for all database entities.
 *
 * Each entity is serialized as one pipe-separated text line. The matching
 * read/write functions operate on plain text files located in the runtime
 * database directory.
 */

// National identity core records.
std::string serializeCore(const NationalCore& nc);
NationalCore deserializeCore(const std::string& line);
void writeNationalCore(const std::string& filename, const std::vector<NationalCore>& data);
std::vector<NationalCore> readNationalCore(const std::string& filename);

// National identity detail records.
std::string serializeDetails(const NationalDetails& nd);
NationalDetails deserializeDetails(const std::string& line);
void writeNationalDetails(const std::string& filename, const std::vector<NationalDetails>& data);
std::vector<NationalDetails> readNationalDetails(const std::string& filename);

// Student records.
std::string serializeStudent(const Student& s);
Student deserializeStudent(const std::string& line);
void writeStudents(const std::string& filename, const std::vector<Student>& data);
std::vector<Student> readStudents(const std::string& filename);

// Teacher records.
std::string serializeTeacher(const Teacher& t);
Teacher deserializeTeacher(const std::string& line);
void writeTeachers(const std::string& filename, const std::vector<Teacher>& data);
std::vector<Teacher> readTeachers(const std::string& filename);

// Course records.
std::string serializeCourse(const Course& c);
Course deserializeCourse(const std::string& line);
void writeCourses(const std::string& filename, const std::vector<Course>& data);
std::vector<Course> readCourses(const std::string& filename);

// Academic term records.
std::string serializeTerm(const Term& t);
Term deserializeTerm(const std::string& line);
void writeTerms(const std::string& filename, const std::vector<Term>& data);
std::vector<Term> readTerms(const std::string& filename);

// Term-course assignment records.
std::string serializeTermCourse(const TermCourse& tc);
TermCourse deserializeTermCourse(const std::string& line);
void writeTermCourses(const std::string& filename, const std::vector<TermCourse>& data);
std::vector<TermCourse> readTermCourses(const std::string& filename);

// Grade records.
std::string serializeGrade(const Grade& g);
Grade deserializeGrade(const std::string& line);
void writeGrades(const std::string& filename, const std::vector<Grade>& data);
std::vector<Grade> readGrades(const std::string& filename);

// Lookup and aggregate helpers used by the GUI workflow.
bool personExists(const std::string& melli);
bool courseExists(const std::string& courseId);
bool termExists(const std::string& termId);
bool termCourseExists(const std::string& termCourseId);
bool studentExists(const std::string& studentId);
bool teacherExists(const std::string& teacherId);
double computeStudentAverage(const std::string& studentId);
void updateStudentPassedAndAverage(const std::string& studentId);

#endif