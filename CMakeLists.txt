cmake_minimum_required(VERSION 3.19)
project(educational-database LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

qt_add_executable(educational-database
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    backend/fileIO.cpp
    backend/fileIO.h
    backend/structures.h
    reportdialog.h reportdialog.cpp reportdialog.ui
    logindialog.h logindialog.cpp logindialog.ui
    firstadmindialog.h firstadmindialog.cpp firstadmindialog.ui
    authmanager.cpp authmanager.h
    resources.qrc
    app.rc

)

target_link_libraries(educational-database
    PRIVATE
        Qt::Core
        Qt::Widgets
)

install(TARGETS educational-database
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET educational-database
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
