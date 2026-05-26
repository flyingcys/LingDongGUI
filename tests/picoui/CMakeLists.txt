set(PICOUI_UNIT_TESTS
    unit/test_picoui_smoke.c
    unit/test_picoui_theme.c
    unit/test_picoui_widgets.c
    unit/test_picoui_layout.c
)

foreach(test_src IN LISTS PICOUI_UNIT_TESTS)
    get_filename_component(test_name "${test_src}" NAME_WE)
    ld_add_c_unit_test(${test_name}
        SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/${test_src}"
        SUPPORT_LIB picoui_test_support
        MAIN_LIB picoui_backend_ldgui
        LABELS "picoui;unit"
    )
endforeach()

ld_add_python_test(check_picoui_public_api
    SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/contract/check_picoui_public_api.py"
    LABELS "picoui;contract"
)

ld_add_python_test(check_picoui_demo_boundary
    SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/contract/check_picoui_demo_boundary.py"
    LABELS "picoui;contract"
)

if(LD_BUILD_RUNTIME_TESTS)
    ld_add_python_test(check_picoui_runtime
        SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/runtime/check_picoui_runtime.py"
        LABELS "picoui;runtime"
    )
endif()
