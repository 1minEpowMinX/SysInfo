cmake_minimum_required(VERSION 3.16)

project(SysInfo VERSION 2.0.0 LANGUAGES CXX)

# Add a variable to the compiler for the build version
add_compile_definitions(PROJECT_VERSION="${PROJECT_VERSION}")
string(TIMESTAMP BUILD_DATETIME "%Y%m%d" UTC)
add_compile_definitions(BUILD_DATETIME="${BUILD_DATETIME}")

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt6 REQUIRED COMPONENTS Network Core)
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS HttpServer)

# Translations
set(TS_FILES
    resources/i18n/sysinfo_ru_RU.ts
    resources/i18n/sysinfo_uk_UA.ts
)

set(PROJECT_SOURCES
        main.cpp
        app/app.cpp
        app/app.h
        utils/utils.h utils/utils.cpp
        tray/tray_guide.h tray/tray_guide.cpp
        tray/about_dialog.h tray/about_dialog.cpp
        settings/settings_manager.h settings/settings_manager.cpp
        integration_server/integration_server.h integration_server/integration_server.cpp
        resources.qrc
        ../.gitignore
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

# Windows: resources.rc (ico in .rc file)
if(WIN32)
    target_sources(SysInfo PRIVATE
        "${CMAKE_SOURCE_DIR}/resources/resources.rc"
    )
    set_source_files_properties(
        "${CMAKE_SOURCE_DIR}/resources/resources.rc"
        PROPERTIES LANGUAGE RC
    )
    set_target_properties(SysInfo PROPERTIES
        WIN32_EXECUTABLE TRUE
        )
endif()

# macOS: Info.plist + icns
if(APPLE)
    target_sources(SysInfo PRIVATE
        "${CMAKE_SOURCE_DIR}/resources/icons/sysinfo_icon.icns"
    )
    set_source_files_properties("${CMAKE_SOURCE_DIR}/resources/icons/sysinfo_icon.icns"
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
        "${CMAKE_SOURCE_DIR}/resources/icons/sysinfo_icon.png"
        "${CMAKE_BINARY_DIR}/resources/icons/sysinfo_icon.png"
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
    Qt6::HttpServer
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

# Windows: add dependencies
if(WIN32)
    add_custom_command(TARGET SysInfo POST_BUILD
            COMMAND "${Qt6_DIR}/../../../bin/windeployqt.exe" $<TARGET_FILE:SysInfo>
            COMMENT "Running windeployqt to bundle Qt dependencies..."
        )
endif()

# macOS: add dependencies
if(APPLE)
    add_custom_command(TARGET SysInfo POST_BUILD
        COMMAND /usr/local/opt/qt/bin/macdeployqt $<TARGET_FILE:SysInfo>
        COMMENT "Running macdeployqt to bundle Qt dependencies..."
    )
endif()
