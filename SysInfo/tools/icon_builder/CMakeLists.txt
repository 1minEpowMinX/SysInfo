# Regenerates resources/icons/sysinfo_icon.{ico,icns} from sysinfo_app.svg.
#
# Built only when SYSINFO_BUILD_ICON_BUILDER is ON, since it is needed when the
# artwork changes rather than on every build, and its outputs are committed.

qt_add_executable(sysinfo_icon_builder
    main.cpp
)

target_link_libraries(sysinfo_icon_builder PRIVATE
    Qt6::Gui
    Qt6::Svg
)

# Lands beside the Qt runtime windeployqt puts in the build root, so the tool
# resolves its libraries and the SVG and ICNS image plugins.
set_target_properties(sysinfo_icon_builder PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

# Regenerates the icon containers in the source tree, in place.
add_custom_target(regenerate_icons
    COMMAND sysinfo_icon_builder
        "${CMAKE_SOURCE_DIR}/resources/icons/sysinfo_app.svg"
        "${CMAKE_SOURCE_DIR}/resources/icons"
    DEPENDS sysinfo_icon_builder
    COMMENT "Rebuilding .ico and .icns from the master SVG"
    VERBATIM
)
