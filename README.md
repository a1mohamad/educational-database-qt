# Educational Database

![Qt](https://img.shields.io/badge/Qt-6.5%2B-41CD52?logo=qt&logoColor=white)
![C++](https://img.shields.io/badge/C%2B%2B-Desktop%20Application-00599C?logo=cplusplus&logoColor=white)
![CMake](https://img.shields.io/badge/CMake-3.19%2B-064F8C?logo=cmake&logoColor=white)
![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Desktop-lightgrey)

**Educational Database** is a Qt Widgets desktop application for managing core educational records in a clean Windows-style interface. The application supports student, teacher, course, term, term-course, grade, report, and user-management workflows through a sidebar-based graphical interface.

The project is built with **C++**, **Qt 6 Widgets**, and **CMake**. It uses a lightweight file-based persistence layer, making it easy to run locally without installing a separate database server.

---

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Application Modules](#application-modules)
- [User Roles](#user-roles)
- [First Run Login Flow](#first-run-login-flow)
- [Data Storage](#data-storage)
- [Project Structure](#project-structure)
- [Build Requirements](#build-requirements)
- [Build from Source](#build-from-source)
- [Run the Application](#run-the-application)
- [Packaging for Release](#packaging-for-release)
- [Recommended GitHub Cleanup](#recommended-github-cleanup)
- [Developer Notes](#developer-notes)
- [Current Scope and Limitations](#current-scope-and-limitations)
- [License](#license)

---

## Overview

This project provides a desktop-based educational records system with a structured graphical user interface. It is designed for local usage and academic database-management scenarios where the user needs to manage people, students, teachers, courses, academic terms, offered courses, grades, and generated reports.

The application is organized around three main sidebar sections:

| Section | Purpose |
|---|---|
| **Database** | Main CRUD interface for educational records |
| **Reports** | Generates HTML-style reports from the current database files |
| **Users** | Manages login accounts, roles, passwords, and logout |

The main database interface contains separate tabs for each entity, making the app easy to navigate and maintain.

---

## Features

### Database Management

- Manage personal identity records.
- Manage students and connect each student to an existing person record.
- Manage teachers and connect each teacher to an existing person record.
- Manage courses with credit values.
- Manage academic terms.
- Manage term-course assignments by connecting terms, courses, and teachers.
- Manage grades by connecting students to term courses.
- Search records directly inside each table.
- Select rows to automatically fill update/delete forms.
- Prevent invalid deletion when a record is already used by another module.
- Automatically recalculate student passed credits and weighted average after grade/course changes.

### Reports

The reports section can generate the following report types:

- All Students Report
- All Teachers Report
- All Courses Report
- All Terms Report
- All Term Courses Report
- All Grades Report
- Student Performance Report
- Failed Students Report
- Passed Students Report

Reports are rendered inside the application using formatted HTML tables.

### Authentication and User Management

- Login dialog before entering the application.
- Temporary first-run admin login.
- First real admin account creation after initial login.
- Admin and normal user roles.
- User creation, password change, deletion, refresh, and search.
- Protection against deleting the last admin user.
- Protection against deleting the currently logged-in user.
- Passwords are stored as hashes instead of plain text.

### User Interface

- Qt Widgets desktop interface.
- Sidebar navigation for Database, Reports, and Users.
- Light professional theme with styled buttons, tables, inputs, and navigation items.
- Role-based access control in the interface.
- Jalali calendar support for teacher hire dates.

---

## Application Modules

| Module | Description |
|---|---|
| **Persons** | Stores national identity information, including Melli code, first name, family name, address, postal code, and telephone. |
| **Students** | Stores student ID, connected Melli code, entrance year, passed credits, and average grade. |
| **Teachers** | Stores teacher ID, connected Melli code, and hire date. |
| **Courses** | Stores course ID, course name, and course credits. |
| **Terms** | Stores term ID, term name, and academic year. |
| **Term Courses** | Connects a term, course, and teacher into an offered course. |
| **Grades** | Stores student grades for specific term courses and calculates passed credits. |
| **Reports** | Produces formatted reports from current data files. |
| **Users** | Handles login accounts and role-based permissions. |

---

## User Roles

| Role | Permissions |
|---|---|
| **admin** | Can add, update, delete, search, refresh, generate reports, and manage users. |
| **user** | Can view, search, refresh, and generate reports. Mutation actions are disabled. |

The application applies permissions after login. Admin-only controls are disabled for normal users.

---

## First Run Login Flow

When the application is launched for the first time and no real user account exists, it allows a temporary admin login:

```text
Username: admin
Password: admin123
```

After logging in with the temporary admin account, the application asks the user to create the first real admin account. The temporary account is not saved to `users.dat`.

After the first admin account is created, future logins must use real accounts stored by the application.

---

## Data Storage

The application uses text files as a lightweight local persistence layer. Runtime data is expected beside the executable, because the application sets its working directory to the executable directory at startup.

At runtime, the application uses this structure:

```text
<application-directory>/
├── educational-database.exe
├── database/
│   ├── students.txt
│   ├── teachers.txt
│   ├── courses.txt
│   ├── terms.txt
│   ├── term_courses.txt
│   ├── grades.txt
│   ├── national_core.txt
│   └── national_details.txt
└── system_files/
    └── users.dat
```

If the folders or files do not exist, the application creates them automatically.

### Database File Formats

Records are stored as pipe-separated text lines using the `|` character as the delimiter.

| File | Format |
|---|---|
| `national_core.txt` | `melli|firstName|family` |
| `national_details.txt` | `melli|address|postalCode|telephone` |
| `students.txt` | `studentId|melli|entranceYear|passed|grade` |
| `teachers.txt` | `teacherId|melli|hireDate` |
| `courses.txt` | `courseId|courseName|credits` |
| `terms.txt` | `termId|termName|year` |
| `term_courses.txt` | `termCourseId|termId|courseId|teacherId` |
| `grades.txt` | `gradeId|termCourseId|studentId|gradeValue|passed` |
| `users.dat` | `username|passwordHash|role` |

Because `|` is the storage delimiter, the application blocks this character in user input.

---

## Project Structure

```text
educational-database/
├── CMakeLists.txt
├── main.cpp
├── mainwindow.h
├── mainwindow.cpp
├── mainwindow.ui
├── reportdialog.h
├── reportdialog.cpp
├── reportdialog.ui
├── logindialog.h
├── logindialog.cpp
├── logindialog.ui
├── firstadmindialog.h
├── firstadmindialog.cpp
├── firstadmindialog.ui
├── authmanager.h
├── authmanager.cpp
├── resources.qrc
├── app.rc
├── app_icon.ico
└── backend/
    ├── structures.h
    ├── fileIO.h
    └── fileIO.cpp
```

### Important Runtime Note

The `.txt` database files and `users.dat` are runtime files. In a built Qt application, they should exist under:

```text
build-folder-or-release-folder/
├── database/
│   └── *.txt
└── system_files/
    └── users.dat
```

Do not rely on the source root as the runtime data location after deployment. The executable uses its own directory as the working directory.

---

## Build Requirements

To build the project from source, install:

- Qt 6.5 or newer
- CMake 3.19 or newer
- A C++ compiler supported by your Qt kit
- Qt Creator, Visual Studio, or another CMake-compatible IDE

This project uses Qt components:

```cmake
Qt6::Core
Qt6::Widgets
```

---

## Build from Source

Clone the repository:

```bash
git clone https://github.com/<your-username>/educational-database.git
cd educational-database
```

Configure the project:

```bash
cmake -S . -B build
```

Build the project:

```bash
cmake --build build --config Release
```

The executable location depends on the selected generator and Qt kit. Common output locations include:

```text
build/Release/educational-database.exe
build/educational-database.exe
```

You can also open the project directly in Qt Creator using `CMakeLists.txt`.

---

## Run the Application

After building, run the executable from the build or release directory.

On first launch, the application creates the required runtime folders and files if they do not already exist:

```text
database/
system_files/
```

Then login using the first-run temporary admin account:

```text
Username: admin
Password: admin123
```

After that, create the first real admin account and continue using the application normally.

---

## Packaging for Release

For a GitHub release, build the project in Release mode and package the final executable together with the required Qt runtime files and runtime data folders.

A recommended release layout is:

```text
EducationalDatabase-v1.0.0/
├── educational-database.exe
├── database/
│   ├── students.txt
│   ├── teachers.txt
│   ├── courses.txt
│   ├── terms.txt
│   ├── term_courses.txt
│   ├── grades.txt
│   ├── national_core.txt
│   └── national_details.txt
├── system_files/
│   └── users.dat
├── platforms/
│   └── qwindows.dll
└── Qt runtime DLLs...
```

The project already contains CMake deployment support through `qt_generate_deploy_app_script`. After building, you can install/deploy using CMake:

```bash
cmake --install build --config Release --prefix dist
```

Depending on the Qt kit and generator, the installed files may be placed directly in `dist/` or inside a subfolder such as `dist/bin/`.

Before publishing a public release, decide whether you want to include sample database files or ship an empty database. If you ship a clean release, let the application create empty files automatically on first launch.

### Important Security Note

Do not publish a real `system_files/users.dat` file from your own machine if it contains real usernames or passwords hashes. For a public GitHub release, either:

1. remove `users.dat` and let the app create a clean one, or
2. provide a clearly documented demo account only if that is intentional.

---

## Recommended GitHub Cleanup

Do not commit generated build files to the repository. A professional GitHub repository should keep source files and exclude build artifacts.

Recommended `.gitignore` entries:

```gitignore
# Build directories
build/
build-*/
cmake-build-*/

# CMake generated files
CMakeCache.txt
CMakeFiles/
cmake_install.cmake
Makefile
install_manifest.txt

# Qt Creator user settings
*.user
*.user.*

# Compiled binaries and objects
*.exe
*.dll
*.lib
*.obj
*.o
*.pdb
*.ilk

# Runtime application data
system_files/users.dat

# Optional: ignore local database files if they contain private data
# database/*.txt
```

If you want to include sample data, keep it clearly separated, for example:

```text
sample_data/
└── database/
    └── *.txt
```

Then explain in the README how users can copy sample data into the runtime `database/` folder.

---

## Developer Notes

### Main Classes and Files

| File | Responsibility |
|---|---|
| `main.cpp` | Application startup, runtime folder preparation, login flow, first-admin flow, and main window loop. |
| `mainwindow.*` | Main application UI, database tabs, sidebar navigation, CRUD operations, search, role permissions, and user page. |
| `reportdialog.*` | Report selection and HTML report generation. |
| `logindialog.*` | Login dialog and temporary first-run login handling. |
| `firstadmindialog.*` | First real admin account creation. |
| `authmanager.*` | User storage, password hashing, login validation, account creation, deletion, and password changes. |
| `backend/structures.h` | Core data structures used by the database layer. |
| `backend/fileIO.*` | Serialization, deserialization, text-file reading/writing, and helper lookup functions. |

### Data Relationships

The application uses simple relationships between records:

```text
Person
├── Student   uses Person.melli
└── Teacher   uses Person.melli

TermCourse
├── Term      uses termId
├── Course    uses courseId
└── Teacher   uses teacherId

Grade
├── Student       uses studentId
└── TermCourse    uses termCourseId
```

When grades are added, updated, or deleted, student statistics are recalculated. A grade value of `10.0` or higher is considered passed, and passed credits are calculated from the related course credits.

---

## Current Scope and Limitations

This project is designed as a local desktop educational database application. Its current architecture intentionally uses text files instead of a database server.

Current scope:

- Local desktop usage
- Single-machine data files
- Role-based local login
- Text-file persistence
- Academic/educational database workflows

Current limitations:

- No network synchronization
- No multi-user database server
- No automatic cloud backup
- No advanced audit logging
- File format depends on the `|` delimiter

For a larger production system, the next technical upgrade would be replacing the text-file persistence layer with SQLite or another relational database system.

---

## License

No license file is currently included in this project.

Before publishing the repository publicly, add a license such as MIT, Apache-2.0, GPL, or another license that matches your intended usage and distribution model.
