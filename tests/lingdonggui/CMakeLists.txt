set(LDGUI_UNIT_TESTS
    unit/test_ldbase.c
    unit/test_ldlabel.c
    unit/test_ldprogressbar.c
    unit/test_layout_window.c
)

foreach(test_src IN LISTS LDGUI_UNIT_TESTS)
    get_filename_component(test_name "${test_src}" NAME_WE)
    ld_add_c_unit_test(${test_name}
        SOURCES
            "${CMAKE_CURRENT_SOURCE_DIR}/${test_src}"
            "${CMAKE_CURRENT_SOURCE_DIR}/../support/ldgui_test_support.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/../support/ldgui_vt_mutex_stubs.c"
        MAIN_LIB longdonggui_host
        LABELS "lingdonggui;unit"
    )
endforeach()
