set(LD_REPO_ROOT "${CMAKE_CURRENT_LIST_DIR}/..")
set(LD_SRC_DIR "${LD_REPO_ROOT}/src")
set(LD_GUI_DIR "${LD_SRC_DIR}/gui")
set(LD_MISC_DIR "${LD_SRC_DIR}/misc")
if(NOT DEFINED LD_PORTING_DIR)
    set(LD_PORTING_DIR "${LD_SRC_DIR}/porting")
endif()
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
set(LD_GUI_TEST_DIR "${LD_GUI_DIR}/test")

function(ld_validate_demo_id)
    set(_supported_demos 1 2 3 4 5)
    list(FIND _supported_demos "${USE_DEMO}" _demo_index)
    if(_demo_index EQUAL -1)
        message(FATAL_ERROR "Unsupported USE_DEMO=${USE_DEMO}. Supported values: 1, 2, 3, 4, 5.")
    endif()
endfunction()

file(GLOB LD_LONGDONGGUI_GUI_SOURCES CONFIGURE_DEPENDS
    "${LD_GUI_DIR}/*.c"
)
file(GLOB LD_LONGDONGGUI_MISC_SOURCES CONFIGURE_DEPENDS
    "${LD_MISC_DIR}/*.c"
)
file(GLOB LD_ARM2D_LIBRARY_SOURCES CONFIGURE_DEPENDS
    "${LD_ARM2D_LIBRARY_DIR}/Source/*.c"
)
file(GLOB LD_ARM2D_HELPER_SOURCES CONFIGURE_DEPENDS
    "${LD_ARM2D_HELPER_DIR}/Source/*.c"
)
file(GLOB LD_MATH_SOURCES CONFIGURE_DEPENDS
    "${LD_MATH_DIR}/*.c"
)
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
set(LD_GUI_HOST_TEST_SOURCES
    "${LD_GUI_DIR}/ldBase.c"
    "${LD_GUI_DIR}/ldWindow.c"
    "${LD_GUI_DIR}/ldWindowLayoutInternal.c"
    "${LD_GUI_DIR}/ldLabel.c"
    "${LD_GUI_DIR}/ldProgressBar.c"
    "${LD_MISC_DIR}/freeRtosHeap4.c"
    "${LD_MISC_DIR}/ldMsg.c"
    "${LD_MISC_DIR}/xBtnAction.c"
    "${LD_MISC_DIR}/xQueue.c"
    "${LD_MISC_DIR}/xString.c"
)

set(LD_LONGDONGGUI_INCLUDE_DIRS
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
    target_include_directories(longdonggui_arm2d
        PUBLIC
            ${LD_LONGDONGGUI_INCLUDE_DIRS}
    )
    ld_apply_common_target_config(longdonggui_arm2d)

    add_library(longdonggui STATIC
        ${LD_LONGDONGGUI_GUI_SOURCES}
        ${LD_LONGDONGGUI_MISC_SOURCES}
    )
    target_include_directories(longdonggui
        PUBLIC
            ${LD_LONGDONGGUI_INCLUDE_DIRS}
    )
    target_link_libraries(longdonggui
        PUBLIC
            longdonggui_arm2d
    )
    ld_apply_common_target_config(longdonggui)

    add_library(longdonggui_porting_default STATIC
        "${LD_PORTING_DIR}/ldConfig.c"
        "${LD_PORTING_DIR}/arm_2d_disp_adapter_0.c"
        ${LD_PERF_COUNTER_SOURCES}
    )
    target_include_directories(longdonggui_porting_default
        PUBLIC
            ${LD_LONGDONGGUI_INCLUDE_DIRS}
    )
    target_link_libraries(longdonggui_porting_default
        PUBLIC
            longdonggui
    )
    ld_apply_common_target_config(longdonggui_porting_default)

    add_library(longdonggui_host STATIC
        ${LD_GUI_HOST_TEST_SOURCES}
    )
    target_include_directories(longdonggui_host
        PUBLIC
            ${LD_LONGDONGGUI_INCLUDE_DIRS}
            "${LD_GUI_TEST_DIR}"
    )
    target_link_libraries(longdonggui_host
        PUBLIC
            longdonggui_arm2d
    )
    ld_apply_common_target_config(longdonggui_host)

    if(LD_ENABLE_COVERAGE)
        find_program(LD_LCOV_EXECUTABLE lcov)
        find_program(LD_GENHTML_EXECUTABLE genhtml)

        if(LD_LCOV_EXECUTABLE AND LD_GENHTML_EXECUTABLE)
            add_custom_target(coverage
                COMMAND "${CMAKE_CTEST_COMMAND}" --test-dir "${CMAKE_BINARY_DIR}" --output-on-failure
                COMMAND "${LD_LCOV_EXECUTABLE}" --capture --directory "${CMAKE_BINARY_DIR}" --output-file "${CMAKE_BINARY_DIR}/coverage.info"
                COMMAND "${LD_GENHTML_EXECUTABLE}" "${CMAKE_BINARY_DIR}/coverage.info" --output-directory "${CMAKE_BINARY_DIR}/coverage-html"
                WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
                USES_TERMINAL
            )
        else()
            add_custom_target(coverage
                COMMAND "${CMAKE_COMMAND}" -E echo "lcov/genhtml not found. Run ctest first, then manually capture coverage from ${CMAKE_BINARY_DIR}."
                USES_TERMINAL
            )
        endif()
    endif()
endfunction()

function(ld_collect_demo_sources out_sources out_include_dirs)
    set(_demo_sources)
    set(_demo_include_dirs)

    ld_validate_demo_id()

    if(USE_DEMO STREQUAL "1")
        file(GLOB _demo_sources CONFIGURE_DEPENDS
            "${LD_DEMO_DIR}/startup/*.c"
            "${LD_DEMO_DIR}/startup/fonts/*.c"
            "${LD_DEMO_DIR}/startup/images/*.c"
        )
        set(_demo_include_dirs
            "${LD_DEMO_DIR}/startup"
            "${LD_DEMO_DIR}/startup/fonts"
            "${LD_DEMO_DIR}/startup/images"
        )
    elseif(USE_DEMO STREQUAL "2")
        file(GLOB _demo_sources CONFIGURE_DEPENDS
            "${LD_DEMO_DIR}/widget/*.c"
            "${LD_DEMO_DIR}/widget/fonts/*.c"
            "${LD_DEMO_DIR}/widget/images/*.c"
        )
        set(_demo_include_dirs
            "${LD_DEMO_DIR}/widget"
            "${LD_DEMO_DIR}/widget/fonts"
            "${LD_DEMO_DIR}/widget/images"
        )
    elseif(USE_DEMO STREQUAL "3")
        file(GLOB _demo_sources CONFIGURE_DEPENDS
            "${LD_DEMO_DIR}/printer/*.c"
            "${LD_DEMO_DIR}/printer/fonts/*.c"
            "${LD_DEMO_DIR}/printer/images/*.c"
        )
        set(_demo_include_dirs
            "${LD_DEMO_DIR}/printer"
            "${LD_DEMO_DIR}/printer/fonts"
            "${LD_DEMO_DIR}/printer/images"
        )
    elseif(USE_DEMO STREQUAL "4" OR USE_DEMO STREQUAL "5")
        file(GLOB _demo_sources CONFIGURE_DEPENDS
            "${LD_DEMO_DIR}/layout/*.c"
            "${LD_DEMO_DIR}/widget/fonts/arial_12.c"
            "${LD_DEMO_DIR}/widget/fonts/arial_16.c"
        )
        set(_demo_include_dirs
            "${LD_DEMO_DIR}/layout"
            "${LD_DEMO_DIR}/widget/fonts"
        )
    endif()

    set(${out_sources} "${_demo_sources}" PARENT_SCOPE)
    set(${out_include_dirs} "${_demo_include_dirs}" PARENT_SCOPE)
endfunction()

function(ld_add_sdl_demo_target)
    if(TARGET ldgui_sdl_demo)
        return()
    endif()

    set(LD_SDL_EXAMPLE_DIR "${LD_EXAMPLES_DIR}/sdl")
    ld_collect_demo_sources(LD_SDL_DEMO_SOURCES LD_SDL_DEMO_INCLUDE_DIRS)

    add_executable(ldgui_sdl_demo
        "${LD_SDL_EXAMPLE_DIR}/virtualNor/virtualNor.c"
        "${LD_SDL_EXAMPLE_DIR}/user/main.c"
        "${LD_SDL_EXAMPLE_DIR}/user/ldConfig.c"
        "${LD_SDL_EXAMPLE_DIR}/user/Virtual_TFT_Port.c"
        "${LD_SDL_EXAMPLE_DIR}/user/arm_2d_disp_adapter_0.c"
        ${LD_SDL_DEMO_SOURCES}
    )

    target_include_directories(ldgui_sdl_demo PRIVATE
        "${LD_SDL_EXAMPLE_DIR}/user"
        "${LD_SDL_EXAMPLE_DIR}/virtualNor"
        ${LD_SDL_DEMO_INCLUDE_DIRS}
    )
    target_link_libraries(ldgui_sdl_demo PRIVATE longdonggui)
    ld_apply_common_target_config(ldgui_sdl_demo)

    find_package(Threads REQUIRED)
    if(TARGET Threads::Threads)
        target_link_libraries(ldgui_sdl_demo PRIVATE Threads::Threads)
    endif()

    if(WIN32)
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(SDL2_ROOT "${LD_SDL_EXAMPLE_DIR}/sdl2/64")
        elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
            set(SDL2_ROOT "${LD_SDL_EXAMPLE_DIR}/sdl2/32")
        else()
            message(FATAL_ERROR "Unsupported pointer size: ${CMAKE_SIZEOF_VOID_P}")
        endif()

        target_include_directories(ldgui_sdl_demo PRIVATE "${SDL2_ROOT}/include/SDL2")
        target_link_directories(ldgui_sdl_demo PRIVATE "${SDL2_ROOT}/lib")
        target_link_libraries(ldgui_sdl_demo PRIVATE SDL2 SDL2main)

        add_custom_command(TARGET ldgui_sdl_demo POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                "${SDL2_ROOT}/bin/SDL2.dll"
                "$<TARGET_FILE_DIR:ldgui_sdl_demo>/SDL2.dll"
            VERBATIM
        )
    else()
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(SDL2 REQUIRED sdl2)

        target_include_directories(ldgui_sdl_demo PRIVATE ${SDL2_INCLUDE_DIRS})
        target_link_directories(ldgui_sdl_demo PRIVATE ${SDL2_LIBRARY_DIRS})
        target_compile_options(ldgui_sdl_demo PRIVATE ${SDL2_CFLAGS_OTHER})
        target_link_options(ldgui_sdl_demo PRIVATE ${SDL2_LDFLAGS_OTHER})
        target_link_libraries(ldgui_sdl_demo PRIVATE ${SDL2_LIBRARIES})
    endif()
endfunction()
