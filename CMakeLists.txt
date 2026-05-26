cmake_minimum_required(VERSION 3.16)

project(longdonggui LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

include(CTest)

option(LD_BUILD_SDL_DEMO "Build SDL demo targets" ON)
option(LD_BUILD_RUNTIME_TESTS "Register runtime tests" ON)
option(LD_BUILD_VISUAL_TESTS "Register visual tests" OFF)
option(LD_ENABLE_COVERAGE "Enable coverage flags" OFF)

set(USE_DEMO "2" CACHE STRING "SDL demo id")
set_property(CACHE USE_DEMO PROPERTY STRINGS 0 1 2 3 4 5 6)

include(cmake/LingDongGUI.cmake)

ld_define_core_targets()

if(LD_BUILD_SDL_DEMO)
    add_subdirectory(examples/sdl)
endif()

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
