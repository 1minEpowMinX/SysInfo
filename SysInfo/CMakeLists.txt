cmake_minimum_required(VERSION 3.30)

project(SysInfo VERSION 3.1.0 LANGUAGES CXX)

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

# Platform half of the hardware collectors: exactly one of these is compiled,
# which is what lets each of them be written without preprocessor branches.
# APPLE is tested before UNIX because macOS satisfies both.
if(WIN32)
    set(HARDWARE_INFO_PLATFORM_SOURCE core/sysinfo/hardware_info_win.cpp)
elseif(APPLE)
    set(HARDWARE_INFO_PLATFORM_SOURCE core/sysinfo/hardware_info_mac.cpp)
elseif(UNIX)
    set(HARDWARE_INFO_PLATFORM_SOURCE core/sysinfo/hardware_info_linux.cpp)
else()
    set(HARDWARE_INFO_PLATFORM_SOURCE core/sysinfo/hardware_info_stub.cpp)
endif()

# --- Core library: pure logic + non-UI services, reusable from exe and tests.
# Lives as a STATIC library so test binaries can link it directly instead
# of recompiling the same .cpp files into every test executable.
qt_add_library(sysinfo_core STATIC
    core/sysinfo/system_info.cpp
    core/sysinfo/system_info.h
    core/sysinfo/system_info_presenter.cpp
    core/sysinfo/system_info_presenter.h
    core/sysinfo/hardware_info.cpp
    core/sysinfo/hardware_info.h
    core/sysinfo/hardware_info_platform.h
    ${HARDWARE_INFO_PLATFORM_SOURCE}
    core/sysinfo/smbios_memory.cpp
    core/sysinfo/smbios_memory.h
    core/sysinfo/device_inventory.cpp
    core/sysinfo/device_inventory.h
    core/settings/settings_manager.cpp
    core/settings/settings_manager.h
    core/settings/onboarding_flags.h
    core/settings/extension_whitelist.h
    core/logging/logger.cpp
    core/logging/logger.h
    core/runtime/single_instance_guard.cpp
    core/runtime/single_instance_guard.h

    services/integration_server.cpp
    services/integration_server.h
)

# PUBLIC: headers of sysinfo_core expose Qt types — consumers must see them.
target_link_libraries(sysinfo_core PUBLIC
    Qt6::Core
    Qt6::Network
    Qt6::HttpServer
)

# PUBLIC include directory lets every consumer write
#   #include "core/sysinfo/system_info.h"
# without the "../" relative-path dance.
target_include_directories(sysinfo_core PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

# hardware_info_mac.cpp reads the drive's media type and model out of the IO
# registry; Windows and Linux need no extra libraries for it.
if(APPLE)
    find_library(IOKIT_LIBRARY IOKit REQUIRED)
    find_library(COREFOUNDATION_LIBRARY CoreFoundation REQUIRED)
    target_link_libraries(sysinfo_core PRIVATE
        ${IOKIT_LIBRARY}
        ${COREFOUNDATION_LIBRARY}
    )
endif()

# --- SysInfo executable: composition + UI on top of sysinfo_core.
set(PROJECT_SOURCES
    main.cpp

    app/app.cpp
    app/app.h
    app/notification_sink.h
    app/user_prompt.h
    app/welcome_notifier.cpp
    app/welcome_notifier.h

    ui/message_box_prompt.cpp
    ui/message_box_prompt.h
    ui/tray_controller.cpp
    ui/tray_controller.h
    ui/tray_guide.cpp
    ui/tray_guide.h
    ui/about_dialog.cpp
    ui/about_dialog.h

    resources.qrc
)

qt_add_executable(SysInfo
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
)

target_link_libraries(SysInfo PRIVATE
    sysinfo_core
    Qt6::Widgets
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
        "${CMAKE_SOURCE_DIR}/../docs/GPL-3.0.txt"
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
        DEPENDS tst_system_info tst_system_info_presenter tst_hardware_info
                tst_smbios_memory tst_device_inventory tst_integration_server
                tst_single_instance_guard tst_welcome_notifier
        COMMENT "Running tests"
        USES_TERMINAL
    )
endif()