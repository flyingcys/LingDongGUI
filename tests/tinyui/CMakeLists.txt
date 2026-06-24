set(TINYUI_UNIT_TESTS
    unit/test_tinyui_core_helpers.c
    unit/test_tinyui_smoke.c
    unit/test_tinyui_native_bridge.c
    unit/test_tinyui_theme.c
    unit/test_tinyui_widgets.c
    unit/test_tinyui_button_events.c
    unit/test_tinyui_layout.c
    unit/test_tinyui_arc.c
    unit/test_tinyui_background.c
    unit/test_tinyui_gauge.c
    unit/test_tinyui_icon_slider.c
    unit/test_tinyui_radial_menu.c
    unit/test_tinyui_list.c
    unit/test_tinyui_progress_bar.c
    unit/test_tinyui_progress_wheel.c
    unit/test_tinyui_qrcode.c
    unit/test_tinyui_animation.c
    unit/test_tinyui_app_lifecycle.c
    unit/test_tinyui_app_timer.c
    unit/test_tinyui_runtime_model.c
    unit/test_tinyui_port_display.c
    unit/test_tinyui_port_input.c
    unit/test_tinyui_port_tick_os.c
    unit/test_tinyui_image.c
    unit/test_tinyui_label.c
    unit/test_tinyui_message_box.c
    unit/test_tinyui_app_window_switch.c
    unit/test_tinyui_date_time.c
    unit/test_tinyui_clock.c
    unit/test_tinyui_line_edit.c
    unit/test_tinyui_keyboard.c
    unit/test_tinyui_canvas.c
    unit/test_tinyui_checkbox.c
    unit/test_tinyui_event.c
    unit/test_tinyui_combo_box.c
    unit/test_tinyui_scroll_selecter.c
    unit/test_tinyui_slider.c
    unit/test_tinyui_switch.c
    unit/test_tinyui_table.c
    unit/test_tinyui_text.c
    unit/test_tinyui_window.c
    unit/test_tinyui_graph.c
    unit/test_tinyui_calendar.c
    unit/test_tinyui_lifecycle.c
)

function(ld_register_tinyui_unit_test test_src)
    get_filename_component(legacy_target "${test_src}" NAME_WE)
    string(REGEX REPLACE "^test_tinyui_" "test_tinyui_" tinyui_target "${legacy_target}")
    ld_add_c_unit_test(${tinyui_target}
        SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/${test_src}"
        SUPPORT_LIB tinyui_test_support
        MAIN_LIB tinyui_backend_ldgui
        LABELS "tinyui;unit"
    )
    set_tests_properties(${tinyui_target} PROPERTIES ENVIRONMENT "SDL_VIDEODRIVER=dummy")
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_definitions(${tinyui_target} PRIVATE _GNU_SOURCE)
    endif()
endfunction()

foreach(test_src IN LISTS TINYUI_UNIT_TESTS)
    ld_register_tinyui_unit_test("${test_src}")
endforeach()

ld_add_c_unit_test(test_tinyui_wrapper_struct_overhead
    SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/unit/test_tinyui_wrapper_struct_overhead.c"
    SUPPORT_LIB tinyui_test_support
    MAIN_LIB tinyui_backend_ldgui
    LABELS "tinyui;probe;memory"
)


ld_add_python_test(check_tinyui_public_api
    SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/contract/check_tinyui_public_api.py"
    LABELS "tinyui;contract"
)

ld_add_python_test(check_tinyui_demo_boundary
    SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/contract/check_tinyui_demo_boundary.py"
    LABELS "tinyui;contract"
)

ld_add_python_test(check_tinyui_transition_guards
    SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/contract/check_tinyui_transition_guards.py"
    LABELS "tinyui;contract;transition"
)

ld_add_python_test(check_tinyui_core_no_sdl
    SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/contract/check_tinyui_core_no_sdl.py"
    LABELS "tinyui;contract;port"
)

ld_add_python_test(check_tinyui_v21_transition_guards
    SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/contract/check_tinyui_v21_transition_guards.py"
    LABELS "tinyui;contract;tinyui_v21;transition"
)

ld_add_python_test(check_tinyui_widget_contract_matrix
    SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/contract/check_tinyui_widget_contract_matrix.py"
    LABELS "tinyui;contract"
)

ld_add_python_test(check_tinyui_release_capability_matrix
    SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/contract/check_tinyui_release_capability_matrix.py"
    LABELS "tinyui;contract;release"
)

ld_add_python_test(check_tinyui_binary_size
    SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/perf/check_tinyui_binary_size.py"
    LABELS "tinyui;perf;size"
)

ld_add_python_test(check_tinyui_perf
    SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/perf/check_tinyui_perf.py"
    LABELS "tinyui;perf;runtime"
)

ld_add_python_test(check_tinyui_object_overhead
    SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/perf/check_tinyui_object_overhead.py"
    LABELS "tinyui;perf;memory"
)

if(LD_BUILD_RUNTIME_TESTS)
    ld_add_python_test(check_tinyui_runtime
        SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/runtime/check_tinyui_runtime.py"
        LABELS "tinyui;runtime"
    )

    find_package(Python3 COMPONENTS Interpreter REQUIRED)
    add_test(
        NAME check_tinyui_visible_ui
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/runtime/check_tinyui_visible_ui.py" --all
    )
    set_tests_properties(check_tinyui_visible_ui PROPERTIES LABELS "tinyui;runtime;visible")

    ld_add_python_test(check_tinyui_backend_mapping
        SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/runtime/check_tinyui_backend_mapping.py"
        LABELS "tinyui;runtime;backend;mapping"
    )
endif()
