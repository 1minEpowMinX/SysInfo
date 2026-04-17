cmake_minimum_required(VERSION 3.30)

project(SysInfo VERSION 2.1.0 LANGUAGES CXX)

add_compile_definitions(PROJECT_VERSION="${PROJECT_VERSION}")
string(TIMESTAMP BUILD_DATE "%Y%m%d" UTC)
add_compile_definitions(BUILD_DATE="${BUILD_DATE}")

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Widgets
    Network
    HttpServer
    LinguistTools
)

set(TS_FILES
    resources/i18n/sysinfo_ru_RU.ts
    resources/i18n/sysinfo_uk_UA.ts
)

set(PROJECT_SOURCES
    main.cpp
    app/app.cpp
    app/app.h
    utils/utils.cpp
    utils/utils.h
    tray/tray_guide.cpp
    tray/tray_guide.h
    tray/about_dialog.cpp
    tray/about_dialog.h
    settings/settings_manager.cpp
    settings/settings_manager.h
    integration_server/integration_server.cpp
    integration_server/integration_server.h
    logger/logger.cpp
    logger/logger.h
    resources.qrc
)

qt_add_executable(SysInfo
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
)

target_link_libraries(SysInfo PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    Qt6::HttpServer
)

# Modern Qt 6 translations
qt_add_translations(SysInfo
    TS_FILES ${TS_FILES}
    RESOURCE_PREFIX "/i18n"
)

if(ANDROID)
    set_property(TARGET SysInfo APPEND PROPERTY
        QT_ANDROID_PACKAGE_SOURCE_DIR
        "${CMAKE_CURRENT_SOURCE_DIR}/android"
    )
endif()

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

if(APPLE)
    target_sources(SysInfo PRIVATE
        "${CMAKE_SOURCE_DIR}/resources/icons/sysinfo_icon.icns"
    )
    set_source_files_properties(
        "${CMAKE_SOURCE_DIR}/resources/icons/sysinfo_icon.icns"
        PROPERTIES MACOSX_PACKAGE_LOCATION "Resources"
    )
    set_target_properties(SysInfo PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/resources/Info.plist"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.pivdenny.SysInfo"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}"
        MACOSX_BUNDLE_ICON_FILE "sysinfo_icon.icns"
    )
endif()

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

include(GNUInstallDirs)
install(TARGETS SysInfo
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Qt deploy script for Win/macOS/Linux (used by `cmake --install`)
qt_generate_deploy_app_script(
    TARGET SysInfo
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)

install(SCRIPT ${deploy_script})

# Copy Qt runtime next to the executable in the build tree so the built binary
# is directly runnable without install.
if(WIN32)
    add_custom_command(TARGET SysInfo POST_BUILD
        COMMAND Qt6::windeployqt
            --no-translations
            --no-compiler-runtime
            --no-system-d3d-compiler
            --no-opengl-sw
            "$<TARGET_FILE:SysInfo>"
        COMMENT "Deploying Qt runtime into build tree (windeployqt)"
        VERBATIM
    )
elseif(APPLE)
    add_custom_command(TARGET SysInfo POST_BUILD
        COMMAND Qt6::macdeployqt
            "$<TARGET_BUNDLE_DIR:SysInfo>"
        COMMENT "Deploying Qt runtime into build tree (macdeployqt)"
        VERBATIM
    )
endif()

add_custom_command(TARGET SysInfo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/../docs/LICENSE.txt"
        "${CMAKE_SOURCE_DIR}/../docs/LGPL-3.0.txt"
        "${CMAKE_SOURCE_DIR}/../docs/README.txt"
        "$<TARGET_FILE_DIR:SysInfo>"
    COMMENT "Copying LICENSE.txt, LGPL-3.0.txt and README.txt to build root"
    VERBATIM
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(SysInfo)
endif()

include(CTest)
if(BUILD_TESTING)
    add_subdirectory(tests)

    # Run the test suite as part of the default build.
    add_custom_target(run_tests ALL
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -C $<CONFIG>
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        DEPENDS tst_utils tst_integration_server
        COMMENT "Running tests"
        USES_TERMINAL
    )
endif()