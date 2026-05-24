cmake_minimum_required(VERSION 3.16)

project(longdonggui LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

option(LD_BUILD_TESTS "Build host tests under src/gui/test" OFF)
option(LD_BUILD_SDL_DEMO "Build SDL demo from the repository root" ON)
option(LD_ENABLE_COVERAGE "Enable coverage-friendly compiler and linker flags" OFF)

set(USE_DEMO "2" CACHE STRING "SDL demo id (1=startup, 2=widget, 3=printer, 4=layout, 5=grid)")
set_property(CACHE USE_DEMO PROPERTY STRINGS 1 2 3 4 5)

include(cmake/LingDongGUI.cmake)

if(NOT DEFINED LD_PORTING_DIR)
    if(LD_BUILD_TESTS)
        set(LD_PORTING_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/porting" CACHE PATH "Porting/config directory used while compiling longdonggui")
    else()
        set(LD_PORTING_DIR "${CMAKE_CURRENT_SOURCE_DIR}/examples/sdl/user" CACHE PATH "Porting/config directory used while compiling longdonggui")
    endif()
endif()

ld_define_core_targets()

if(LD_BUILD_TESTS)
    enable_testing()
    add_subdirectory(src/gui/test)
endif()

if(LD_BUILD_SDL_DEMO AND NOT LD_BUILDING_FROM_SDL_SUBDIR)
    add_subdirectory(examples/sdl)
endif()
