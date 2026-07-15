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
    set(_tinyui_unit_env "SDL_VIDEODRIVER=dummy")
    if(CMAKE_C_FLAGS MATCHES "-fsanitize=address" OR CMAKE_EXE_LINKER_FLAGS MATCHES "-fsanitize=address")
        # ASan with large static data/globals can OOM under default RSS soft limit
        # and abort with a bare SIGSEGV (no ASan report). Disable soft RSS limit.
        set(_tinyui_unit_env "${_tinyui_unit_env};ASAN_OPTIONS=detect_leaks=0:abort_on_error=0:soft_rss_limit_mb=0:hard_rss_limit_mb=0:symbolize=1")
    endif()
    set_tests_properties(${tinyui_target} PROPERTIES ENVIRONMENT "${_tinyui_unit_env}")
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_definitions(${tinyui_target} PRIVATE _GNU_SOURCE)
    endif()
endfunction()

foreach(test_src IN LISTS TINYUI_UNIT_TESTS)
    ld_register_tinyui_unit_test("${test_src}")
endforeach()

# M2 closeout label: runtime/object/timer/event/four samples/allocation probes.
# Attach to existing unit targets only; do not invent a bypass fake target.
set(_TINYUI_M2_CORE_UNIT_TESTS
    test_tinyui_runtime_model
    test_tinyui_app_lifecycle
    test_tinyui_app_timer
    test_tinyui_event
    test_tinyui_lifecycle
    test_tinyui_label
    test_tinyui_button_events
    test_tinyui_checkbox
    test_tinyui_slider
    test_tinyui_widgets
    test_tinyui_steady_state_allocation
)
foreach(_tinyui_m2_test IN LISTS _TINYUI_M2_CORE_UNIT_TESTS)
    if(TEST ${_tinyui_m2_test})
        get_test_property(${_tinyui_m2_test} LABELS _tinyui_m2_labels)
        if(NOT _tinyui_m2_labels)
            set(_tinyui_m2_labels "")
        endif()
        set_tests_properties(${_tinyui_m2_test} PROPERTIES
            LABELS "${_tinyui_m2_labels};tinyui_m2_core"
        )
    endif()
endforeach()

# M3 closeout label: full widget unit surface + lightweight probes + L5 gates.
# Reuses existing tests only; does not invent bypass targets.
set(_TINYUI_M3_CORE_UNIT_TESTS
    test_tinyui_runtime_model
    test_tinyui_app_lifecycle
    test_tinyui_app_timer
    test_tinyui_event
    test_tinyui_lifecycle
    test_tinyui_core_helpers
    test_tinyui_widgets
    test_tinyui_label
    test_tinyui_button_events
    test_tinyui_checkbox
    test_tinyui_slider
    test_tinyui_switch
    test_tinyui_progress_bar
    test_tinyui_progress_wheel
    test_tinyui_arc
    test_tinyui_gauge
    test_tinyui_list
    test_tinyui_combo_box
    test_tinyui_scroll_selecter
    test_tinyui_calendar
    test_tinyui_icon_slider
    test_tinyui_radial_menu
    test_tinyui_text
    test_tinyui_line_edit
    test_tinyui_keyboard
    test_tinyui_table
    test_tinyui_graph
    test_tinyui_image
    test_tinyui_canvas
    test_tinyui_qrcode
    test_tinyui_date_time
    test_tinyui_clock
    test_tinyui_animation
    test_tinyui_message_box
    test_tinyui_window
    test_tinyui_background
    test_tinyui_theme
    test_tinyui_layout
    test_tinyui_resource
    test_tinyui_steady_state_allocation
    test_tinyui_wrapper_struct_overhead
)
foreach(_tinyui_m3_test IN LISTS _TINYUI_M3_CORE_UNIT_TESTS)
    if(TEST ${_tinyui_m3_test})
        get_test_property(${_tinyui_m3_test} LABELS _tinyui_m3_labels)
        if(NOT _tinyui_m3_labels)
            set(_tinyui_m3_labels "")
        endif()
        set_tests_properties(${_tinyui_m3_test} PROPERTIES
            LABELS "${_tinyui_m3_labels};tinyui_m3_core"
        )
    endif()
endforeach()

ld_add_c_unit_test(test_tinyui_v23_runtime_contract
    SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/contract/test_tinyui_v23_runtime_contract.c"
    SUPPORT_LIB tinyui_test_support
    MAIN_LIB tinyui_backend_ldgui
    LABELS "tinyui;contract;m1;tinyui_m2_core"
)

ld_add_c_unit_test(test_tinyui_v23_event_timer_focus_contract
    SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/contract/test_tinyui_v23_event_timer_focus_contract.c"
    SUPPORT_LIB tinyui_test_support
    MAIN_LIB tinyui_backend_ldgui
    LABELS "tinyui;contract;m1;tinyui_m2_core"
)

ld_add_c_unit_test(test_tinyui_public_abi
    SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/abi/test_tinyui_public_abi.c"
    SUPPORT_LIB tinyui_test_support
    MAIN_LIB tinyui_backend_ldgui
    LABELS "tinyui;abi;m1;lightweight;tinyui_m2_core"
)

ld_add_c_unit_test(test_tinyui_internal_abi
    SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/abi/test_tinyui_internal_abi.c"
    SUPPORT_LIB tinyui_test_support
    MAIN_LIB tinyui_backend_ldgui
    LABELS "tinyui;abi;m1;lightweight;tinyui_m2_core"
)

# Compile-only 32-bit budget probe. Prefer an MCU cross compiler when present;
# otherwise try a host multilib -m32 compile. Absence of either toolchain only
# skips the target — it must never be recorded as a 32-bit ABI pass.
set(_TINYUI_ABI32_CC "")
if(EXISTS "/opt/gcc-arm-none-eabi-10-2020-q4-major/bin/arm-none-eabi-gcc")
    set(_TINYUI_ABI32_CC "/opt/gcc-arm-none-eabi-10-2020-q4-major/bin/arm-none-eabi-gcc")
elseif(CMAKE_C_COMPILER)
    execute_process(
        COMMAND ${CMAKE_C_COMPILER} -m32 -E -x c /dev/null
        RESULT_VARIABLE _TINYUI_HOST_M32_OK
        OUTPUT_QUIET ERROR_QUIET
    )
    if(_TINYUI_HOST_M32_OK EQUAL 0)
        set(_TINYUI_ABI32_CC "${CMAKE_C_COMPILER}")
        set(_TINYUI_ABI32_HOST_M32 ON)
    endif()
endif()

if(_TINYUI_ABI32_CC)
    set(_TINYUI_ABI32_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/abi/test_tinyui_internal_abi.c")
    set(_TINYUI_ABI32_OBJECT "${CMAKE_CURRENT_BINARY_DIR}/tinyui_abi32_compile.o")
    set(_TINYUI_ABI32_INCLUDES
        "-I${CMAKE_SOURCE_DIR}/tinyui/include"
        "-I${CMAKE_BINARY_DIR}/generated"
    )
    if(_TINYUI_ABI32_HOST_M32)
        add_custom_command(
            OUTPUT "${_TINYUI_ABI32_OBJECT}"
            COMMAND "${_TINYUI_ABI32_CC}" -m32 -std=c11 -c
                    ${_TINYUI_ABI32_INCLUDES}
                    -o "${_TINYUI_ABI32_OBJECT}"
                    "${_TINYUI_ABI32_SOURCE}"
            DEPENDS "${_TINYUI_ABI32_SOURCE}"
            COMMENT "Compile TinyUI 32-bit internal ABI probe (-m32)"
            VERBATIM
        )
    else()
        add_custom_command(
            OUTPUT "${_TINYUI_ABI32_OBJECT}"
            COMMAND "${_TINYUI_ABI32_CC}" -std=c11 -ffreestanding -c
                    ${_TINYUI_ABI32_INCLUDES}
                    -o "${_TINYUI_ABI32_OBJECT}"
                    "${_TINYUI_ABI32_SOURCE}"
            DEPENDS "${_TINYUI_ABI32_SOURCE}"
            COMMENT "Compile TinyUI 32-bit internal ABI probe (MCU toolchain)"
            VERBATIM
        )
    endif()
    add_custom_target(tinyui_abi32_compile DEPENDS "${_TINYUI_ABI32_OBJECT}")
else()
    add_custom_target(tinyui_abi32_compile
        COMMAND ${CMAKE_COMMAND} -E echo
                "tinyui_abi32_compile skipped: no 32-bit compiler available"
    )
endif()

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

# M2 Task 1: do not force tinyui_demo before app_lifecycle unit tests.
# Demo migration remains M4 full-tree debt; core runtime tests must build without it.
# if(TARGET test_tinyui_app_lifecycle AND TARGET tinyui_demo)
#     add_dependencies(test_tinyui_app_lifecycle tinyui_demo)
# endif()

# Full backend link probe: must pull the same start-group set as unit tests so
# LTO does not drop longdonggui/arm2d/porting symbols used by default resources.
ld_add_c_unit_test(test_tinyui_backend_link_default_resources
    SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/unit/test_tinyui_backend_link_default_resources.c"
    SUPPORT_LIB tinyui_test_support
    MAIN_LIB tinyui_backend_ldgui
    LABELS "tinyui;unit;link"
)
set_tests_properties(test_tinyui_backend_link_default_resources PROPERTIES
    ENVIRONMENT "SDL_VIDEODRIVER=dummy"
)

ld_add_c_unit_test(test_tinyui_wrapper_struct_overhead
    SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/unit/test_tinyui_wrapper_struct_overhead.c"
    SUPPORT_LIB tinyui_test_support
    MAIN_LIB tinyui_backend_ldgui
    LABELS "tinyui;probe;memory;tinyui_m2_core;tinyui_m3_core"
)

# M5 Task 2: formal ABI gate names required by release manifest.
# Thin probes share the same budgets as wrapper/object-overhead evidence.
ld_add_c_unit_test(test_tinyui_pool_abi
    SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/unit/test_tinyui_pool_abi.c"
    SUPPORT_LIB tinyui_test_support
    MAIN_LIB tinyui_backend_ldgui
    LABELS "tinyui;probe;memory;abi;pool;tinyui_m2_core;tinyui_m3_core"
)

ld_add_c_unit_test(test_tinyui_descriptor_abi
    SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/unit/test_tinyui_descriptor_abi.c"
    SUPPORT_LIB tinyui_test_support
    MAIN_LIB tinyui_backend_ldgui
    LABELS "tinyui;probe;memory;abi;descriptor;tinyui_m2_core;tinyui_m3_core"
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
    LABELS "tinyui;contract;demo;consumer"
)

ld_add_python_test(test_check_tinyui_demo_boundary
    SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/contract/test_check_tinyui_demo_boundary.py"
    LABELS "tinyui;contract;demo;unit"
)

# M4 Task 6: install package + out-of-tree C/C++ find_package consumer.
add_test(
    NAME check_tinyui_install_consumer
    COMMAND "${CMAKE_COMMAND}"
        "-Dbinary_dir=${CMAKE_BINARY_DIR}"
        "-Dsource_dir=${CMAKE_SOURCE_DIR}"
        "-Dconsumer_src=${CMAKE_CURRENT_SOURCE_DIR}/consumer"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/consumer/run_install_consumer.cmake"
)
set_tests_properties(check_tinyui_install_consumer PROPERTIES
    LABELS "tinyui;contract;install;consumer"
)

# M4 Task 7: current-facing docs forbidden tokens + ```c/cpp compile``` vs install package.
find_package(Python3 COMPONENTS Interpreter REQUIRED)
add_test(
    NAME check_tinyui_docs_examples
    COMMAND "${Python3_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/contract/check_tinyui_docs_examples.py"
            --root "${CMAKE_SOURCE_DIR}"
            --build-dir "${CMAKE_BINARY_DIR}/_tinyui_docs_examples"
            --prefix "${CMAKE_BINARY_DIR}/_tinyui_install"
            --host-build "${CMAKE_BINARY_DIR}"
)
set_tests_properties(check_tinyui_docs_examples PROPERTIES
    LABELS "tinyui;contract;docs;consumer"
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
    LABELS "tinyui;contract;release;tinyui_m3_core"
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
set_tests_properties(check_tinyui_object_overhead PROPERTIES LABELS "tinyui;perf;memory;tinyui_m2_core;tinyui_m3_core")

ld_add_python_test(test_check_tinyui_object_overhead
    SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/perf/test_check_tinyui_object_overhead.py"
    LABELS "tinyui;perf;memory;unit;tinyui_m2_core"
)

add_executable(tinyui_minimal_consumer
    "${CMAKE_CURRENT_SOURCE_DIR}/minimal/minimal_consumer.c"
)
if(CMAKE_C_COMPILER_ID MATCHES "GNU" OR (CMAKE_C_COMPILER_ID MATCHES "Clang" AND NOT APPLE))
    target_link_libraries(tinyui_minimal_consumer PRIVATE
        -Wl,--start-group
        tinyui_backend_ldgui
        tinyui_port_mcu
        tinyui_core
        longdonggui
        longdonggui_arm2d
        tinyui_backend_ldgui_porting
        -Wl,--end-group
        m
    )
else()
    target_link_libraries(tinyui_minimal_consumer PRIVATE tinyui_backend_ldgui tinyui_port_mcu)
endif()
ld_apply_common_target_config(tinyui_minimal_consumer)
target_compile_options(tinyui_minimal_consumer PRIVATE
    -ffunction-sections
    -fdata-sections
)
# Full-profile core/libs use LTO; only force non-LTO objects when the whole
# minimal profile already disabled LTO so nm/map inspection stays plain.
if(TINYUI_PROFILE STREQUAL "minimal")
    target_compile_options(tinyui_minimal_consumer PRIVATE -fno-lto)
    target_link_options(tinyui_minimal_consumer PRIVATE -fno-lto)
endif()
if(CMAKE_C_COMPILER_ID MATCHES "GNU" OR (CMAKE_C_COMPILER_ID MATCHES "Clang" AND NOT APPLE))
    target_link_options(tinyui_minimal_consumer PRIVATE -Wl,--gc-sections,-Map,$<TARGET_FILE:tinyui_minimal_consumer>.map)
elseif(CMAKE_C_COMPILER_ID STREQUAL "AppleClang")
    target_link_options(tinyui_minimal_consumer PRIVATE -Wl,-dead_strip,-map,$<TARGET_FILE:tinyui_minimal_consumer>.map)
else()
    message(FATAL_ERROR "tinyui_minimal_consumer requires GNU, Clang, or AppleClang for section GC and link map")
endif()

# M5 Task 2: formal name check_tinyui_minimal_profile must exist in every
# profile inventory and stay enabled. Symbol enforce is meaningful only for a
# non-LTO minimal artifact; full/demo trees therefore consume a sibling
# build/v2.3-minimal (or BUILD_V23_MINIMAL_DIR / TINYUI_MINIMAL_BUILD_DIR).
if(NOT DEFINED TINYUI_V23_MINIMAL_BUILD_DIR OR TINYUI_V23_MINIMAL_BUILD_DIR STREQUAL "")
    set(TINYUI_V23_MINIMAL_BUILD_DIR "${CMAKE_BINARY_DIR}/../v2.3-minimal")
endif()
add_test(
    NAME check_tinyui_minimal_profile
    COMMAND "${Python3_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/perf/check_tinyui_minimal_profile_wrapper.py"
            --current-profile "${TINYUI_PROFILE}"
            --binary "$<TARGET_FILE:tinyui_minimal_consumer>"
            --map "$<TARGET_FILE:tinyui_minimal_consumer>.map"
            --build-dir "${CMAKE_BINARY_DIR}"
            --minimal-build-dir "${TINYUI_V23_MINIMAL_BUILD_DIR}"
            --baseline "${CMAKE_CURRENT_SOURCE_DIR}/perf/tinyui_perf_baseline.json"
)
set_tests_properties(check_tinyui_minimal_profile PROPERTIES LABELS "tinyui;contract;size;minimal")

add_test(
    NAME test_check_tinyui_minimal_symbols
    COMMAND "${Python3_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/perf/test_check_tinyui_minimal_symbols.py"
)
set_tests_properties(test_check_tinyui_minimal_symbols PROPERTIES LABELS "tinyui;contract;size;minimal;unit")

add_test(
    NAME test_check_tinyui_feature_options
    COMMAND "${Python3_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/contract/test_check_tinyui_feature_options.py"
)
set_tests_properties(test_check_tinyui_feature_options PROPERTIES LABELS "tinyui;contract;unit;m1")

add_test(
    NAME check_tinyui_feature_options
    COMMAND "${Python3_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/contract/check_tinyui_feature_options.py"
            --root "${CMAKE_SOURCE_DIR}"
            --build-dir "${CMAKE_BINARY_DIR}"
            --target tinyui_core
)
set_tests_properties(check_tinyui_feature_options PROPERTIES LABELS "tinyui;contract;m1")

add_test(
    NAME test_check_tinyui_lightweight_architecture
    COMMAND "${Python3_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/contract/test_check_tinyui_lightweight_architecture.py"
)
set_tests_properties(test_check_tinyui_lightweight_architecture
    PROPERTIES LABELS "tinyui;contract;unit;m1;lightweight"
)

add_test(
    NAME check_tinyui_lightweight_architecture
    COMMAND "${Python3_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/contract/check_tinyui_lightweight_architecture.py"
            --root "${CMAKE_SOURCE_DIR}"
)
set_tests_properties(check_tinyui_lightweight_architecture
    PROPERTIES LABELS "tinyui;contract;m1;lightweight"
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

    # M2 Task 9 / M3 Task 10: focused L5 evidence gates for v23 scenarios.
    # Keep full-tree --all visible/mapping checks separate (M4 demo debt).
    set(_v23_scenarios
        v23_core_vertical
        v23_value_instruments
        v23_selection_collection
        v23_input_data
        v23_media_composite
        v23_theme_layout_resource
    )
    foreach(_scenario IN LISTS _v23_scenarios)
        add_test(
            NAME check_tinyui_visible_ui_${_scenario}
            COMMAND "${Python3_EXECUTABLE}"
                    "${CMAKE_CURRENT_SOURCE_DIR}/runtime/check_tinyui_visible_ui.py"
                    --scenario ${_scenario}
                    --build-dir "${CMAKE_BINARY_DIR}"
        )
        set_tests_properties(check_tinyui_visible_ui_${_scenario}
            PROPERTIES LABELS "tinyui;runtime;visible;tinyui_m3_l5;tinyui_m3_core"
        )
        add_test(
            NAME check_tinyui_backend_mapping_${_scenario}
            COMMAND "${Python3_EXECUTABLE}"
                    "${CMAKE_CURRENT_SOURCE_DIR}/runtime/check_tinyui_backend_mapping.py"
                    --scenario ${_scenario}
                    --build-dir "${CMAKE_BINARY_DIR}"
        )
        set_tests_properties(check_tinyui_backend_mapping_${_scenario}
            PROPERTIES LABELS "tinyui;runtime;backend;mapping;tinyui_m3_l5;tinyui_m3_core"
        )
    endforeach()
    # Preserve M2 core labels on the four-sample scenario.
    set_tests_properties(check_tinyui_visible_ui_v23_core_vertical
        PROPERTIES LABELS "tinyui;runtime;visible;tinyui_m2_core;tinyui_m3_l5;tinyui_m3_core"
    )
    set_tests_properties(check_tinyui_backend_mapping_v23_core_vertical
        PROPERTIES LABELS "tinyui;runtime;backend;mapping;tinyui_m2_core;tinyui_m3_l5;tinyui_m3_core"
    )

    ld_add_python_test(check_tinyui_backend_mapping
        SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/runtime/check_tinyui_backend_mapping.py"
        LABELS "tinyui;runtime;backend;mapping"
    )

    # M5 Task 2: formal L5-E aggregate gate (keeps per-scenario gates intact).
    add_test(
        NAME check_tinyui_event_evidence
        COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/runtime/check_tinyui_event_evidence.py"
                --build-dir "${CMAKE_BINARY_DIR}"
    )
    set_tests_properties(check_tinyui_event_evidence PROPERTIES
        LABELS "tinyui;runtime;event;tinyui_m3_l5;tinyui_m3_core"
    )
endif()

# M5 Task 1: fail-closed release gate aggregator (expected red until Task2+ evidence).
find_package(Python3 COMPONENTS Interpreter REQUIRED)
add_test(
    NAME check_tinyui_v23_release_gates
    COMMAND "${Python3_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/contract/check_tinyui_v23_release_gates.py"
            --build-dir "${CMAKE_BINARY_DIR}"
            --manifest "${CMAKE_CURRENT_SOURCE_DIR}/contract/tinyui_v23_release_gate_manifest.json"
            --artifact-root "${CMAKE_BINARY_DIR}/release-evidence"
)
set_tests_properties(check_tinyui_v23_release_gates PROPERTIES
    LABELS "tinyui;contract;release;v2.3"
)

ld_add_python_test(test_check_tinyui_v23_release_gates
    SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/contract/test_check_tinyui_v23_release_gates.py"
    LABELS "tinyui;contract;release;unit;v2.3"
)
