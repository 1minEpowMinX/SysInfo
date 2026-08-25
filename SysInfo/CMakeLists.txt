cmake_minimum_required(VERSION 3.30)

project(SysInfo VERSION 3.1.0 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build metadata the About window and the /version route report. Placed after
# CMAKE_CXX_STANDARD, which one of them reads.
add_compile_definitions(PROJECT_VERSION="${PROJECT_VERSION}")
add_compile_definitions(PROJECT_CXX_STANDARD="${CMAKE_CXX_STANDARD}")
string(TIMESTAMP BUILD_DATE "%Y%m%d" UTC)
add_compile_definitions(BUILD_DATE="${BUILD_DATE}")

# Year the platform resource containers stamp into their copyright strings.
string(TIMESTAMP BUILD_YEAR "%Y" UTC)
add_compile_definitions(BUILD_YEAR="${BUILD_YEAR}")

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Widgets
    Svg
    Network
    HttpServer
    LinguistTools
)

set(TS_FILES
    resources/i18n/sysinfo_ru_RU.ts
    resources/i18n/sysinfo_uk_UA.ts
)

# Hardware collectors divided by platform. APPLE is tested before UNIX because
# macOS satisfies both.
if(WIN32)
    set(HARDWARE_INFO_PLATFORM_SOURCE core/sysinfo/hardware_info_win.cpp)
elseif(APPLE)
    set(HARDWARE_INFO_PLATFORM_SOURCE core/sysinfo/hardware_info_mac.cpp)
elseif(UNIX)
    set(HARDWARE_INFO_PLATFORM_SOURCE core/sysinfo/hardware_info_linux.cpp)
else()
    set(HARDWARE_INFO_PLATFORM_SOURCE core/sysinfo/hardware_info_stub.cpp)
endif()

# --- Core library: the data, settings, logging and runtime layers, reusable
# from the executable and the tests. Lives as a STATIC library so test
# binaries can link it directly instead of recompiling the same .cpp files
# into every test executable.
qt_add_library(sysinfo_core STATIC
    # Ports whose implementations live in ui/. They sit below both app/ and
    # ui/ so that neither has to include the other; the ports of core/settings
    # stay beside the store that implements them.
    core/ports/notification_sink.h
    core/ports/tray_view.h
    core/ports/dialog_presenter.h
    core/ports/user_prompt.h

    core/sysinfo/about_facts.h
    core/sysinfo/system_info.cpp
    core/sysinfo/system_info.h
    core/sysinfo/info_source.cpp
    core/sysinfo/info_source.h
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
)

target_link_libraries(sysinfo_core PUBLIC
    Qt6::Core
    Qt6::Network
)

# PUBLIC include directory lets every consumer write
#   #include "core/sysinfo/system_info.h"
# without the "../" relative-paths.
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

# --- Service library: the local HTTP endpoint, on top of sysinfo_core.
# Separate from it so that a test of the data layer does not link an HTTP
# server it never speaks to.
qt_add_library(sysinfo_services STATIC
    services/integration_server.cpp
    services/integration_server.h
    services/request_policy.cpp
    services/request_policy.h
)

target_link_libraries(sysinfo_services PUBLIC
    sysinfo_core
    Qt6::HttpServer
)

# --- SysInfo executable: composition + UI on top of the two libraries.
set(PROJECT_SOURCES
    main.cpp

    app/app.cpp
    app/app.h
    app/welcome_notifier.cpp
    app/welcome_notifier.h

    ui/message_box_prompt.cpp
    ui/message_box_prompt.h
    ui/tray_controller.cpp
    ui/tray_controller.h
    ui/widget_dialogs.cpp
    ui/widget_dialogs.h
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
    sysinfo_services
    Qt6::Widgets
    Qt6::Svg
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
    configure_file(
        "${CMAKE_SOURCE_DIR}/resources/resources.rc.in"
        "${CMAKE_CURRENT_BINARY_DIR}/resources.rc"
        @ONLY
    )
    target_sources(SysInfo PRIVATE
        "${CMAKE_CURRENT_BINARY_DIR}/resources.rc"
    )
    set_source_files_properties(
        "${CMAKE_CURRENT_BINARY_DIR}/resources.rc"
        PROPERTIES LANGUAGE RC
    )
    set_target_properties(SysInfo PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

if(APPLE)
    # configure_file substitutes variables and never target properties, so the
    # bundle values are variables first and are read back into the properties
    # below. A plist named by MACOSX_BUNDLE_INFO_PLIST is copied verbatim — CMake
    # substitutes nothing into it — which is why they have to be resolved here.
    set(MACOSX_BUNDLE_GUI_IDENTIFIER      "com.pivdenny.SysInfo")
    set(MACOSX_BUNDLE_BUNDLE_VERSION       "${PROJECT_VERSION}")
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}")

    configure_file(
        "${CMAKE_SOURCE_DIR}/resources/Info.plist.in"
        "${CMAKE_BINARY_DIR}/Info.plist"
        @ONLY
    )
    target_sources(SysInfo PRIVATE
        "${CMAKE_SOURCE_DIR}/resources/icons/sysinfo_icon.icns"
    )
    set_source_files_properties(
        "${CMAKE_SOURCE_DIR}/resources/icons/sysinfo_icon.icns"
        PROPERTIES MACOSX_PACKAGE_LOCATION "Resources"
    )
    set_target_properties(SysInfo PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_BINARY_DIR}/Info.plist"
        MACOSX_BUNDLE_GUI_IDENTIFIER      "${MACOSX_BUNDLE_GUI_IDENTIFIER}"
        MACOSX_BUNDLE_BUNDLE_VERSION       "${MACOSX_BUNDLE_BUNDLE_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${MACOSX_BUNDLE_SHORT_VERSION_STRING}"
        MACOSX_BUNDLE_ICON_FILE "sysinfo_icon"
    )
endif()

if(UNIX AND NOT APPLE)
    configure_file(
        "${CMAKE_SOURCE_DIR}/resources/sysinfo.desktop"
        "${CMAKE_BINARY_DIR}/resources/sysinfo.desktop"
        COPYONLY
    )
    configure_file(
        "${CMAKE_SOURCE_DIR}/resources/icons/sysinfo_app.svg"
        "${CMAKE_BINARY_DIR}/resources/icons/sysinfo_app.svg"
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

# Desktop integration for freedesktop systems. sysinfo.desktop carries
# "Icon=sysinfo", which is an icon-theme name rather than a path: the icon only
# resolves once it is installed into the theme under exactly that basename —
# hence the RENAME on the scalable entry.
#
# Placed after GNUInstallDirs, which defines CMAKE_INSTALL_DATAROOTDIR.
if(UNIX AND NOT APPLE)
    install(FILES "${CMAKE_SOURCE_DIR}/resources/sysinfo.desktop"
        DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/applications"
    )

    install(DIRECTORY "${CMAKE_SOURCE_DIR}/resources/icons/hicolor"
        DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons"
    )

    install(FILES "${CMAKE_SOURCE_DIR}/resources/icons/sysinfo_app.svg"
        DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/scalable/apps"
        RENAME "sysinfo.svg"
    )
endif()

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

# README.txt names the Qt release the binaries were taken from in four places,
# one of them the major.minor the LGPL replacement clause permits. Configured
# rather than copied, so the version comes from the Qt package this build found
# and cannot fall behind it.
configure_file(
    "${CMAKE_SOURCE_DIR}/../docs/README.txt.in"
    "${CMAKE_CURRENT_BINARY_DIR}/README.txt"
    @ONLY
)

set(LICENSE_NOTICES
    "${CMAKE_SOURCE_DIR}/../docs/GPL-3.0.txt"
    "${CMAKE_SOURCE_DIR}/../docs/LGPL-3.0.txt"
    "${CMAKE_CURRENT_BINARY_DIR}/README.txt"
)

# windeployqt leaves libc++.dll and libunwind.dll in the tree for a Windows
# build made with llvm-mingw, and Apache-2.0 section 4(a) requires their license
# to travel with them. No other toolchain ships those files.
if(WIN32 AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    list(APPEND LICENSE_NOTICES "${CMAKE_SOURCE_DIR}/../docs/LLVM-Apache-2.0.txt")
endif()

add_custom_command(TARGET SysInfo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${LICENSE_NOTICES}
        "$<TARGET_FILE_DIR:SysInfo>"
    COMMENT "Copying the license notices next to the executable"
    VERBATIM
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(SysInfo)
endif()

# The .ico and .icns are derived from resources/icons/sysinfo_app.svg and
# committed, because neither format has a vector form and the platforms demand
# them. Turn this on to rebuild them after the artwork changes:
#   cmake -DSYSINFO_BUILD_ICON_BUILDER=ON . && cmake --build . --target regenerate_icons
option(SYSINFO_BUILD_ICON_BUILDER
    "Build the tool that regenerates the platform icon containers" OFF)
if(SYSINFO_BUILD_ICON_BUILDER)
    add_subdirectory(tools/icon_builder)
endif()

include(CTest)
if(BUILD_TESTING)
    add_subdirectory(tests)

    # Run the test suite as part of the default build. The roster comes from
    # tests/CMakeLists.txt.
    add_custom_target(run_tests ALL
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -C $<CONFIG>
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Running tests"
        USES_TERMINAL
    )
    add_dependencies(run_tests ${SYSINFO_TEST_TARGETS})
endif()
