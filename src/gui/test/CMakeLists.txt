set(LD_GUI_TEST_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

function(ld_add_gui_host_test test_name)
    add_executable(${test_name}
        "${LD_GUI_TEST_DIR}/test_support.c"
        "${LD_GUI_TEST_DIR}/${test_name}.c"
    )
    target_include_directories(${test_name}
        PRIVATE
            "${LD_GUI_TEST_DIR}"
    )
    target_link_libraries(${test_name}
        PRIVATE
            longdonggui_host
    )
    ld_apply_common_target_config(${test_name})
    add_test(NAME ${test_name} COMMAND ${test_name})
endfunction()

ld_add_gui_host_test(test_layout_window)
ld_add_gui_host_test(test_ldbase)
ld_add_gui_host_test(test_ldlabel)
ld_add_gui_host_test(test_ldprogressbar)

add_custom_target(ldgui_host_tests
    DEPENDS
        test_layout_window
        test_ldbase
        test_ldlabel
        test_ldprogressbar
)
