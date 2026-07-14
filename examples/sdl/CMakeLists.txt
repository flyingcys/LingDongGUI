cmake_minimum_required(VERSION 3.16)

project(ldgui_sdl_demo LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

set(USE_DEMO "2" CACHE STRING "SDL demo id (0=legacy-widget, 1=startup, 2=widget, 3=printer, 4=layout, 5=grid, 6=widget-swipe)")
set_property(CACHE USE_DEMO PROPERTY STRINGS 0 1 2 3 4 5 6)

if(NOT COMMAND ld_apply_common_target_config)
    include("${CMAKE_CURRENT_LIST_DIR}/../../cmake/LingDongGUI.cmake")
    ld_define_core_targets()
endif()

set(SUPPORTED_DEMOS 0 1 2 3 4 5 6)
list(FIND SUPPORTED_DEMOS "${USE_DEMO}" USE_DEMO_INDEX)
if(USE_DEMO_INDEX EQUAL -1)
    message(FATAL_ERROR "Unsupported USE_DEMO=${USE_DEMO}. Supported values: 0, 1, 2, 3, 4, 5, 6.")
endif()

set(SDL_EXAMPLE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(EXAMPLES_DIR "${SDL_EXAMPLE_DIR}/..")
set(COMMON_DIR "${EXAMPLES_DIR}/common")
set(REPO_SRC_DIR "${SDL_EXAMPLE_DIR}/../../src")
set(ARM2D_COMMON_DIR "${COMMON_DIR}/Arm-2D/examples/common")
set(ARM2D_CONTROLS_DIR "${ARM2D_COMMON_DIR}/controls")
set(ARM2D_QRCODE_DIR "${ARM2D_CONTROLS_DIR}/qrcode_box")
set(ARM2D_ASSET_DIR "${ARM2D_COMMON_DIR}/asset")
set(DEMO_DIR "${COMMON_DIR}/demo")

set(SDL_DEMO_CORE_SOURCES
    "${SDL_EXAMPLE_DIR}/virtualNor/virtualNor.c"
    "${SDL_EXAMPLE_DIR}/user/main.c"
    "${SDL_EXAMPLE_DIR}/user/ldConfig.c"
    "${SDL_EXAMPLE_DIR}/user/Virtual_TFT_Port.c"
    "${SDL_EXAMPLE_DIR}/user/arm_2d_disp_adapter_0.c"
)

set(ARM2D_CONTROL_SOURCES
    "${ARM2D_CONTROLS_DIR}/controls.c"
    "${ARM2D_CONTROLS_DIR}/spinning_wheel.c"
    "${ARM2D_CONTROLS_DIR}/progress_wheel.c"
    "${ARM2D_CONTROLS_DIR}/text_box.c"
)
set(ARM2D_QRCODE_SOURCES
    "${ARM2D_QRCODE_DIR}/qrcode_box.c"
    "${ARM2D_QRCODE_DIR}/qrcodegen.c"
)
file(GLOB ARM2D_LIBRARY_SOURCES CONFIGURE_DEPENDS
    "${COMMON_DIR}/Arm-2D/Library/Source/*.c"
)
file(GLOB ARM2D_HELPER_SOURCES CONFIGURE_DEPENDS
    "${COMMON_DIR}/Arm-2D/Helper/Source/*.c"
)
file(GLOB MATH_SOURCES CONFIGURE_DEPENDS
    "${COMMON_DIR}/math/*.c"
)
file(GLOB GUI_SOURCES CONFIGURE_DEPENDS
    "${REPO_SRC_DIR}/gui/*.c"
)
file(GLOB MISC_SOURCES CONFIGURE_DEPENDS
    "${REPO_SRC_DIR}/misc/*.c"
)
file(GLOB WIDGET_FONT_SOURCES CONFIGURE_DEPENDS
    "${DEMO_DIR}/widget/fonts/*.c"
)
file(GLOB WIDGET_IMAGE_SOURCES CONFIGURE_DEPENDS
    "${DEMO_DIR}/widget/images/*.c"
)
file(GLOB WIDGET_SWIPE_PAGE_SOURCES CONFIGURE_DEPENDS
    "${DEMO_DIR}/widget/uiWidgetSwipePage*.c"
)
set(WIDGET_BASE_SOURCES
    "${DEMO_DIR}/widget/uiWidget.c"
    "${DEMO_DIR}/widget/uiWidgetLogic.c"
)
set(WIDGET_DEMO2_SOURCES
    ${WIDGET_BASE_SOURCES}
    "${DEMO_DIR}/widget/uiWidgetLegacy.c"
    "${DEMO_DIR}/widget/uiWidgetPage1.c"
    "${DEMO_DIR}/widget/uiWidgetPage2.c"
    "${DEMO_DIR}/widget/uiWidgetPage3.c"
    "${DEMO_DIR}/widget/uiWidgetPage4.c"
    ${WIDGET_FONT_SOURCES}
    ${WIDGET_IMAGE_SOURCES}
)
set(WIDGET_DEMO6_SOURCES
    ${WIDGET_BASE_SOURCES}
    "${DEMO_DIR}/widget/uiWidgetSwipeCommon.c"
    "${DEMO_DIR}/widget/uiWidgetSwipeLogic.c"
    ${WIDGET_SWIPE_PAGE_SOURCES}
    ${WIDGET_FONT_SOURCES}
    ${WIDGET_IMAGE_SOURCES}
)
set(DEMO_SOURCES)
set(DEMO_INCLUDE_DIRS)

if(USE_DEMO STREQUAL "0")
    set(DEMO_SOURCES
        "${DEMO_DIR}/widget/uiWidgetLegacy.c"
        ${WIDGET_FONT_SOURCES}
        ${WIDGET_IMAGE_SOURCES}
    )
    set(DEMO_INCLUDE_DIRS
        "${DEMO_DIR}/widget"
        "${DEMO_DIR}/widget/fonts"
        "${DEMO_DIR}/widget/images"
    )
elseif(USE_DEMO STREQUAL "1")
    file(GLOB DEMO_SOURCES CONFIGURE_DEPENDS
        "${DEMO_DIR}/startup/*.c"
        "${DEMO_DIR}/startup/fonts/*.c"
        "${DEMO_DIR}/startup/images/*.c"
    )
    set(DEMO_INCLUDE_DIRS
        "${DEMO_DIR}/startup"
        "${DEMO_DIR}/startup/fonts"
        "${DEMO_DIR}/startup/images"
    )
elseif(USE_DEMO STREQUAL "2")
    set(DEMO_SOURCES
        ${WIDGET_DEMO2_SOURCES}
    )
    set(DEMO_INCLUDE_DIRS
        "${DEMO_DIR}/widget"
        "${DEMO_DIR}/widget/fonts"
        "${DEMO_DIR}/widget/images"
    )
elseif(USE_DEMO STREQUAL "3")
    file(GLOB DEMO_SOURCES CONFIGURE_DEPENDS
        "${DEMO_DIR}/printer/*.c"
        "${DEMO_DIR}/printer/fonts/*.c"
        "${DEMO_DIR}/printer/images/*.c"
    )
    set(DEMO_INCLUDE_DIRS
        "${DEMO_DIR}/printer"
        "${DEMO_DIR}/printer/fonts"
        "${DEMO_DIR}/printer/images"
    )
elseif(USE_DEMO STREQUAL "4")
    file(GLOB DEMO_SOURCES CONFIGURE_DEPENDS
        "${DEMO_DIR}/layout/*.c"
        "${DEMO_DIR}/widget/fonts/arial_12.c"
        "${DEMO_DIR}/widget/fonts/arial_16.c"
    )
    set(DEMO_INCLUDE_DIRS
        "${DEMO_DIR}/layout"
        "${DEMO_DIR}/widget/fonts"
    )
elseif(USE_DEMO STREQUAL "5")
    file(GLOB DEMO_SOURCES CONFIGURE_DEPENDS
        "${DEMO_DIR}/layout/*.c"
        "${DEMO_DIR}/widget/fonts/arial_12.c"
        "${DEMO_DIR}/widget/fonts/arial_16.c"
    )
    set(DEMO_INCLUDE_DIRS
        "${DEMO_DIR}/layout"
        "${DEMO_DIR}/widget/fonts"
    )
elseif(USE_DEMO STREQUAL "6")
    set(DEMO_SOURCES
        ${WIDGET_DEMO6_SOURCES}
    )
    set(DEMO_INCLUDE_DIRS
        "${DEMO_DIR}/widget"
        "${DEMO_DIR}/widget/fonts"
        "${DEMO_DIR}/widget/images"
    )
endif()

if(ENABLE_TEST)
    enable_testing()
    find_package(Python3 COMPONENTS Interpreter REQUIRED)

    add_executable(ldswitch_internal_test
        "${SDL_EXAMPLE_DIR}/tests/switch/test_ldswitch_internal.c"
        "${REPO_SRC_DIR}/gui/ldSwitchInternal.c"
    )

    target_include_directories(ldswitch_internal_test PRIVATE
        "${REPO_SRC_DIR}/gui"
    )

    target_compile_options(ldswitch_internal_test PRIVATE
        -fdiagnostics-color=always
        -g
        -std=gnu11
    )

    add_test(NAME ldswitch_internal_test COMMAND ldswitch_internal_test)
    set_tests_properties(ldswitch_internal_test PROPERTIES LABELS "sdl;unit")

    add_executable(ldswitch_widget_test
        "${SDL_EXAMPLE_DIR}/tests/switch/test_ldswitch_widget.c"
        "${REPO_SRC_DIR}/gui/ldSwitchInternal.c"
    )

    target_include_directories(ldswitch_widget_test PRIVATE
        "${REPO_SRC_DIR}/gui"
        "${REPO_SRC_DIR}/misc"
        "${REPO_SRC_DIR}/porting"
        "${COMMON_DIR}/Arm-2D"
        "${COMMON_DIR}/Arm-2D/Helper/Include"
        "${COMMON_DIR}/Arm-2D/Library/Include"
        "${COMMON_DIR}/math"
        "${ARM2D_CONTROLS_DIR}"
    )

    target_compile_definitions(ldswitch_widget_test PRIVATE
        RTE_Acceleration_Arm_2D_Alpha_Blending
    )

    target_compile_options(ldswitch_widget_test PRIVATE
        -fdiagnostics-color=always
        -g
        -std=gnu11
    )

    add_test(NAME ldswitch_widget_test COMMAND ldswitch_widget_test)
    set_tests_properties(ldswitch_widget_test PROPERTIES LABELS "sdl;unit")

    add_executable(uiwidget_swipe_logic_test
        "${SDL_EXAMPLE_DIR}/tests/swipe/test_uiwidget_swipe_logic.c"
        "${DEMO_DIR}/widget/uiWidgetSwipeLogic.c"
    )

    target_include_directories(uiwidget_swipe_logic_test PRIVATE
        "${DEMO_DIR}/widget"
    )

    target_compile_options(uiwidget_swipe_logic_test PRIVATE
        -fdiagnostics-color=always
        -g
        -std=gnu11
    )

    add_test(NAME uiwidget_swipe_logic_test COMMAND uiwidget_swipe_logic_test)
    set_tests_properties(uiwidget_swipe_logic_test PROPERTIES LABELS "sdl;unit")

    add_executable(layout_window_test
        "${SDL_EXAMPLE_DIR}/tests/layout/test_layout_window.c"
        ${ARM2D_LIBRARY_SOURCES}
        "${REPO_SRC_DIR}/gui/ldWindowLayoutInternal.c"
        "${REPO_SRC_DIR}/gui/ldBase.c"
        "${REPO_SRC_DIR}/gui/ldSwitch.c"
        "${REPO_SRC_DIR}/gui/ldSwitchInternal.c"
        "${REPO_SRC_DIR}/gui/ldWindow.c"
        "${REPO_SRC_DIR}/porting/ldArm2dUserDrawCircle.c"
        ${MISC_SOURCES}
    )

    target_include_directories(layout_window_test PRIVATE
        "${SDL_EXAMPLE_DIR}/user"
        "${COMMON_DIR}/Arm-2D"
        "${COMMON_DIR}/Arm-2D/Helper/Include"
        "${COMMON_DIR}/Arm-2D/Library/Include"
        "${COMMON_DIR}/Arm-2D/Library/Source"
        "${ARM2D_CONTROLS_DIR}"
        "${COMMON_DIR}/math"
        "${REPO_SRC_DIR}/gui"
        "${REPO_SRC_DIR}/misc"
        "${REPO_SRC_DIR}/porting"
    )

    target_compile_definitions(layout_window_test PRIVATE
        __va_list=va_list
        RTE_Acceleration_Arm_2D_Helper_Disp_Adapter0
        RTE_Acceleration_Arm_2D_Alpha_Blending
        "-D__ARM_2D_USER_APP_CFG_H__=\"ldConfig.h\""
        "-DARM_SECTION(x)="
    )

    target_compile_options(layout_window_test PRIVATE
        -fdiagnostics-color=always
        -g
        -std=gnu11
        -ffunction-sections
        -fdata-sections
        -fno-ms-extensions
        -Wno-macro-redefined
        "-D__ARM_2D_USER_APP_CFG_H__=\"ldConfig.h\""
        "-DARM_SECTION(x)="
    )

    if(APPLE)
        target_link_options(layout_window_test PRIVATE
            -Wl,-dead_strip
        )
    else()
        target_link_options(layout_window_test PRIVATE
            -Wl,--gc-sections
        )
    endif()

    if(NOT WIN32)
        target_link_libraries(layout_window_test PRIVATE m)
    endif()

    add_test(NAME layout_window_test COMMAND layout_window_test)
    set_tests_properties(layout_window_test PROPERTIES LABELS "sdl;unit")

    add_test(
        NAME check_use_demo_0_legacy_widget
        COMMAND "${Python3_EXECUTABLE}" "${SDL_EXAMPLE_DIR}/tests/check_use_demo_0_legacy_widget.py"
    )
    set_tests_properties(check_use_demo_0_legacy_widget PROPERTIES
        WORKING_DIRECTORY "${SDL_EXAMPLE_DIR}"
        LABELS "sdl;contract"
    )

    add_test(
        NAME check_switch_focus_routing
        COMMAND "${Python3_EXECUTABLE}" "${SDL_EXAMPLE_DIR}/tests/check_switch_focus_routing.py"
    )
    set_tests_properties(check_switch_focus_routing PROPERTIES
        WORKING_DIRECTORY "${SDL_EXAMPLE_DIR}"
        LABELS "sdl;contract"
    )

    add_test(
        NAME check_use_demo_5_grid_widget
        COMMAND "${Python3_EXECUTABLE}" "${SDL_EXAMPLE_DIR}/tests/check_use_demo_5_grid_widget.py"
    )
    set_tests_properties(check_use_demo_5_grid_widget PROPERTIES
        WORKING_DIRECTORY "${SDL_EXAMPLE_DIR}"
        LABELS "sdl;contract"
    )

    add_test(
        NAME check_use_demo_6_widget_swipe
        COMMAND "${Python3_EXECUTABLE}" "${SDL_EXAMPLE_DIR}/tests/check_use_demo_6_widget_swipe.py"
    )
    set_tests_properties(check_use_demo_6_widget_swipe PROPERTIES
        WORKING_DIRECTORY "${SDL_EXAMPLE_DIR}"
        LABELS "sdl;contract"
    )

    add_test(
        NAME check_use_demo_0_runtime
        COMMAND "${Python3_EXECUTABLE}" "${SDL_EXAMPLE_DIR}/tests/check_use_demo_runtime.py"
                --demo 0
                --build-dir "${CMAKE_BINARY_DIR}/runtime-demo-0"
    )
    set_tests_properties(check_use_demo_0_runtime PROPERTIES
        WORKING_DIRECTORY "${SDL_EXAMPLE_DIR}"
        LABELS "sdl;runtime"
    )

    add_test(
        NAME check_use_demo_6_runtime
        COMMAND "${Python3_EXECUTABLE}" "${SDL_EXAMPLE_DIR}/tests/check_use_demo_runtime.py"
                --demo 6
                --build-dir "${CMAKE_BINARY_DIR}/runtime-demo-6"
    )
    set_tests_properties(check_use_demo_6_runtime PROPERTIES
        WORKING_DIRECTORY "${SDL_EXAMPLE_DIR}"
        LABELS "sdl;runtime"
    )

    add_test(
        NAME check_switch_capture_matrix
        COMMAND "${Python3_EXECUTABLE}" "${SDL_EXAMPLE_DIR}/tests/check_switch_capture_matrix.py"
                --build-dir "${CMAKE_BINARY_DIR}/switch-capture-demo-0"
    )
    set_tests_properties(check_switch_capture_matrix PROPERTIES
        WORKING_DIRECTORY "${SDL_EXAMPLE_DIR}"
        LABELS "sdl;runtime"
    )
endif()

add_executable(ldgui_sdl_demo
    ${SDL_DEMO_CORE_SOURCES}
    ${ARM2D_CONTROL_SOURCES}
    ${ARM2D_QRCODE_SOURCES}
    ${ARM2D_LIBRARY_SOURCES}
    ${ARM2D_HELPER_SOURCES}
    ${MATH_SOURCES}
    ${GUI_SOURCES}
    ${MISC_SOURCES}
    "${REPO_SRC_DIR}/porting/ldArm2dUserDrawCircle.c"
    ${DEMO_SOURCES}
)

target_include_directories(ldgui_sdl_demo PRIVATE
    "${SDL_EXAMPLE_DIR}/user"
    "${SDL_EXAMPLE_DIR}/virtualNor"
    "${COMMON_DIR}/Arm-2D"
    "${COMMON_DIR}/Arm-2D/Helper/Include"
    "${COMMON_DIR}/Arm-2D/Library/Include"
    "${COMMON_DIR}/Arm-2D/Library/Source"
    "${ARM2D_CONTROLS_DIR}"
    "${ARM2D_QRCODE_DIR}"
    "${COMMON_DIR}/math"
    "${REPO_SRC_DIR}/gui"
    "${REPO_SRC_DIR}/misc"
    "${REPO_SRC_DIR}/porting"
    ${DEMO_INCLUDE_DIRS}
)

target_compile_definitions(ldgui_sdl_demo PRIVATE
    USE_DEMO=${USE_DEMO}
    __va_list=va_list
    RTE_Acceleration_Arm_2D_Helper_Disp_Adapter0
    RTE_Acceleration_Arm_2D_Alpha_Blending
)

target_compile_options(ldgui_sdl_demo PRIVATE
    -fdiagnostics-color=always
    -w
    -g
    -std=gnu11
    -MMD
    -ffunction-sections
    -fdata-sections
    -fno-ms-extensions
    -Wno-macro-redefined
    -Ofast
    -flto
    "-D__ARM_2D_USER_APP_CFG_H__=\"ldConfig.h\""
    "-DARM_SECTION(x)="
)

target_link_options(ldgui_sdl_demo PRIVATE
    -Ofast
    -flto
)

find_package(Threads REQUIRED)

if(WIN32)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(SDL2_ROOT "${SDL_EXAMPLE_DIR}/sdl2/64")
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
        set(SDL2_ROOT "${SDL_EXAMPLE_DIR}/sdl2/32")
    else()
        message(FATAL_ERROR "Unsupported pointer size: ${CMAKE_SIZEOF_VOID_P}")
    endif()

    target_include_directories(ldgui_sdl_demo PRIVATE "${SDL2_ROOT}/include/SDL2")
    target_link_directories(ldgui_sdl_demo PRIVATE "${SDL2_ROOT}/lib")
    target_link_libraries(ldgui_sdl_demo PRIVATE SDL2 SDL2main)
    if(TARGET Threads::Threads)
        target_link_libraries(ldgui_sdl_demo PRIVATE Threads::Threads)
    endif()

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
    if(TARGET Threads::Threads)
        target_link_libraries(ldgui_sdl_demo PRIVATE Threads::Threads)
    endif()
endif()

function(add_tinyui_demo target source)
    add_executable(${target} ${source} ${ARGN})
    target_include_directories(${target} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../tinyui/demo")
    # Runtime demos already get host-facing port hooks from
    # tinyui_backend_ldgui_runtime -> tinyui_backend_ldgui_porting.
    # Pulling in longdonggui_porting_default again duplicates Disp0_DrawBitmap /
    # touch / timestamp symbols from src/porting/ldConfig.c.
    if(CMAKE_C_COMPILER_ID MATCHES "GNU")
        target_link_libraries(${target} PRIVATE
            -Wl,--start-group
            tinyui_backend_ldgui_runtime
            tinyui_backend_ldgui_porting
            tinyui_port_sdl
            tinyui_core
            longdonggui
            longdonggui_arm2d
            -Wl,--end-group
        )
    elseif(CMAKE_C_COMPILER_ID MATCHES "Clang" AND NOT APPLE)
        target_link_libraries(${target} PRIVATE
            -Wl,--start-group
            tinyui_backend_ldgui_runtime
            tinyui_backend_ldgui_porting
            tinyui_port_sdl
            tinyui_core
            longdonggui
            longdonggui_arm2d
            -Wl,--end-group
        )
    else()
        target_link_libraries(${target} PRIVATE tinyui_backend_ldgui_runtime)
    endif()
    if(COMMAND ld_apply_tinyui_runtime_screen_config)
        ld_apply_tinyui_runtime_screen_config(${target})
    endif()
    ld_apply_common_target_config(${target})
    if(APPLE AND CMAKE_C_COMPILER_ID MATCHES "Clang")
        target_link_options(${target} PRIVATE
            "-Wl,-object_path_lto,${target}.lto.o"
        )
    endif()
endfunction()


# Single tinyui_demo binary — all demos compiled in, select at runtime.
# Usage: ./tinyui_demo <demo_name>   (e.g. ./tinyui_demo arc_basic)
#        ./tinyui_demo               (no args: runs first demo, animation_basic)
set(TINYUI_DEMO_DIR "${SDL_EXAMPLE_DIR}/../../tinyui/demo")
add_tinyui_demo(tinyui_demo
    "${SDL_EXAMPLE_DIR}/../../tinyui_demo/main.c"
    "${SDL_EXAMPLE_DIR}/../../tinyui/port/sdl/tinyui_demo_runtime_stubs.c"
    "${TINYUI_DEMO_DIR}/tinyui_demos.c"
    "${TINYUI_DEMO_DIR}/animation_basic/animation_basic.c"
    "${TINYUI_DEMO_DIR}/arc_basic/arc_basic.c"
    "${TINYUI_DEMO_DIR}/basic_widgets/basic_widgets.c"
    "${TINYUI_DEMO_DIR}/calendar_basic/calendar_basic.c"
    "${TINYUI_DEMO_DIR}/clock_basic/clock_basic.c"
    "${TINYUI_DEMO_DIR}/combo_box_basic/combo_box_basic.c"
    "${TINYUI_DEMO_DIR}/date_time_basic/date_time_basic.c"
    "${TINYUI_DEMO_DIR}/gauge_basic/gauge_basic.c"
    "${TINYUI_DEMO_DIR}/graph_basic/graph_basic.c"
    "${TINYUI_DEMO_DIR}/grid_parity/grid_parity.c"
    "${TINYUI_DEMO_DIR}/hello_world/hello_world.c"
    "${TINYUI_DEMO_DIR}/icon_slider_basic/icon_slider_basic.c"
    "${TINYUI_DEMO_DIR}/keyboard_basic/keyboard_basic.c"
    "${TINYUI_DEMO_DIR}/layout_flex/layout_flex.c"
    "${TINYUI_DEMO_DIR}/layout_grid/layout_grid.c"
    "${TINYUI_DEMO_DIR}/layout_parity/layout_parity.c"
    "${TINYUI_DEMO_DIR}/legacy_demo0_parity/legacy_demo0_parity.c"
    "${TINYUI_DEMO_DIR}/legacy_widget_parity/legacy_widget_parity.c"
    "${TINYUI_DEMO_DIR}/line_edit_basic/line_edit_basic.c"
    "${TINYUI_DEMO_DIR}/list_basic/list_basic.c"
    "${TINYUI_DEMO_DIR}/message_box_basic/message_box_basic.c"
    "${TINYUI_DEMO_DIR}/progress_bar_basic/progress_bar_basic.c"
    "${TINYUI_DEMO_DIR}/progress_wheel_basic/progress_wheel_basic.c"
    "${TINYUI_DEMO_DIR}/qrcode_basic/qrcode_basic.c"
    "${TINYUI_DEMO_DIR}/radial_menu_basic/radial_menu_basic.c"
    "${TINYUI_DEMO_DIR}/scroll_selecter_basic/scroll_selecter_basic.c"
    "${TINYUI_DEMO_DIR}/settings_panel/settings_panel.c"
    "${TINYUI_DEMO_DIR}/table_basic/table_basic.c"
    "${TINYUI_DEMO_DIR}/theme_showcase/theme_showcase.c"
)
