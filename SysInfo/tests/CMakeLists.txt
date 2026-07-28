find_package(Qt6 REQUIRED COMPONENTS Test)

if(WIN32)
    get_target_property(_qt_core_location Qt6::Core LOCATION)
    get_filename_component(_qt_bin_dir "${_qt_core_location}" DIRECTORY)
endif()

function(sysinfo_register_test name)
    add_test(NAME ${name} COMMAND ${name})
    if(WIN32 AND _qt_bin_dir)
        set_tests_properties(${name} PROPERTIES
            ENVIRONMENT "PATH=${_qt_bin_dir};$ENV{PATH}"
        )
    endif()
endfunction()

qt_add_executable(tst_system_info tst_system_info.cpp)
set_target_properties(tst_system_info PROPERTIES WIN32_EXECUTABLE OFF)
target_link_libraries(tst_system_info PRIVATE
    sysinfo_core
    Qt6::Test
)
sysinfo_register_test(tst_system_info)

qt_add_executable(tst_system_info_presenter tst_system_info_presenter.cpp)
set_target_properties(tst_system_info_presenter PROPERTIES WIN32_EXECUTABLE OFF)
target_link_libraries(tst_system_info_presenter PRIVATE
    sysinfo_core
    Qt6::Test
)
sysinfo_register_test(tst_system_info_presenter)

qt_add_executable(tst_hardware_info tst_hardware_info.cpp)
set_target_properties(tst_hardware_info PROPERTIES WIN32_EXECUTABLE OFF)
target_link_libraries(tst_hardware_info PRIVATE
    sysinfo_core
    Qt6::Test
)
sysinfo_register_test(tst_hardware_info)

qt_add_executable(tst_device_inventory tst_device_inventory.cpp)
set_target_properties(tst_device_inventory PROPERTIES WIN32_EXECUTABLE OFF)
target_link_libraries(tst_device_inventory PRIVATE
    sysinfo_core
    Qt6::Test
)
sysinfo_register_test(tst_device_inventory)

qt_add_executable(tst_integration_server tst_integration_server.cpp)
set_target_properties(tst_integration_server PROPERTIES WIN32_EXECUTABLE OFF)
target_link_libraries(tst_integration_server PRIVATE
    sysinfo_core
    Qt6::Test
)
sysinfo_register_test(tst_integration_server)
