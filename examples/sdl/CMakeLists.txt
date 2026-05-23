cmake_minimum_required(VERSION 3.16)

project(ldgui_sdl_demo LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

set(USE_DEMO "2" CACHE STRING "SDL demo id (1=startup, 2=widget, 3=printer)")
set_property(CACHE USE_DEMO PROPERTY STRINGS 1 2 3)

set(SUPPORTED_DEMOS 1 2 3)
list(FIND SUPPORTED_DEMOS "${USE_DEMO}" USE_DEMO_INDEX)
if(USE_DEMO_INDEX EQUAL -1)
    message(FATAL_ERROR "Unsupported USE_DEMO=${USE_DEMO}. Supported values: 1, 2, 3.")
endif()

set(SDL_EXAMPLE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(EXAMPLES_DIR "${SDL_EXAMPLE_DIR}/..")
set(COMMON_DIR "${EXAMPLES_DIR}/common")
set(REPO_SRC_DIR "${SDL_EXAMPLE_DIR}/../../src")

set(SDL_DEMO_CORE_SOURCES
    "${SDL_EXAMPLE_DIR}/virtualNor/virtualNor.c"
    "${SDL_EXAMPLE_DIR}/user/main.c"
    "${SDL_EXAMPLE_DIR}/user/ldConfig.c"
    "${SDL_EXAMPLE_DIR}/user/Virtual_TFT_Port.c"
    "${SDL_EXAMPLE_DIR}/user/arm_2d_disp_adapter_0.c"
)

file(GLOB ARM2D_CONTROL_SOURCES CONFIGURE_DEPENDS
    "${COMMON_DIR}/Arm-2D/examples/common/controls/*.c"
)
file(GLOB ARM2D_QRCODE_SOURCES CONFIGURE_DEPENDS
    "${COMMON_DIR}/Arm-2D/examples/common/controls/qrcode_box/*.c"
)
file(GLOB ARM2D_ASSET_SOURCES CONFIGURE_DEPENDS
    "${COMMON_DIR}/Arm-2D/examples/common/asset/*.c"
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
file(GLOB STARTUP_DEMO_SOURCES CONFIGURE_DEPENDS
    "${COMMON_DIR}/demo/startup/*.c"
    "${COMMON_DIR}/demo/startup/fonts/*.c"
    "${COMMON_DIR}/demo/startup/images/*.c"
)
file(GLOB WIDGET_DEMO_SOURCES CONFIGURE_DEPENDS
    "${COMMON_DIR}/demo/widget/*.c"
    "${COMMON_DIR}/demo/widget/fonts/*.c"
    "${COMMON_DIR}/demo/widget/images/*.c"
)
file(GLOB PRINTER_DEMO_SOURCES CONFIGURE_DEPENDS
    "${COMMON_DIR}/demo/printer/*.c"
    "${COMMON_DIR}/demo/printer/fonts/*.c"
    "${COMMON_DIR}/demo/printer/images/*.c"
)

add_executable(ldgui_sdl_demo
    ${SDL_DEMO_CORE_SOURCES}
    ${ARM2D_CONTROL_SOURCES}
    ${ARM2D_QRCODE_SOURCES}
    ${ARM2D_ASSET_SOURCES}
    ${ARM2D_LIBRARY_SOURCES}
    ${ARM2D_HELPER_SOURCES}
    ${MATH_SOURCES}
    ${GUI_SOURCES}
    ${MISC_SOURCES}
    ${STARTUP_DEMO_SOURCES}
    ${WIDGET_DEMO_SOURCES}
    ${PRINTER_DEMO_SOURCES}
)

target_include_directories(ldgui_sdl_demo PRIVATE
    "${SDL_EXAMPLE_DIR}/user"
    "${SDL_EXAMPLE_DIR}/virtualNor"
    "${COMMON_DIR}/Arm-2D"
    "${COMMON_DIR}/Arm-2D/Helper/Include"
    "${COMMON_DIR}/Arm-2D/Library/Include"
    "${COMMON_DIR}/Arm-2D/Library/Source"
    "${COMMON_DIR}/Arm-2D/examples/common/controls"
    "${COMMON_DIR}/Arm-2D/examples/common/controls/qrcode_box"
    "${COMMON_DIR}/math"
    "${REPO_SRC_DIR}/gui"
    "${REPO_SRC_DIR}/misc"
    "${COMMON_DIR}/demo/startup"
    "${COMMON_DIR}/demo/startup/fonts"
    "${COMMON_DIR}/demo/startup/images"
    "${COMMON_DIR}/demo/widget"
    "${COMMON_DIR}/demo/widget/fonts"
    "${COMMON_DIR}/demo/widget/images"
    "${COMMON_DIR}/demo/printer"
    "${COMMON_DIR}/demo/printer/fonts"
    "${COMMON_DIR}/demo/printer/images"
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
