cmake_minimum_required(VERSION 3.16)

project(SysInfo VERSION 2.0 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt6 REQUIRED COMPONENTS Network Core)

# Translations
set(TS_FILES
    resources/i18n/SysInfo_uk_UA.ts
    resources/i18n/SysInfo_en_US.ts
)

set(PROJECT_SOURCES
        main.cpp
        app/app.cpp
        app/app.h
        utils/utils.h utils/utils.cpp
        tray/trayguide.h tray/trayguide.cpp
        settings/settingsmanager.h settings/settingsmanager.cpp
        resources/icons/SysInfo_Icon.ico
        resources.qrc
        .gitignore
        ${TS_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(SysInfo
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET SysInfo APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(SysInfo SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(SysInfo
            ${PROJECT_SOURCES}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

# Windows: resources.rc + ico
if(WIN32)
    target_sources(SysInfo PRIVATE resources/resources.rc)
    set_source_files_properties(resources/resources.rc PROPERTIES LANGUAGE RC)
    set_target_properties(SysInfo PROPERTIES
        WIN32_EXECUTABLE TRUE
        )
endif()

# macOS: Info.plist + icns
if(APPLE)
    target_sources(SysInfo PRIVATE
        "${CMAKE_SOURCE_DIR}/resources/icons/SysInfo_Icon.icns"
    )
    set_source_files_properties("${CMAKE_SOURCE_DIR}/resources/icons/SysInfo_Icon.icns"
        PROPERTIES MACOSX_PACKAGE_LOCATION "Resources"
    )
    set_target_properties(SysInfo PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/resources/Info.plist"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.pivdenny.SysInfo"
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}"
        MACOSX_BUNDLE_ICON_FILE "SysInfo_Icon.icns"
)
endif()

# Linux: .desktop + png
if(UNIX AND NOT APPLE)
    configure_file(
        "${CMAKE_SOURCE_DIR}/resources/sysinfo.desktop"
        "${CMAKE_BINARY_DIR}/resources/sysinfo.desktop"
        COPYONLY
    )
    configure_file(
        "${CMAKE_SOURCE_DIR}/resources/icons/SysInfo_Icon.png"
        "${CMAKE_BINARY_DIR}/resources/icons/SysInfo_Icon.png"
        COPYONLY
    )
    set_target_properties(SysInfo PROPERTIES
        OUTPUT_NAME "SysInfo"
    )
endif()

target_link_libraries(SysInfo PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt6::Network
    Qt6::Core
)

include(GNUInstallDirs)
install(TARGETS SysInfo
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(SysInfo)
endif()
