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

qt_add_executable(tst_info_source tst_info_source.cpp)
set_target_properties(tst_info_source PROPERTIES WIN32_EXECUTABLE OFF)
target_link_libraries(tst_info_source PRIVATE
    sysinfo_core
    Qt6::Test
)
sysinfo_register_test(tst_info_source)

qt_add_executable(tst_smbios_memory tst_smbios_memory.cpp)
set_target_properties(tst_smbios_memory PROPERTIES WIN32_EXECUTABLE OFF)
target_link_libraries(tst_smbios_memory PRIVATE
    sysinfo_core
    Qt6::Test
)
sysinfo_register_test(tst_smbios_memory)

qt_add_executable(tst_device_inventory tst_device_inventory.cpp)
set_target_properties(tst_device_inventory PROPERTIES WIN32_EXECUTABLE OFF)
target_link_libraries(tst_device_inventory PRIVATE
    sysinfo_core
    Qt6::Test
)
sysinfo_register_test(tst_device_inventory)

qt_add_executable(tst_single_instance_guard tst_single_instance_guard.cpp)
set_target_properties(tst_single_instance_guard PROPERTIES WIN32_EXECUTABLE OFF)
target_link_libraries(tst_single_instance_guard PRIVATE
    sysinfo_core
    Qt6::Test
)
sysinfo_register_test(tst_single_instance_guard)

# Compiles the notifier straight in: it lives in the SysInfo executable rather
# than in sysinfo_core, and pulls in no Qt6::Widgets. Its port comes from
# sysinfo_core, which is where NotificationSink is moc'd.
qt_add_executable(tst_welcome_notifier
    tst_welcome_notifier.cpp
    ../app/welcome_notifier.cpp
    ../app/welcome_notifier.h
)
set_target_properties(tst_welcome_notifier PROPERTIES WIN32_EXECUTABLE OFF)
target_link_libraries(tst_welcome_notifier PRIVATE
    sysinfo_core
    Qt6::Test
)
sysinfo_register_test(tst_welcome_notifier)

qt_add_executable(tst_request_policy tst_request_policy.cpp)
set_target_properties(tst_request_policy PROPERTIES WIN32_EXECUTABLE OFF)
target_link_libraries(tst_request_policy PRIVATE
    sysinfo_services
    Qt6::Test
)
sysinfo_register_test(tst_request_policy)

qt_add_executable(tst_integration_server tst_integration_server.cpp)
set_target_properties(tst_integration_server PROPERTIES WIN32_EXECUTABLE OFF)
target_link_libraries(tst_integration_server PRIVATE
    sysinfo_services
    Qt6::Test
)
sysinfo_register_test(tst_integration_server)
