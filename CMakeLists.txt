cmake_minimum_required(VERSION 3.16)

project(Debug3000 VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets)
find_package(Qt6 REQUIRED COMPONENTS WebEngineWidgets)
find_package(Qt6 REQUIRED COMPONENTS LinguistTools)

set(TS_FILES
    translations/DebugCrafter_en.ts
    translations/DebugCrafter_ru.ts
)


set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    codeeditor.cpp
    codeeditor.h
    filecontroller.cpp
    filecontroller.h
    fileprocessor.cpp
    fileprocessor.h
    scriptrunner.cpp
    scriptrunner.h
    settingsmanager.cpp
    settingsmanager.h
    settingsdialog.cpp
    settingsdialog.h
)

qt6_add_translations(
    Debug3000
    TS_FILES ${TS_FILES}
    SOURCES ${PROJECT_SOURCES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(Debug3000
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        codeeditor.h codeeditor.cpp
        filecontroller.h filecontroller.cpp
        fileprocessor.h fileprocessor.cpp
        scriptrunner.h scriptrunner.cpp
        settingsmanager.h settingsmanager.cpp
        settingsdialog.h settingsdialog.cpp
        resources.qrc

    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET Debug3000 APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(Debug3000 SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(Debug3000
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(Debug3000 PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Gui)
target_link_libraries(Debug3000 PRIVATE Qt6::WebEngineWidgets)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.Debug3000)
endif()
set_target_properties(Debug3000 PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS Debug3000
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(Debug3000)
endif()
