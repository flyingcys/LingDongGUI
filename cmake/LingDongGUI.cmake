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
set(LD_ARM2D_ASSET_DIR "${LD_ARM2D_COMMON_DIR}/asset")
set(LD_MATH_DIR "${LD_COMMON_DIR}/math")
set(LD_DEMO_DIR "${LD_COMMON_DIR}/demo")
set(LD_TESTS_DIR "${LD_REPO_ROOT}/tests")

file(GLOB LD_LONGDONGGUI_GUI_SOURCES CONFIGURE_DEPENDS "${LD_GUI_DIR}/*.c")
file(GLOB LD_LONGDONGGUI_MISC_SOURCES CONFIGURE_DEPENDS "${LD_MISC_DIR}/*.c")
file(GLOB LD_ARM2D_LIBRARY_SOURCES CONFIGURE_DEPENDS "${LD_ARM2D_LIBRARY_DIR}/Source/*.c")
file(GLOB LD_ARM2D_HELPER_SOURCES CONFIGURE_DEPENDS "${LD_ARM2D_HELPER_DIR}/Source/*.c")
file(GLOB LD_MATH_SOURCES CONFIGURE_DEPENDS "${LD_MATH_DIR}/*.c")

set(LD_PERF_COUNTER_SOURCES)
if(NOT APPLE)
    list(APPEND LD_PERF_COUNTER_SOURCES
        "${LD_COMMON_DIR}/perf_counter/perf_counter.c"
        "${LD_COMMON_DIR}/perf_counter/perfc_port_default.c"
    )
endif()
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
set(LD_ARM2D_PROGRESS_WHEEL_ASSET_SOURCES
    "${LD_ARM2D_ASSET_DIR}/arm_2d_asset_QuaterArc.c"
    "${LD_ARM2D_ASSET_DIR}/arm_2d_asset_WhiteDotSmall.c"
)
set(LD_ARM2D_CLOCK_ASSET_SOURCES
    "${LD_ARM2D_ASSET_DIR}/arm_2d_asset_pointer_sec.c"
    "${LD_ARM2D_ASSET_DIR}/arm_2d_asset_clockface.c"
)
set(LD_TINYUI_DEFAULT_RESOURCE_SOURCES
    "${LD_DEMO_DIR}/widget/fonts/arial_12.c"
    "${LD_DEMO_DIR}/widget/fonts/arial_16.c"
)
file(GLOB LD_TINYUI_DEFAULT_IMAGE_RESOURCE_SOURCES CONFIGURE_DEPENDS
    "${LD_DEMO_DIR}/widget/images/*.c"
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
    # BUILD_INTERFACE only: install consumers must not inherit backend Arm-2D
    # compile definitions via TinyUI::tinyui / static archive exports.
    target_compile_definitions(${target}
        PUBLIC
            $<BUILD_INTERFACE:RTE_Acceleration_Arm_2D>
            $<BUILD_INTERFACE:__va_list=va_list>
            $<BUILD_INTERFACE:RTE_Acceleration_Arm_2D_Helper_Disp_Adapter0>
            $<BUILD_INTERFACE:RTE_Acceleration_Arm_2D_Alpha_Blending>
            $<BUILD_INTERFACE:RTE_Acceleration_Arm_2D_Transform>
            $<BUILD_INTERFACE:RTE_Acceleration_Arm_2D_Filter>
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
            "-ffile-prefix-map=${CMAKE_BINARY_DIR}=<tinyui-build>"
            "-fdebug-prefix-map=${CMAKE_BINARY_DIR}=<tinyui-build>"
            "-fmacro-prefix-map=${CMAKE_BINARY_DIR}=<tinyui-build>"
            "-D__ARM_2D_USER_APP_CFG_H__=\"ldConfig.h\""
            "-DARM_SECTION(x)="
    )

    # Clang -flto emits LLVM bitcode archives that install consumers and GNU ld
    # cannot link. Sanitizer profiles therefore force plain objects + GC of
    # unused sections (host demos still need asset completeness, not LTO DCE).
    set(_ld_sanitizer_build FALSE)
    if(CMAKE_C_FLAGS MATCHES "-fsanitize=" OR CMAKE_CXX_FLAGS MATCHES "-fsanitize="
       OR CMAKE_EXE_LINKER_FLAGS MATCHES "-fsanitize="
       OR CMAKE_SHARED_LINKER_FLAGS MATCHES "-fsanitize=")
        set(_ld_sanitizer_build TRUE)
    endif()

    if(LD_ENABLE_COVERAGE AND CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${target} PRIVATE -O0 --coverage)
        target_link_options(${target} PRIVATE --coverage)
    elseif(TINYUI_PROFILE STREQUAL "minimal" OR _ld_sanitizer_build)
        # Minimal / sanitizer: plain objects for nm/map/install consumers.
        target_compile_options(${target} PRIVATE -Ofast -fno-lto)
        target_link_options(${target} PRIVATE -Ofast -fno-lto -Wl,--gc-sections)
    else()
        target_compile_options(${target} PRIVATE -Ofast -flto)
        target_link_options(${target} PRIVATE -Ofast -flto)
    endif()
endfunction()

function(ld_apply_tinyui_runtime_screen_config target)
    set(tinyui_runtime_screen_width 480)
    set(tinyui_runtime_screen_height 320)

    get_target_property(existing_width ${target} TINYUI_RUNTIME_SCREEN_WIDTH)
    if(existing_width)
        set(tinyui_runtime_screen_width ${existing_width})
    endif()

    get_target_property(existing_height ${target} TINYUI_RUNTIME_SCREEN_HEIGHT)
    if(existing_height)
        set(tinyui_runtime_screen_height ${existing_height})
    endif()

    target_compile_definitions(${target}
        PRIVATE
            LD_CFG_SCREEN_WIDTH=${tinyui_runtime_screen_width}
            LD_CFG_SCREEN_HEIGHT=${tinyui_runtime_screen_height}
            LD_CFG_PFB_WIDTH=${tinyui_runtime_screen_width}
    )
endfunction()

function(ld_define_core_targets)
    if(TARGET longdonggui)
        return()
    endif()

    # TinyUI platform port selection. The default (sdl) leaves every existing
    # demo/test bundle unchanged; mcu/none are exercised by dedicated harness
    # targets (see tinyui_port_mcu / mcu_host_smoke), not by rewiring the
    # default bundle.
    set(LD_TINYUI_PORT "sdl" CACHE STRING "TinyUI platform port: sdl | mcu | none")

    # ── TinyUI feature profile / per-widget compile-time trimming ───────────
    set(TINYUI_PROFILE "full" CACHE STRING "TinyUI feature profile: full | minimal")
    set_property(CACHE TINYUI_PROFILE PROPERTY STRINGS full minimal)

    set(TINYUI_WIDGET_FEATURES
        WINDOW BACKGROUND LABEL BUTTON CHECKBOX SWITCH SLIDER TEXT IMAGE
        LINE_EDIT KEYBOARD CANVAS COMBO_BOX SCROLL_SELECTOR TABLE GRAPH
        CALENDAR ARC GAUGE ICON_SLIDER RADIAL_MENU PROGRESS_BAR PROGRESS_WHEEL
        QRCODE ANIMATION DATE_TIME CLOCK LIST MESSAGE_BOX
    )
    set(TINYUI_WIDGET_SOURCE_WINDOW window.c)
    set(TINYUI_WIDGET_SOURCE_BACKGROUND background.c)
    set(TINYUI_WIDGET_SOURCE_LABEL label.c)
    set(TINYUI_WIDGET_SOURCE_BUTTON button.c)
    set(TINYUI_WIDGET_SOURCE_CHECKBOX checkbox.c)
    set(TINYUI_WIDGET_SOURCE_SWITCH switch.c)
    set(TINYUI_WIDGET_SOURCE_SLIDER slider.c)
    set(TINYUI_WIDGET_SOURCE_TEXT text.c)
    set(TINYUI_WIDGET_SOURCE_IMAGE image.c)
    set(TINYUI_WIDGET_SOURCE_LINE_EDIT line_edit.c)
    set(TINYUI_WIDGET_SOURCE_KEYBOARD keyboard.c)
    set(TINYUI_WIDGET_SOURCE_CANVAS canvas.c)
    set(TINYUI_WIDGET_SOURCE_COMBO_BOX combo_box.c)
    set(TINYUI_WIDGET_SOURCE_SCROLL_SELECTOR scroll_selector.c)
    set(TINYUI_WIDGET_SOURCE_TABLE table.c)
    set(TINYUI_WIDGET_SOURCE_GRAPH graph.c)
    set(TINYUI_WIDGET_SOURCE_CALENDAR calendar.c)
    set(TINYUI_WIDGET_SOURCE_ARC arc.c)
    set(TINYUI_WIDGET_SOURCE_GAUGE gauge.c)
    set(TINYUI_WIDGET_SOURCE_ICON_SLIDER icon_slider.c)
    set(TINYUI_WIDGET_SOURCE_RADIAL_MENU radial_menu.c)
    set(TINYUI_WIDGET_SOURCE_PROGRESS_BAR progress_bar.c)
    set(TINYUI_WIDGET_SOURCE_PROGRESS_WHEEL progress_wheel.c)
    set(TINYUI_WIDGET_SOURCE_QRCODE qrcode.c)
    set(TINYUI_WIDGET_SOURCE_ANIMATION animation.c)
    set(TINYUI_WIDGET_SOURCE_DATE_TIME date_time.c)
    set(TINYUI_WIDGET_SOURCE_CLOCK clock.c)
    set(TINYUI_WIDGET_SOURCE_LIST list.c)
    set(TINYUI_WIDGET_SOURCE_MESSAGE_BOX message_box.c)

    if(TINYUI_PROFILE STREQUAL "minimal")
        set(_tinyui_minimal_widgets WINDOW BACKGROUND LABEL BUTTON)
        foreach(_feature IN LISTS TINYUI_WIDGET_FEATURES)
            set(_default OFF)
            if(_feature IN_LIST _tinyui_minimal_widgets)
                set(_default ON)
            endif()
            set(TINYUI_ENABLE_${_feature} ${_default} CACHE BOOL "Enable TinyUI widget ${_feature}" FORCE)
        endforeach()
        set(TINYUI_ENABLE_THEME OFF CACHE BOOL "Enable TinyUI theme module" FORCE)
        set(TINYUI_ENABLE_DIAGNOSTICS OFF CACHE BOOL "Enable TinyUI diagnostics module" FORCE)
        set(TINYUI_ENABLE_NATIVE_INTEROP OFF CACHE BOOL "Enable TinyUI native interop module" FORCE)
    else()
        foreach(_feature IN LISTS TINYUI_WIDGET_FEATURES)
            option(TINYUI_ENABLE_${_feature} "Enable TinyUI widget ${_feature}" ON)
        endforeach()
        option(TINYUI_ENABLE_THEME "Enable TinyUI theme module" ON)
        option(TINYUI_ENABLE_DIAGNOSTICS "Enable TinyUI diagnostics module" ON)
        option(TINYUI_ENABLE_NATIVE_INTEROP "Enable TinyUI native interop module" ON)
    endif()
    set(TINYUI_PROFILE "${TINYUI_PROFILE}" PARENT_SCOPE)

    set(TINYUI_GENERATED_INCLUDE_DIR "${CMAKE_BINARY_DIR}/generated/tinyui")
    file(MAKE_DIRECTORY "${TINYUI_GENERATED_INCLUDE_DIR}")
    configure_file(
        "${LD_REPO_ROOT}/tinyui/include/tinyui_config.h.in"
        "${TINYUI_GENERATED_INCLUDE_DIR}/tinyui_config.h"
        @ONLY
    )

    add_library(longdonggui_arm2d STATIC
        ${LD_ARM2D_LIBRARY_SOURCES}
        ${LD_ARM2D_HELPER_SOURCES}
        ${LD_ARM2D_CONTROL_SOURCES}
        ${LD_ARM2D_PROGRESS_WHEEL_ASSET_SOURCES}
        ${LD_ARM2D_CLOCK_ASSET_SOURCES}
        ${LD_ARM2D_QRCODE_SOURCES}
        ${LD_MATH_SOURCES}
        "${LD_PORTING_DIR}/ldArm2dUserDrawCircle.c"
    )
    # BUILD_INTERFACE only: install/export must not leak Arm-2D/LingDongGUI headers.
    foreach(_inc IN LISTS LD_COMMON_INCLUDE_DIRS)
        target_include_directories(longdonggui_arm2d PUBLIC $<BUILD_INTERFACE:${_inc}>)
    endforeach()
    ld_apply_common_target_config(longdonggui_arm2d)

    add_library(longdonggui STATIC
        ${LD_LONGDONGGUI_GUI_SOURCES}
        ${LD_LONGDONGGUI_MISC_SOURCES}
    )
    foreach(_inc IN LISTS LD_COMMON_INCLUDE_DIRS)
        target_include_directories(longdonggui PUBLIC $<BUILD_INTERFACE:${_inc}>)
    endforeach()
    target_link_libraries(longdonggui PUBLIC longdonggui_arm2d)
    ld_apply_common_target_config(longdonggui)

    add_library(longdonggui_porting_default STATIC
        "${LD_PORTING_DIR}/ldConfig.c"
        "${LD_PORTING_DIR}/arm_2d_disp_adapter_0.c"
        ${LD_PERF_COUNTER_SOURCES}
    )
    foreach(_inc IN LISTS LD_COMMON_INCLUDE_DIRS)
        target_include_directories(longdonggui_porting_default PUBLIC $<BUILD_INTERFACE:${_inc}>)
    endforeach()
    target_link_libraries(longdonggui_porting_default PUBLIC longdonggui)
    ld_apply_common_target_config(longdonggui_porting_default)

    set(LD_TINYUI_BACKEND_LDGUI_DIR "${LD_REPO_ROOT}/tinyui/src/drivers")

    add_library(tinyui_backend_ldgui_porting STATIC
        "${LD_TINYUI_BACKEND_LDGUI_DIR}/tinyui_ldgui_port.c"
        "${LD_TINYUI_BACKEND_LDGUI_DIR}/tinyui_ldgui_disp_adapter.c"
        "${LD_TINYUI_BACKEND_LDGUI_DIR}/tinyui_ldgui_neutral_runtime.c"
        ${LD_PERF_COUNTER_SOURCES}
    )
    foreach(_inc IN ITEMS
        "${LD_TINYUI_BACKEND_LDGUI_DIR}"
        "${LD_REPO_ROOT}/tinyui/include"
        "${TINYUI_GENERATED_INCLUDE_DIR}"
        "${LD_REPO_ROOT}/tinyui/src/core"
        "${LD_REPO_ROOT}/tinyui/src/drivers"
    )
        target_include_directories(tinyui_backend_ldgui_porting PUBLIC $<BUILD_INTERFACE:${_inc}>)
    endforeach()
    foreach(_inc IN LISTS LD_COMMON_INCLUDE_DIRS)
        target_include_directories(tinyui_backend_ldgui_porting PUBLIC $<BUILD_INTERFACE:${_inc}>)
    endforeach()
    target_compile_definitions(tinyui_backend_ldgui_porting PRIVATE
        __ARM_2D_USER_APP_CFG_H__="tinyui_ldgui_port_config.h"
    )
    target_link_libraries(tinyui_backend_ldgui_porting PUBLIC longdonggui tinyui_core)
    ld_apply_common_target_config(tinyui_backend_ldgui_porting)

    add_library(longdonggui_host STATIC
        ${LD_LONGDONGGUI_GUI_SOURCES}
        ${LD_LONGDONGGUI_MISC_SOURCES}
    )
    foreach(_inc IN LISTS LD_COMMON_INCLUDE_DIRS)
        target_include_directories(longdonggui_host PUBLIC $<BUILD_INTERFACE:${_inc}>)
    endforeach()
    target_link_libraries(longdonggui_host PUBLIC longdonggui_arm2d)
    ld_apply_common_target_config(longdonggui_host)

    set(_tinyui_core_sources
        ${LD_REPO_ROOT}/tinyui/src/core/app.c
        ${LD_REPO_ROOT}/tinyui/src/core/native.c
        ${LD_REPO_ROOT}/tinyui/src/core/runtime.c
        ${LD_REPO_ROOT}/tinyui/src/core/runtime_bridge.c
        ${LD_REPO_ROOT}/tinyui/src/core/widget.c
        ${LD_REPO_ROOT}/tinyui/src/core/widget_registry.c
        ${LD_REPO_ROOT}/tinyui/src/core/event.c
        ${LD_REPO_ROOT}/tinyui/src/core/resource.c
        ${LD_REPO_ROOT}/tinyui/src/display/display.c
        ${LD_REPO_ROOT}/tinyui/src/indev/indev.c
        ${LD_REPO_ROOT}/tinyui/src/tick/tick.c
        ${LD_REPO_ROOT}/tinyui/src/osal/osal.c
        ${LD_REPO_ROOT}/tinyui/src/layout/flex.c
        ${LD_REPO_ROOT}/tinyui/src/layout/grid.c
        ${LD_TINYUI_DEFAULT_RESOURCE_SOURCES}
        ${LD_TINYUI_DEFAULT_IMAGE_RESOURCE_SOURCES}
    )
    if(TINYUI_ENABLE_THEME)
        list(APPEND _tinyui_core_sources ${LD_REPO_ROOT}/tinyui/src/theme/theme.c)
    endif()
    foreach(_feature IN LISTS TINYUI_WIDGET_FEATURES)
        if(TINYUI_ENABLE_${_feature})
            list(APPEND _tinyui_core_sources
                ${LD_REPO_ROOT}/tinyui/src/widgets/${TINYUI_WIDGET_SOURCE_${_feature}})
        endif()
    endforeach()

    add_library(tinyui_core STATIC ${_tinyui_core_sources})
    # Public headers for in-tree builds only. Install consumers get headers solely
    # via TinyUI::tinyui INSTALL_INTERFACE (include/), never via tinyui_core.
    foreach(_inc IN ITEMS
        ${LD_REPO_ROOT}/tinyui/include
        ${TINYUI_GENERATED_INCLUDE_DIR}
        ${LD_REPO_ROOT}/tinyui/src/core
        ${LD_REPO_ROOT}/tinyui/src/drivers
    )
        target_include_directories(tinyui_core PUBLIC $<BUILD_INTERFACE:${_inc}>)
    endforeach()
    foreach(_inc IN LISTS LD_COMMON_INCLUDE_DIRS)
        target_include_directories(tinyui_core PUBLIC $<BUILD_INTERFACE:${_inc}>)
    endforeach()
    # Diagnostics flag is also emitted into generated tinyui_config.h; keep the
    # target define BUILD-only so install exports do not re-export it.
    if(TINYUI_ENABLE_DIAGNOSTICS)
        target_compile_definitions(tinyui_core PUBLIC $<BUILD_INTERFACE:TINYUI_ENABLE_DIAGNOSTICS=1>)
    else()
        target_compile_definitions(tinyui_core PUBLIC $<BUILD_INTERFACE:TINYUI_ENABLE_DIAGNOSTICS=0>)
    endif()
    ld_apply_common_target_config(tinyui_core)
    if(TINYUI_PROFILE STREQUAL "minimal")
        # Minimal profile artifacts are inspected with nm/map; keep object code
        # non-LTO so -fno-lto consumers can link and enumerate symbols.
        target_compile_options(tinyui_core PRIVATE -fno-lto)
        target_link_options(tinyui_core PRIVATE -fno-lto)
    endif()

    add_library(tinyui_port_sdl STATIC
        ${LD_REPO_ROOT}/tinyui/port/sdl/hal.c
    )
    # 测试观测脚手架(markers/capture/auto-quit)仅在 ENABLE_TEST 下编入,
    # 经 tinyui_sdl_observe.h 的宏被 hal.c 调用;生产构建 port 零测试符号。
    if(ENABLE_TEST)
        target_sources(tinyui_port_sdl PRIVATE
            ${LD_REPO_ROOT}/tests/tinyui/runtime/tinyui_sdl_observe.c)
        target_compile_definitions(tinyui_port_sdl PRIVATE ENABLE_TEST)
    endif()
    target_include_directories(tinyui_port_sdl PUBLIC
        ${LD_REPO_ROOT}/tinyui/include
        ${TINYUI_GENERATED_INCLUDE_DIR}
        ${LD_REPO_ROOT}/tinyui/src/core
        ${LD_REPO_ROOT}/tinyui
        ${LD_REPO_ROOT}/tinyui/port/sdl
    )
    target_link_libraries(tinyui_port_sdl PUBLIC tinyui_core)
    if(WIN32)
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(LD_SDL2_ROOT "${LD_EXAMPLES_DIR}/sdl/sdl2/64")
        else()
            set(LD_SDL2_ROOT "${LD_EXAMPLES_DIR}/sdl/sdl2/32")
        endif()
        target_include_directories(tinyui_port_sdl PUBLIC "${LD_SDL2_ROOT}/include/SDL2")
        target_link_directories(tinyui_port_sdl PUBLIC "${LD_SDL2_ROOT}/lib")
        target_link_libraries(tinyui_port_sdl PUBLIC SDL2 SDL2main)
    else()
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(SDL2 REQUIRED sdl2)
        target_include_directories(tinyui_port_sdl PUBLIC ${SDL2_INCLUDE_DIRS})
        target_link_directories(tinyui_port_sdl PUBLIC ${SDL2_LIBRARY_DIRS})
        target_compile_options(tinyui_port_sdl PRIVATE ${SDL2_CFLAGS_OTHER})
        target_link_options(tinyui_port_sdl PRIVATE ${SDL2_LDFLAGS_OTHER})
        target_link_libraries(tinyui_port_sdl PUBLIC ${SDL2_LIBRARIES})
    endif()
    ld_apply_tinyui_runtime_screen_config(tinyui_port_sdl)
    ld_apply_common_target_config(tinyui_port_sdl)

    # ── No-SDL bundle ─────────────────────────────────────────────────────
    # Backend-neutral consumer bundle: everything needed to run TinyUI with a
    # platform port that is NOT SDL. Consumers add their own port (e.g.
    # tinyui_port_mcu) to satisfy the frame-driver link contract.
    add_library(tinyui_backend_ldgui_core INTERFACE)
    target_link_libraries(tinyui_backend_ldgui_core INTERFACE
        tinyui_core
        longdonggui
        tinyui_backend_ldgui_porting
    )
    target_include_directories(tinyui_backend_ldgui_core INTERFACE
        ${LD_REPO_ROOT}/tinyui/include
        ${TINYUI_GENERATED_INCLUDE_DIR}
        ${LD_REPO_ROOT}/tinyui/src/core
        ${LD_REPO_ROOT}/tinyui/src/drivers
        ${LD_REPO_ROOT}/tinyui
    )

    # ── MCU (no-SDL) platform port ────────────────────────────────────────
    # Provides tinyui_runtime_host_step_app / _shutdown_app by forwarding to
    # the backend-neutral runtime loop. Contains no SDL and no hardware access,
    # so it compiles for any target.
    add_library(tinyui_port_mcu STATIC
        ${LD_REPO_ROOT}/tinyui/port/mcu/tinyui_port_mcu.c
        ${LD_REPO_ROOT}/tinyui/port/mcu/Retarget.c)
    target_include_directories(tinyui_port_mcu PUBLIC
        ${LD_REPO_ROOT}/tinyui/include
        ${TINYUI_GENERATED_INCLUDE_DIR}
        ${LD_REPO_ROOT}/tinyui/src/core
        ${LD_REPO_ROOT}/tinyui/src/drivers
        ${LD_REPO_ROOT}/tinyui)
    target_link_libraries(tinyui_port_mcu PUBLIC tinyui_core)
    ld_apply_common_target_config(tinyui_port_mcu)

    # Select the platform port linked into the runtime bundle.
    # Default "sdl" keeps every existing consumer byte-for-byte unchanged.
    # For "mcu"/"none" builds also pass -DLD_BUILD_SDL_DEMO=OFF (the SDL demos
    # require SDL); "none" links no port, so the consumer must add one itself
    # (e.g. its own tinyui_port_mcu) to satisfy the frame-driver link contract.
    if(LD_TINYUI_PORT STREQUAL "mcu")
        set(LD_TINYUI_SELECTED_PORT tinyui_port_mcu)
    elseif(LD_TINYUI_PORT STREQUAL "none")
        set(LD_TINYUI_SELECTED_PORT "")
    else()
        set(LD_TINYUI_SELECTED_PORT tinyui_port_sdl)
    endif()

    foreach(LD_TINYUI_BACKEND_TARGET IN ITEMS tinyui_backend_ldgui tinyui_backend_ldgui_runtime)
        add_library(${LD_TINYUI_BACKEND_TARGET} INTERFACE)
        foreach(_inc IN ITEMS
            ${LD_REPO_ROOT}/tinyui/include
            ${TINYUI_GENERATED_INCLUDE_DIR}
            ${LD_REPO_ROOT}/tinyui/src/core
            ${LD_REPO_ROOT}/tinyui/src/drivers
            ${LD_REPO_ROOT}/tinyui
        )
            target_include_directories(${LD_TINYUI_BACKEND_TARGET} INTERFACE
                $<BUILD_INTERFACE:${_inc}>)
        endforeach()
        target_link_libraries(${LD_TINYUI_BACKEND_TARGET} INTERFACE
            tinyui_core
            longdonggui
            tinyui_backend_ldgui_porting
            ${LD_TINYUI_SELECTED_PORT}
        )
    endforeach()

    # ── Install/export package target (M4 Task 6) ─────────────────────────
    # TinyUI::tinyui is the sole user-facing target. Its INTERFACE includes are
    # public headers only; backend/static archives are linked without exporting
    # their private include directories.
    add_library(tinyui INTERFACE)
    add_library(TinyUI::tinyui ALIAS tinyui)
    target_include_directories(tinyui INTERFACE
        $<BUILD_INTERFACE:${LD_REPO_ROOT}/tinyui/include>
        $<BUILD_INTERFACE:${TINYUI_GENERATED_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:include>
    )
    # Link order: tinyui_core before longdonggui (which pulls longdonggui_arm2d).
    # Enough to resolve image_source/font consumer symbols without SDL port.
    target_link_libraries(tinyui INTERFACE
        tinyui_core
        longdonggui
    )

    set(TINYUI_GENERATED_INCLUDE_DIR "${TINYUI_GENERATED_INCLUDE_DIR}" PARENT_SCOPE)
    set(TINYUI_PACKAGE_VERSION "2.3.0" PARENT_SCOPE)

endfunction()

# Install public headers, static archives, and TinyUIConfig package files.
# Call after ld_define_core_targets() from the top-level CMakeLists.txt.
function(ld_install_tinyui_package)
    if(NOT TARGET tinyui OR NOT TARGET tinyui_core OR NOT TARGET longdonggui OR NOT TARGET longdonggui_arm2d)
        message(FATAL_ERROR "ld_install_tinyui_package: core targets missing; call ld_define_core_targets() first")
    endif()
    if(NOT DEFINED TINYUI_GENERATED_INCLUDE_DIR)
        set(TINYUI_GENERATED_INCLUDE_DIR "${CMAKE_BINARY_DIR}/generated/tinyui")
    endif()
    if(NOT DEFINED TINYUI_PACKAGE_VERSION)
        set(TINYUI_PACKAGE_VERSION "2.3.0")
    endif()

    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)

    # Headers: only the public contract tree (no internal/port/src/LingDongGUI/Arm-2D).
    install(FILES
        "${LD_REPO_ROOT}/tinyui/include/tinyui.h"
        "${TINYUI_GENERATED_INCLUDE_DIR}/tinyui_config.h"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    )
    install(DIRECTORY "${LD_REPO_ROOT}/tinyui/include/core/"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/core"
        FILES_MATCHING PATTERN "*.h"
    )
    install(DIRECTORY "${LD_REPO_ROOT}/tinyui/include/widgets/"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/widgets"
        FILES_MATCHING PATTERN "*.h"
    )
    install(DIRECTORY "${LD_REPO_ROOT}/tinyui/include/layout/"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/layout"
        FILES_MATCHING PATTERN "*.h"
    )
    install(DIRECTORY "${LD_REPO_ROOT}/tinyui/include/theme/"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/theme"
        FILES_MATCHING PATTERN "*.h"
    )
    install(DIRECTORY "${LD_REPO_ROOT}/tinyui/include/style/"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/style"
        FILES_MATCHING PATTERN "*.h"
    )
    install(DIRECTORY "${LD_REPO_ROOT}/tinyui/include/resource/"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/resource"
        FILES_MATCHING PATTERN "*.h"
    )
    install(FILES "${LD_REPO_ROOT}/tinyui/include/integration/input.h"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/integration"
    )

    # Libraries: implementation archives + user-facing INTERFACE target.
    # Do NOT attach INCLUDES DESTINATION to backend archives.
    install(TARGETS tinyui
        EXPORT TinyUITargets
    )
    install(TARGETS tinyui_core longdonggui longdonggui_arm2d
        EXPORT TinyUITargets
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    )

    set(_tinyui_cmake_install_dir "${CMAKE_INSTALL_LIBDIR}/cmake/TinyUI")

    configure_package_config_file(
        "${LD_REPO_ROOT}/cmake/TinyUIConfig.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/TinyUIConfig.cmake"
        INSTALL_DESTINATION "${_tinyui_cmake_install_dir}"
    )
    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/TinyUIConfigVersion.cmake"
        VERSION "${TINYUI_PACKAGE_VERSION}"
        COMPATIBILITY SameMajorVersion
    )

    install(EXPORT TinyUITargets
        NAMESPACE TinyUI::
        DESTINATION "${_tinyui_cmake_install_dir}"
    )
    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/TinyUIConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/TinyUIConfigVersion.cmake"
        DESTINATION "${_tinyui_cmake_install_dir}"
    )
endfunction()

function(ld_add_c_unit_test target)
    cmake_parse_arguments(LDTEST "" "SUPPORT_LIB;MAIN_LIB;TEST_NAME" "SOURCES;LABELS" ${ARGN})
    add_executable(${target} ${LDTEST_SOURCES})
    if(CMAKE_C_COMPILER_ID MATCHES "GNU" OR (CMAKE_C_COMPILER_ID MATCHES "Clang" AND NOT APPLE))
        target_link_libraries(${target} PRIVATE
            -Wl,--start-group
            ${LDTEST_SUPPORT_LIB}
            ${LDTEST_MAIN_LIB}
            tinyui_core
            longdonggui
            longdonggui_arm2d
            -Wl,--end-group
        )
    else()
        target_link_libraries(${target} PRIVATE ${LDTEST_SUPPORT_LIB} ${LDTEST_MAIN_LIB})
    endif()
    ld_apply_common_target_config(${target})
    set(LDTEST_CTEST_NAME "${target}")
    if(LDTEST_TEST_NAME)
        set(LDTEST_CTEST_NAME "${LDTEST_TEST_NAME}")
    endif()
    add_test(NAME ${LDTEST_CTEST_NAME} COMMAND ${target})
    if(LDTEST_LABELS)
        set_tests_properties(${LDTEST_CTEST_NAME} PROPERTIES LABELS "${LDTEST_LABELS}")
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
