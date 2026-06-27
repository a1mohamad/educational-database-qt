#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <string>

/**
 * @brief Stores shared national identity information for a person.
 *
 * The melli field acts as the common link between personal information,
 * students, and teachers.
 */
struct NationalCore {
    std::string melli;      ///< National identification code.
    std::string firstName;  ///< Person's first name.
    std::string family;     ///< Person's family name.
};

/**
 * @brief Stores additional contact and address information for a person.
 */
struct NationalDetails {
    std::string melli;       ///< National identification code linked to NationalCore.
    std::string address;     ///< Person's address.
    std::string postalCode;  ///< Postal code.
    std::string telephone;   ///< Telephone number.
};

/**
 * @brief Represents a student record in the educational database.
 */
struct Student {
    std::string studentId; ///< Unique student identifier.
    std::string melli;     ///< National code linked to the person table.
    int entranceYear;      ///< Student entrance year.
    int passed;            ///< Total passed credits.
    double grade;          ///< Current calculated average grade.
};

/**
 * @brief Represents a teacher record in the educational database.
 */
struct Teacher {
    std::string teacherId; ///< Unique teacher identifier.
    std::string melli;     ///< National code linked to the person table.
    std::string hireDate;  ///< Teacher hire date.
};

/**
 * @brief Represents a course definition.
 */
struct Course {
    std::string courseId;    ///< Unique course identifier.
    std::string courseName;  ///< Human-readable course name.
    int credits;             ///< Number of course credits.
};

/**
 * @brief Represents an academic term.
 */
struct Term {
    std::string termId;    ///< Unique term identifier.
    std::string termName;  ///< Human-readable term name.
    int year;              ///< Academic year.
};

/**
 * @brief Connects a term, course, and teacher into a teachable class offering.
 */
struct TermCourse {
    std::string termCourseId; ///< Unique term-course identifier.
    std::string termId;       ///< Linked term identifier.
    std::string courseId;     ///< Linked course identifier.
    std::string teacherId;    ///< Linked teacher identifier.
};

/**
 * @brief Represents a student's grade for one term-course record.
 */
struct Grade {
    std::string gradeId;      ///< Unique grade record identifier.
    std::string termCourseId; ///< Linked term-course identifier.
    std::string studentId;    ///< Linked student identifier.
    double gradeValue;        ///< Numeric grade value.
    int passed;               ///< Passed credits for this grade record.
};

/**
 * @brief Legacy/simple login state representation.
 */
struct LoggedInUser {
    std::string username; ///< Logged-in username.
    bool isAdmin;         ///< Whether the user has administrator permissions.
};

/**
 * @brief Legacy/simple user-account representation.
 */
struct UserAccount {
    std::string username;     ///< Account username.
    std::string salt;         ///< Password salt field kept for compatibility.
    std::string passwordHash; ///< Stored password hash.
    std::string role;         ///< "admin" or "normal".
};

#endif