cmake_minimum_required(VERSION 3.16)
project(ldgui_sdl_demo LANGUAGES C)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(LD_BUILD_SDL_DEMO ON CACHE BOOL "Build SDL demo from examples/sdl entrypoint" FORCE)
    set(LD_BUILD_TESTS OFF CACHE BOOL "Do not build host tests from the SDL-only entrypoint" FORCE)
    set(LD_PORTING_DIR "${CMAKE_CURRENT_SOURCE_DIR}/user" CACHE PATH "Porting/config directory used while compiling longdonggui" FORCE)
    set(LD_BUILDING_FROM_SDL_SUBDIR ON CACHE INTERNAL "Avoid recursive add_subdirectory when examples/sdl is the entrypoint")
    add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../.." "${CMAKE_CURRENT_BINARY_DIR}/root")
    return()
endif()

ld_add_sdl_demo_target()
