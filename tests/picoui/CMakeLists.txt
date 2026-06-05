set(PICOUI_UNIT_TESTS
    unit/test_picoui_smoke.c
    unit/test_picoui_native_bridge.c
    unit/test_picoui_theme.c
    unit/test_picoui_widgets.c
    unit/test_picoui_button_events.c
    unit/test_picoui_layout.c
    unit/test_picoui_arc.c
    unit/test_picoui_background.c
    unit/test_picoui_gauge.c
    unit/test_picoui_icon_slider.c
    unit/test_picoui_radial_menu.c
    unit/test_picoui_list.c
    unit/test_picoui_progress_bar.c
    unit/test_picoui_progress_wheel.c
    unit/test_picoui_qrcode.c
    unit/test_picoui_animation.c
    unit/test_picoui_app_lifecycle.c
    unit/test_picoui_app_timer.c
    unit/test_picoui_port_display.c
    unit/test_picoui_port_input.c
    unit/test_picoui_port_sdl.c
    unit/test_picoui_port_tick_os.c
    unit/test_picoui_image.c
    unit/test_picoui_label.c
    unit/test_picoui_message_box.c
    unit/test_picoui_app_window_switch.c
    unit/test_picoui_date_time.c
    unit/test_picoui_clock.c
    unit/test_picoui_line_edit.c
    unit/test_picoui_keyboard.c
    unit/test_picoui_canvas.c
    unit/test_picoui_checkbox.c
    unit/test_picoui_event.c
    unit/test_picoui_combo_box.c
    unit/test_picoui_scroll_selecter.c
    unit/test_picoui_slider.c
    unit/test_picoui_switch.c
    unit/test_picoui_table.c
    unit/test_picoui_text.c
    unit/test_picoui_window.c
    unit/test_picoui_graph.c
    unit/test_picoui_calendar.c
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

target_link_libraries(test_picoui_port_sdl PRIVATE picoui_port_sdl)

ld_add_python_test(check_picoui_public_api
    SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/contract/check_picoui_public_api.py"
    LABELS "picoui;contract"
)

ld_add_python_test(check_picoui_demo_boundary
    SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/contract/check_picoui_demo_boundary.py"
    LABELS "picoui;contract"
)

ld_add_python_test(check_picoui_widget_contract_matrix
    SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/contract/check_picoui_widget_contract_matrix.py"
    LABELS "picoui;contract"
)

ld_add_python_test(check_picoui_release_capability_matrix
    SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/contract/check_picoui_release_capability_matrix.py"
    LABELS "picoui;contract;release"
)

if(LD_BUILD_RUNTIME_TESTS)
    ld_add_python_test(check_picoui_runtime
        SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/runtime/check_picoui_runtime.py"
        LABELS "picoui;runtime"
    )

    find_package(Python3 COMPONENTS Interpreter REQUIRED)
    add_test(
        NAME check_picoui_visible_ui
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/runtime/check_picoui_visible_ui.py" --all
    )
    set_tests_properties(check_picoui_visible_ui PROPERTIES LABELS "picoui;runtime;visible")

    ld_add_python_test(check_picoui_backend_mapping
        SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/runtime/check_picoui_backend_mapping.py"
        LABELS "picoui;runtime;backend;mapping"
    )
endif()
