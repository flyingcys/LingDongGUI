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
    unit/test_picoui_port_sdl_boundary.c
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

set(PICOUI_V1_1_UNIT_TESTS
    unit/test_picoui_v1_1_public_headers.c
    unit/test_picoui_v1_1_port_sdl.c
    native/test_picoui_v1_1_window_tree.c
    native/test_picoui_v1_1_label_render.c
    native/test_picoui_v1_1_button_state.c
)

set(PICOUI_NATIVE_TESTS
    native/test_picoui_native_runtime.c
    native/test_picoui_native_sdl_port.c
    native/test_picoui_native_display_indev.c
    native/test_picoui_native_screen.c
    native/test_picoui_native_screen_window.c
    native/test_picoui_native_render_window_label_button.c
    native/test_picoui_native_background.c
    native/test_picoui_native_input_button.c
    native/test_picoui_native_checkbox.c
    native/test_picoui_native_switch.c
    native/test_picoui_native_slider.c
    native/test_picoui_native_arc.c
    native/test_picoui_native_gauge.c
    native/test_picoui_native_calendar.c
    native/test_picoui_native_clock.c
    native/test_picoui_native_combo_box.c
    native/test_picoui_native_date_time.c
    native/test_picoui_native_graph.c
    native/test_picoui_native_icon_slider.c
    native/test_picoui_native_text.c
    native/test_picoui_native_image.c
    native/test_picoui_native_list.c
    native/test_picoui_native_message_box.c
    native/test_picoui_native_progress.c
    native/test_picoui_native_qrcode.c
    native/test_picoui_native_radial_menu.c
    native/test_picoui_native_scroll_selecter.c
    native/test_picoui_native_table.c
    native/test_picoui_native_animation_canvas.c
    native/test_picoui_native_animation_frames.c
    native/test_picoui_native_canvas_renderer.c
    native/test_picoui_native_qrcode_renderer.c
    native/test_picoui_native_font_renderer.c
    native/test_picoui_native_image_renderer.c
    native/test_picoui_native_resource.c
    native/test_picoui_native_keyboard_line_edit.c
    native/test_picoui_native_widget_tree.c
    native/test_picoui_native_geometry_dirty.c
    native/test_picoui_native_flex_layout.c
    native/test_picoui_native_grid_layout.c
    native/test_picoui_native_style_theme.c
)

foreach(test_src IN LISTS PICOUI_UNIT_TESTS)
    get_filename_component(test_name "${test_src}" NAME_WE)
    ld_add_c_unit_test(${test_name}
        SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/${test_src}"
        SUPPORT_LIB picoui_test_support
        MAIN_LIB picoui
        LABELS "picoui;unit"
    )
endforeach()

foreach(test_src IN LISTS PICOUI_NATIVE_TESTS)
    get_filename_component(test_name "${test_src}" NAME_WE)
    ld_add_c_unit_test(${test_name}
        SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/${test_src}"
        SUPPORT_LIB picoui_test_support
        MAIN_LIB picoui
        LABELS "picoui;native"
    )
endforeach()

foreach(test_src IN LISTS PICOUI_V1_1_UNIT_TESTS)
    get_filename_component(test_name "${test_src}" NAME_WE)
    ld_add_c_unit_test(${test_name}
        SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/${test_src}"
        SUPPORT_LIB picoui_v1_1_test_support
        MAIN_LIB picoui_v1_1
        LABELS "picoui;unit;v1_1"
    )
endforeach()

add_dependencies(test_picoui_native_runtime picoui_basic_widgets_demo)
target_compile_definitions(test_picoui_native_runtime PRIVATE
    PICOUI_BASIC_WIDGETS_DEMO_PATH="$<TARGET_FILE:picoui_basic_widgets_demo>"
)

ld_add_python_test(check_picoui_public_api
    SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/contract/check_picoui_public_api.py"
    LABELS "picoui;contract"
)

ld_add_python_test(check_picoui_v1_1_boundary_contract
    SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/contract/check_picoui_v1_1_boundary_contract.py"
    LABELS "picoui;contract;v1_1"
)

ld_add_python_test(check_picoui_v1_1_build_graph
    SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/contract/check_picoui_v1_1_build_graph.py"
    LABELS "picoui;contract;v1_1"
)

ld_add_python_test(check_picoui_app_compatibility_decisions
    SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/contract/check_picoui_app_compatibility_decisions.py"
    LABELS "picoui;contract;native"
)

ld_add_python_test(check_picoui_demo_main_style
    SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/contract/check_picoui_demo_main_style.py"
    LABELS "picoui;contract;native"
)

ld_add_python_test(check_picoui_demo_main_style_inventory
    SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/contract/check_picoui_demo_main_style_inventory.py"
    LABELS "picoui;contract;native"
)

ld_add_python_test(check_picoui_sdl_boundary
    SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/contract/check_picoui_sdl_boundary.py"
    LABELS "picoui;contract;native"
)

ld_add_python_test(check_picoui_ldgui_dependency_inventory
    SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/contract/check_picoui_ldgui_dependency_inventory.py"
    LABELS "picoui;contract;native"
)

ld_add_python_test(check_picoui_no_ldgui_runtime_dependency
    SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/contract/check_picoui_no_ldgui_runtime_dependency.py"
    LABELS "picoui;contract;native"
)

ld_add_python_test(check_picoui_native_migration_ledger
    SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/contract/check_picoui_native_migration_ledger.py"
    LABELS "picoui;contract;native;release"
)

ld_add_python_test(check_picoui_native_release_matrix
    SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/contract/check_picoui_native_release_matrix.py"
    LABELS "picoui;contract;native;release"
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

    ld_add_python_test(check_picoui_v1_1_basic_widgets
        SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/runtime/check_picoui_v1_1_basic_widgets.py"
        LABELS "picoui;runtime;v1_1"
    )

    find_package(Python3 COMPONENTS Interpreter REQUIRED)
    add_test(
        NAME check_picoui_visible_ui
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/runtime/check_picoui_visible_ui.py" --all
    )
    set_tests_properties(check_picoui_visible_ui PROPERTIES LABELS "picoui;runtime;visible")

    ld_add_python_test(check_picoui_backend_mapping
        SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/runtime/check_picoui_backend_mapping.py"
        LABELS "runtime;backend;mapping;legacy"
    )
endif()
