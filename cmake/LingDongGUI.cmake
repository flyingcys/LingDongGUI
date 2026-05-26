include(CMakeParseArguments)

set(LD_REPO_ROOT "${CMAKE_CURRENT_LIST_DIR}/..")
set(LD_SRC_DIR "${LD_REPO_ROOT}/src")
set(LD_GUI_DIR "${LD_SRC_DIR}/gui")
set(LD_MISC_DIR "${LD_SRC_DIR}/misc")
set(LD_PORTING_DIR "${LD_SRC_DIR}/porting")
set(LD_EXAMPLES_DIR "${LD_REPO_ROOT}/examples")
set(LD_COMMON_DIR "${LD_EXAMPLES_DIR}/common")
set(LD_ARM2D_DIR "${LD_COMMON_DIR}/Arm-2D")
set(LD_ARM2D_LIBRARY_DIR "${LD_ARM2D_DIR}/Library")
set(LD_ARM2D_HELPER_DIR "${LD_ARM2D_DIR}/Helper")
set(LD_ARM2D_COMMON_DIR "${LD_COMMON_DIR}/Arm-2D/examples/common")
set(LD_ARM2D_CONTROLS_DIR "${LD_ARM2D_COMMON_DIR}/controls")
set(LD_ARM2D_QRCODE_DIR "${LD_ARM2D_CONTROLS_DIR}/qrcode_box")
set(LD_MATH_DIR "${LD_COMMON_DIR}/math")
set(LD_DEMO_DIR "${LD_COMMON_DIR}/demo")
set(LD_TESTS_DIR "${LD_REPO_ROOT}/tests")

file(GLOB LD_LONGDONGGUI_GUI_SOURCES CONFIGURE_DEPENDS "${LD_GUI_DIR}/*.c")
file(GLOB LD_LONGDONGGUI_MISC_SOURCES CONFIGURE_DEPENDS "${LD_MISC_DIR}/*.c")
file(GLOB LD_ARM2D_LIBRARY_SOURCES CONFIGURE_DEPENDS "${LD_ARM2D_LIBRARY_DIR}/Source/*.c")
file(GLOB LD_ARM2D_HELPER_SOURCES CONFIGURE_DEPENDS "${LD_ARM2D_HELPER_DIR}/Source/*.c")
file(GLOB LD_MATH_SOURCES CONFIGURE_DEPENDS "${LD_MATH_DIR}/*.c")

set(LD_PERF_COUNTER_SOURCES
    "${LD_COMMON_DIR}/perf_counter/perf_counter.c"
    "${LD_COMMON_DIR}/perf_counter/perfc_port_default.c"
)
set(LD_ARM2D_CONTROL_SOURCES
    "${LD_ARM2D_CONTROLS_DIR}/controls.c"
    "${LD_ARM2D_CONTROLS_DIR}/spinning_wheel.c"
    "${LD_ARM2D_CONTROLS_DIR}/progress_wheel.c"
    "${LD_ARM2D_CONTROLS_DIR}/text_box.c"
)
set(LD_ARM2D_QRCODE_SOURCES
    "${LD_ARM2D_QRCODE_DIR}/qrcode_box.c"
    "${LD_ARM2D_QRCODE_DIR}/qrcodegen.c"
)

set(LD_COMMON_INCLUDE_DIRS
    "${LD_GUI_DIR}"
    "${LD_MISC_DIR}"
    "${LD_PORTING_DIR}"
    "${LD_ARM2D_DIR}"
    "${LD_ARM2D_HELPER_DIR}/Include"
    "${LD_ARM2D_LIBRARY_DIR}/Include"
    "${LD_ARM2D_LIBRARY_DIR}/Source"
    "${LD_ARM2D_COMMON_DIR}"
    "${LD_ARM2D_CONTROLS_DIR}"
    "${LD_ARM2D_QRCODE_DIR}"
    "${LD_MATH_DIR}"
)

function(ld_apply_common_target_config target)
    target_compile_definitions(${target}
        PUBLIC
            __va_list=va_list
            RTE_Acceleration_Arm_2D_Helper_Disp_Adapter0
            RTE_Acceleration_Arm_2D_Alpha_Blending
    )

    target_compile_options(${target}
        PRIVATE
            -fdiagnostics-color=always
            -w
            -g
            -std=gnu11
            -MMD
            -ffunction-sections
            -fdata-sections
            -fno-ms-extensions
            -Wno-macro-redefined
            "-D__ARM_2D_USER_APP_CFG_H__=\"ldConfig.h\""
            "-DARM_SECTION(x)="
    )

    if(LD_ENABLE_COVERAGE AND CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${target} PRIVATE -O0 --coverage)
        target_link_options(${target} PRIVATE --coverage)
    else()
        target_compile_options(${target} PRIVATE -Ofast -flto)
        target_link_options(${target} PRIVATE -Ofast -flto)
    endif()
endfunction()

function(ld_define_core_targets)
    if(TARGET longdonggui)
        return()
    endif()

    add_library(longdonggui_arm2d STATIC
        ${LD_ARM2D_LIBRARY_SOURCES}
        ${LD_ARM2D_HELPER_SOURCES}
        ${LD_ARM2D_CONTROL_SOURCES}
        ${LD_ARM2D_QRCODE_SOURCES}
        ${LD_MATH_SOURCES}
    )
    target_include_directories(longdonggui_arm2d PUBLIC ${LD_COMMON_INCLUDE_DIRS})
    ld_apply_common_target_config(longdonggui_arm2d)

    add_library(longdonggui STATIC
        ${LD_LONGDONGGUI_GUI_SOURCES}
        ${LD_LONGDONGGUI_MISC_SOURCES}
    )
    target_include_directories(longdonggui PUBLIC ${LD_COMMON_INCLUDE_DIRS})
    target_link_libraries(longdonggui PUBLIC longdonggui_arm2d)
    ld_apply_common_target_config(longdonggui)

    add_library(longdonggui_porting_default STATIC
        "${LD_PORTING_DIR}/ldConfig.c"
        "${LD_PORTING_DIR}/arm_2d_disp_adapter_0.c"
        ${LD_PERF_COUNTER_SOURCES}
    )
    target_include_directories(longdonggui_porting_default PUBLIC ${LD_COMMON_INCLUDE_DIRS})
    target_link_libraries(longdonggui_porting_default PUBLIC longdonggui)
    ld_apply_common_target_config(longdonggui_porting_default)

    add_library(longdonggui_host STATIC
        ${LD_LONGDONGGUI_GUI_SOURCES}
        ${LD_LONGDONGGUI_MISC_SOURCES}
    )
    target_include_directories(longdonggui_host PUBLIC ${LD_COMMON_INCLUDE_DIRS})
    target_link_libraries(longdonggui_host PUBLIC longdonggui_arm2d)
    ld_apply_common_target_config(longdonggui_host)
endfunction()

function(ld_add_c_unit_test target)
    cmake_parse_arguments(LDTEST "" "SUPPORT_LIB;MAIN_LIB" "SOURCES;LABELS" ${ARGN})
    add_executable(${target} ${LDTEST_SOURCES})
    target_link_libraries(${target} PRIVATE ${LDTEST_SUPPORT_LIB} ${LDTEST_MAIN_LIB})
    ld_apply_common_target_config(${target})
    add_test(NAME ${target} COMMAND ${target})
    if(LDTEST_LABELS)
        set_tests_properties(${target} PROPERTIES LABELS "${LDTEST_LABELS}")
    endif()
endfunction()

function(ld_add_python_test test_name)
    cmake_parse_arguments(LDPY "" "SCRIPT" "LABELS" ${ARGN})
    find_package(Python3 COMPONENTS Interpreter REQUIRED)
    add_test(NAME ${test_name} COMMAND "${Python3_EXECUTABLE}" "${LDPY_SCRIPT}")
    if(LDPY_LABELS)
        set_tests_properties(${test_name} PROPERTIES LABELS "${LDPY_LABELS}")
    endif()
endfunction()

function(ld_add_shell_test test_name)
    cmake_parse_arguments(LDSH "" "SCRIPT" "LABELS" ${ARGN})
    add_test(NAME ${test_name} COMMAND /bin/sh "${LDSH_SCRIPT}")
    if(LDSH_LABELS)
        set_tests_properties(${test_name} PROPERTIES LABELS "${LDSH_LABELS}")
    endif()
endfunction()
