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

qt_add_executable(tst_utils
    tst_utils.cpp
    ../utils/utils.cpp
    ../utils/utils.h
)
set_target_properties(tst_utils PROPERTIES WIN32_EXECUTABLE OFF)
target_link_libraries(tst_utils PRIVATE
    Qt6::Core
    Qt6::Network
    Qt6::Test
)
sysinfo_register_test(tst_utils)

qt_add_executable(tst_integration_server
    tst_integration_server.cpp
    ../integration_server/integration_server.cpp
    ../integration_server/integration_server.h
    ../utils/utils.cpp
    ../utils/utils.h
)
set_target_properties(tst_integration_server PROPERTIES WIN32_EXECUTABLE OFF)
target_link_libraries(tst_integration_server PRIVATE
    Qt6::Core
    Qt6::Network
    Qt6::HttpServer
    Qt6::Test
)
sysinfo_register_test(tst_integration_server)
