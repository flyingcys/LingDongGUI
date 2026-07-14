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
    unit/test_tinyui_resource.c
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
    unit/test_tinyui_steady_state_allocation.c
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

ld_add_c_unit_test(test_tinyui_v23_runtime_contract
    SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/contract/test_tinyui_v23_runtime_contract.c"
    SUPPORT_LIB tinyui_test_support
    MAIN_LIB tinyui_backend_ldgui
    LABELS "tinyui;contract;m1"
)

ld_add_c_unit_test(test_tinyui_v23_event_timer_focus_contract
    SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/contract/test_tinyui_v23_event_timer_focus_contract.c"
    SUPPORT_LIB tinyui_test_support
    MAIN_LIB tinyui_backend_ldgui
    LABELS "tinyui;contract;m1"
)

ld_add_c_unit_test(test_tinyui_public_abi
    SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/abi/test_tinyui_public_abi.c"
    SUPPORT_LIB tinyui_test_support
    MAIN_LIB tinyui_backend_ldgui
    LABELS "tinyui;abi;m1"
)

ld_add_c_unit_test(test_tinyui_v23_style_theme_contract
    SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/contract/test_tinyui_v23_style_theme_contract.c"
    SUPPORT_LIB tinyui_test_support
    MAIN_LIB tinyui_backend_ldgui
    LABELS "tinyui;contract;m1"
)
target_compile_definitions(test_tinyui_v23_style_theme_contract
    PRIVATE TINYUI_ENABLE_DIAGNOSTICS=1
)

ld_add_c_unit_test(test_tinyui_v23_layout_resource_contract
    SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/contract/test_tinyui_v23_layout_resource_contract.c"
    SUPPORT_LIB tinyui_test_support
    MAIN_LIB tinyui_backend_ldgui
    LABELS "tinyui;contract;m1"
)

if(TARGET test_tinyui_app_lifecycle AND TARGET tinyui_demo)
    add_dependencies(test_tinyui_app_lifecycle tinyui_demo)
endif()

add_executable(test_tinyui_backend_link_default_resources
    "${CMAKE_CURRENT_SOURCE_DIR}/unit/test_tinyui_backend_link_default_resources.c"
)
target_link_libraries(test_tinyui_backend_link_default_resources PRIVATE tinyui_backend_ldgui)
ld_apply_common_target_config(test_tinyui_backend_link_default_resources)
add_test(NAME test_tinyui_backend_link_default_resources COMMAND test_tinyui_backend_link_default_resources)
set_tests_properties(test_tinyui_backend_link_default_resources PROPERTIES
    ENVIRONMENT "SDL_VIDEODRIVER=dummy"
    LABELS "tinyui;unit;link"
)

ld_add_c_unit_test(test_tinyui_wrapper_struct_overhead
    SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/unit/test_tinyui_wrapper_struct_overhead.c"
    SUPPORT_LIB tinyui_test_support
    MAIN_LIB tinyui_backend_ldgui
    LABELS "tinyui;probe;memory"
)


# ── MCU no-SDL host verification harness ──────────────────────────────────
# Gate-B artifact for the port-layer remediation: proves the MCU port
# (tinyui_port_mcu) drives frames through the backend-neutral runtime loop and
# renders real pixels to a user flush callback WITHOUT linking SDL (no
# tinyui_port_sdl in the link line).
add_executable(mcu_host_smoke
    "${CMAKE_CURRENT_SOURCE_DIR}/port_mcu/mcu_host_smoke.c"
)
# GNU ld needs --start-group for the tinyui_core <-> port/porting circular refs;
# Apple ld64 rejects --start-group (resolves whole archives), so guard it like
# ld_add_c_unit_test does.
if(CMAKE_C_COMPILER_ID MATCHES "GNU" OR (CMAKE_C_COMPILER_ID MATCHES "Clang" AND NOT APPLE))
    target_link_libraries(mcu_host_smoke PRIVATE
        -Wl,--start-group
        tinyui_port_mcu
        tinyui_core
        longdonggui
        longdonggui_arm2d
        tinyui_backend_ldgui_porting
        -Wl,--end-group
        m
    )
else()
    target_link_libraries(mcu_host_smoke PRIVATE
        tinyui_port_mcu
        tinyui_core
        longdonggui
        longdonggui_arm2d
        tinyui_backend_ldgui_porting
        m
    )
endif()
ld_apply_common_target_config(mcu_host_smoke)
add_test(NAME mcu_host_smoke COMMAND mcu_host_smoke)
set_tests_properties(mcu_host_smoke PROPERTIES LABELS "tinyui;mcu")

find_package(Python3 COMPONENTS Interpreter REQUIRED)
file(GLOB_RECURSE TINYUI_PUBLIC_HEADER_CONTRACT_INPUTS CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/tinyui/include/*.h"
)
set(TINYUI_PUBLIC_CONTRACT_DIR "${CMAKE_BINARY_DIR}/generated/tinyui-public-contract")
execute_process(
    COMMAND "${Python3_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/contract/generate_tinyui_public_contract_probes.py"
            --root "${CMAKE_SOURCE_DIR}"
            --output-dir "${TINYUI_PUBLIC_CONTRACT_DIR}"
    RESULT_VARIABLE tinyui_public_contract_generate_result
    OUTPUT_VARIABLE tinyui_public_contract_generate_stdout
    ERROR_VARIABLE tinyui_public_contract_generate_stderr
)
if(NOT tinyui_public_contract_generate_result EQUAL 0)
    message(FATAL_ERROR
        "TinyUI public contract probe generation failed:\n"
        "${tinyui_public_contract_generate_stdout}${tinyui_public_contract_generate_stderr}"
    )
endif()
execute_process(
    COMMAND "${Python3_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/contract/check_tinyui_public_contract_manifest.py"
            --root "${CMAKE_SOURCE_DIR}"
            --output-dir "${TINYUI_PUBLIC_CONTRACT_DIR}"
    RESULT_VARIABLE tinyui_public_contract_manifest_result
    OUTPUT_VARIABLE tinyui_public_contract_manifest_stdout
    ERROR_VARIABLE tinyui_public_contract_manifest_stderr
)
if(NOT tinyui_public_contract_manifest_result EQUAL 0)
    message(FATAL_ERROR
        "TinyUI public contract probe manifest validation failed:\n"
        "${tinyui_public_contract_manifest_stdout}${tinyui_public_contract_manifest_stderr}"
    )
endif()
enable_language(CXX)
include("${TINYUI_PUBLIC_CONTRACT_DIR}/probes.cmake")

add_test(
    NAME check_tinyui_public_headers_c
    COMMAND "${CMAKE_COMMAND}" --build "${CMAKE_BINARY_DIR}" --target tinyui_public_header_probes_c
)
set_tests_properties(check_tinyui_public_headers_c PROPERTIES LABELS "tinyui;contract;public-header")
add_test(
    NAME check_tinyui_public_headers_cpp
    COMMAND "${CMAKE_COMMAND}" --build "${CMAKE_BINARY_DIR}" --target tinyui_public_header_probes_cpp
)
set_tests_properties(check_tinyui_public_headers_cpp PROPERTIES LABELS "tinyui;contract;public-header")
add_test(
    NAME check_tinyui_public_symbols_link
    COMMAND "${CMAKE_COMMAND}" --build "${CMAKE_BINARY_DIR}" --target tinyui_public_symbol_link_probes
)
set_tests_properties(check_tinyui_public_symbols_link PROPERTIES LABELS "tinyui;contract;public-link")

ld_add_python_test(check_tinyui_public_api
    SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/contract/check_tinyui_public_api.py"
    LABELS "tinyui;contract"
)

ld_add_python_test(test_check_tinyui_v23_public_api
    SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/contract/test_check_tinyui_v23_public_api.py"
    LABELS "tinyui;contract;unit;m1"
)

add_test(
    NAME check_tinyui_v23_public_api
    COMMAND "${Python3_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/contract/check_tinyui_v23_public_api.py"
            --root "${CMAKE_SOURCE_DIR}"
            --check "${CMAKE_CURRENT_SOURCE_DIR}/contract/tinyui_v23_public_api.json"
)
set_tests_properties(check_tinyui_v23_public_api PROPERTIES LABELS "tinyui;contract;m1")

find_package(Python3 COMPONENTS Interpreter REQUIRED)
add_test(
    NAME check_ldgui_public_api_inventory
    COMMAND "${Python3_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/contract/check_ldgui_public_api_inventory.py"
            --root "${CMAKE_SOURCE_DIR}"
            --output "${CMAKE_CURRENT_SOURCE_DIR}/contract/ldgui_public_api_inventory.json"
            --expected-output "${CMAKE_CURRENT_SOURCE_DIR}/contract/ldgui_public_api_expected_symbols.json"
            --check
)
set_tests_properties(check_ldgui_public_api_inventory PROPERTIES LABELS "tinyui;contract;inventory")

ld_add_python_test(check_tinyui_demo_boundary
    SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/contract/check_tinyui_demo_boundary.py"
    LABELS "tinyui;contract"
)

ld_add_python_test(check_tinyui_deprecated_api_usage
    SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/contract/check_tinyui_deprecated_api_usage.py"
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

add_test(
    NAME check_tinyui_binary_size
    COMMAND "${Python3_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/perf/check_tinyui_binary_size.py"
            --binary "${CMAKE_BINARY_DIR}/examples/sdl/tinyui_demo"
            --build-dir "${CMAKE_BINARY_DIR}"
            --baseline "${CMAKE_CURRENT_SOURCE_DIR}/perf/tinyui_perf_baseline.json"
            --enforce
)
set_tests_properties(check_tinyui_binary_size PROPERTIES LABELS "tinyui;perf;size")

ld_add_python_test(test_check_tinyui_binary_size
    SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/perf/test_check_tinyui_binary_size.py"
    LABELS "tinyui;perf;size;unit"
)

ld_add_python_test(check_tinyui_perf
    SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/perf/check_tinyui_perf.py"
    LABELS "tinyui;perf;runtime"
)

ld_add_python_test(test_tinyui_v23_baseline
    SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/perf/test_tinyui_v23_baseline.py"
    LABELS "tinyui;perf;unit"
)

add_test(
    NAME check_tinyui_v23_baseline
    COMMAND "${Python3_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/perf/check_tinyui_v23_baseline.py"
            check
            --baseline-dir "${CMAKE_CURRENT_SOURCE_DIR}/perf/v2.3-baselines"
            --build-dir "${CMAKE_BINARY_DIR}"
)
set_tests_properties(check_tinyui_v23_baseline PROPERTIES LABELS "tinyui;perf;runtime")

add_test(
    NAME check_tinyui_object_overhead
    COMMAND "${Python3_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/perf/check_tinyui_object_overhead.py"
            --probe "$<TARGET_FILE:test_tinyui_wrapper_struct_overhead>"
)
set_tests_properties(check_tinyui_object_overhead PROPERTIES LABELS "tinyui;perf;memory")

ld_add_python_test(test_check_tinyui_object_overhead
    SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/perf/test_check_tinyui_object_overhead.py"
    LABELS "tinyui;perf;memory;unit"
)

add_executable(tinyui_minimal_consumer
    "${CMAKE_CURRENT_SOURCE_DIR}/minimal/minimal_consumer.c"
)
target_link_libraries(tinyui_minimal_consumer PRIVATE tinyui_backend_ldgui tinyui_port_mcu)
ld_apply_common_target_config(tinyui_minimal_consumer)
target_compile_options(tinyui_minimal_consumer PRIVATE
    -fno-lto
    -ffunction-sections
    -fdata-sections
)
target_link_options(tinyui_minimal_consumer PRIVATE -fno-lto)
if(CMAKE_C_COMPILER_ID MATCHES "GNU" OR (CMAKE_C_COMPILER_ID MATCHES "Clang" AND NOT APPLE))
    target_link_options(tinyui_minimal_consumer PRIVATE -Wl,--gc-sections,-Map,$<TARGET_FILE:tinyui_minimal_consumer>.map)
elseif(CMAKE_C_COMPILER_ID STREQUAL "AppleClang")
    target_link_options(tinyui_minimal_consumer PRIVATE -Wl,-dead_strip,-map,$<TARGET_FILE:tinyui_minimal_consumer>.map)
else()
    message(FATAL_ERROR "tinyui_minimal_consumer requires GNU, Clang, or AppleClang for section GC and link map")
endif()

add_test(
    NAME check_tinyui_minimal_profile
    COMMAND "${Python3_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/perf/check_tinyui_minimal_symbols.py"
            --binary "$<TARGET_FILE:tinyui_minimal_consumer>"
            --build-dir "${CMAKE_BINARY_DIR}"
            --baseline "${CMAKE_CURRENT_SOURCE_DIR}/perf/tinyui_perf_baseline.json"
            --mode enforce
)
set_tests_properties(check_tinyui_minimal_profile PROPERTIES LABELS "tinyui;perf;minimal")

add_test(
    NAME check_tinyui_minimal_symbols
    COMMAND "${Python3_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/perf/check_tinyui_minimal_symbols.py"
            --binary "$<TARGET_FILE:tinyui_minimal_consumer>"
            --build-dir "${CMAKE_BINARY_DIR}"
            --baseline "${CMAKE_CURRENT_SOURCE_DIR}/perf/tinyui_perf_baseline.json"
            --mode enforce
)
set_tests_properties(check_tinyui_minimal_symbols PROPERTIES LABELS "tinyui;perf;minimal;legacy")

add_test(
    NAME test_check_tinyui_minimal_symbols
    COMMAND "${Python3_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/perf/test_check_tinyui_minimal_symbols.py"
)
set_tests_properties(test_check_tinyui_minimal_symbols PROPERTIES LABELS "tinyui;perf;minimal;unit")

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
