# Fail-closed driver: install TinyUI package, build & run out-of-tree C/C++ consumers.
#
# Required -D variables:
#   binary_dir   - host project build directory (stage under here)
#   source_dir   - repository root
#   consumer_src - absolute path to tests/tinyui/consumer fixture in the repo

if(NOT DEFINED binary_dir OR binary_dir STREQUAL "")
  message(FATAL_ERROR "run_install_consumer.cmake: binary_dir is required")
endif()
if(NOT DEFINED source_dir OR source_dir STREQUAL "")
  message(FATAL_ERROR "run_install_consumer.cmake: source_dir is required")
endif()
if(NOT DEFINED consumer_src OR consumer_src STREQUAL "")
  message(FATAL_ERROR "run_install_consumer.cmake: consumer_src is required")
endif()

get_filename_component(binary_dir "${binary_dir}" ABSOLUTE)
get_filename_component(source_dir "${source_dir}" ABSOLUTE)
get_filename_component(consumer_src "${consumer_src}" ABSOLUTE)

set(_install_dir "${binary_dir}/_tinyui_install")
set(_consumer_src_dir "${binary_dir}/_tinyui_consumer_src")
set(_consumer_build_dir "${binary_dir}/_tinyui_consumer_build")

message(STATUS "install consumer: binary_dir=${binary_dir}")
message(STATUS "install consumer: install_dir=${_install_dir}")
message(STATUS "install consumer: consumer_src_dir=${_consumer_src_dir}")
message(STATUS "install consumer: consumer_build_dir=${_consumer_build_dir}")

# 1) Wipe previous stage trees under the build directory.
file(REMOVE_RECURSE "${_install_dir}" "${_consumer_src_dir}" "${_consumer_build_dir}")

# 2) Install the host package into the stage prefix.
execute_process(
  COMMAND "${CMAKE_COMMAND}" --install "${binary_dir}" --prefix "${_install_dir}"
  RESULT_VARIABLE _rc
  OUTPUT_VARIABLE _out
  ERROR_VARIABLE _err
)
if(NOT _rc EQUAL 0)
  message(FATAL_ERROR
    "cmake --install failed (rc=${_rc})\nstdout:\n${_out}\nstderr:\n${_err}")
endif()

# 3) Copy consumer fixture out of the live source tree into the build stage.
file(COPY "${consumer_src}/"
  DESTINATION "${_consumer_src_dir}"
  FILES_MATCHING
    PATTERN "CMakeLists.txt"
    PATTERN "main.c"
    PATTERN "main.cpp"
)

if(NOT EXISTS "${_consumer_src_dir}/CMakeLists.txt"
   OR NOT EXISTS "${_consumer_src_dir}/main.c"
   OR NOT EXISTS "${_consumer_src_dir}/main.cpp")
  message(FATAL_ERROR
    "consumer fixture copy incomplete under ${_consumer_src_dir}")
endif()

# 4) Assert staged consumer is under binary_dir and is NOT the in-repo fixture.
#    (binary_dir itself may live under the repo; the gate requires a staged copy,
#     not building against tests/tinyui/consumer in place.)
file(RELATIVE_PATH _rel_to_bin "${binary_dir}" "${_consumer_src_dir}")
if(_rel_to_bin MATCHES "^\\.\\." OR _rel_to_bin STREQUAL "")
  message(FATAL_ERROR
    "consumer source stage must be a subdirectory of binary_dir:\n"
    "  binary_dir=${binary_dir}\n"
    "  consumer_src_dir=${_consumer_src_dir}\n"
    "  relative=${_rel_to_bin}")
endif()

get_filename_component(_fixture_norm "${consumer_src}" ABSOLUTE)
get_filename_component(_stage_norm "${_consumer_src_dir}" ABSOLUTE)
if(_stage_norm STREQUAL _fixture_norm)
  message(FATAL_ERROR
    "consumer must be staged out of the live fixture path:\n"
    "  fixture=${_fixture_norm}\n"
    "  stage=${_stage_norm}")
endif()

# Reject staging back into the source fixture tree (tests/tinyui/consumer/...).
file(RELATIVE_PATH _rel_stage_to_fixture "${_fixture_norm}" "${_stage_norm}")
if(NOT _rel_stage_to_fixture MATCHES "^\\.\\.")
  message(FATAL_ERROR
    "consumer source stage must not live under the in-repo fixture:\n"
    "  fixture=${_fixture_norm}\n"
    "  stage=${_stage_norm}")
endif()

# 5) Configure the staged consumer against the install prefix only.
#    When the host package was built with sanitizers, installed static
#    archives still reference the runtime. Inherit host sanitizer flags so
#    out-of-tree consumers can link (GCC/Clang host linker otherwise fails).
set(_consumer_cmake_args
  "-DCMAKE_PREFIX_PATH=${_install_dir}"
)
if(EXISTS "${binary_dir}/CMakeCache.txt")
  file(STRINGS "${binary_dir}/CMakeCache.txt" _host_cache_lines REGEX
    "^(CMAKE_C_FLAGS|CMAKE_CXX_FLAGS|CMAKE_EXE_LINKER_FLAGS|CMAKE_SHARED_LINKER_FLAGS):")
  foreach(_line IN LISTS _host_cache_lines)
    if(_line MATCHES "^([^:]+):[^=]*=(.*)$")
      set(_key "${CMAKE_MATCH_1}")
      set(_val "${CMAKE_MATCH_2}")
      if(_val MATCHES "-fsanitize=")
        list(APPEND _consumer_cmake_args "-D${_key}=${_val}")
      endif()
    endif()
  endforeach()
endif()

execute_process(
  COMMAND "${CMAKE_COMMAND}"
    -S "${_consumer_src_dir}"
    -B "${_consumer_build_dir}"
    ${_consumer_cmake_args}
  RESULT_VARIABLE _rc
  OUTPUT_VARIABLE _out
  ERROR_VARIABLE _err
)
if(NOT _rc EQUAL 0)
  message(FATAL_ERROR
    "consumer cmake configure failed (rc=${_rc})\nstdout:\n${_out}\nstderr:\n${_err}")
endif()

# 6) Build both executables.
execute_process(
  COMMAND "${CMAKE_COMMAND}" --build "${_consumer_build_dir}" --parallel
  RESULT_VARIABLE _rc
  OUTPUT_VARIABLE _out
  ERROR_VARIABLE _err
)
if(NOT _rc EQUAL 0)
  message(FATAL_ERROR
    "consumer cmake build failed (rc=${_rc})\nstdout:\n${_out}\nstderr:\n${_err}")
endif()

# 7) Run C and C++ consumers; both must exit 0.
foreach(_exe IN ITEMS tinyui_consumer_c tinyui_consumer_cpp)
  set(_path "${_consumer_build_dir}/${_exe}")
  if(NOT EXISTS "${_path}")
    file(GLOB_RECURSE _candidates
      "${_consumer_build_dir}/${_exe}"
      "${_consumer_build_dir}/*/${_exe}")
    list(FILTER _candidates EXCLUDE REGEX "CMakeFiles")
    list(LENGTH _candidates _n)
    if(_n EQUAL 0)
      message(FATAL_ERROR
        "consumer executable not found: ${_exe} under ${_consumer_build_dir}")
    endif()
    list(GET _candidates 0 _path)
  endif()

  execute_process(
    COMMAND "${_path}"
    RESULT_VARIABLE _rc
    OUTPUT_VARIABLE _out
    ERROR_VARIABLE _err
  )
  if(NOT _rc EQUAL 0)
    message(FATAL_ERROR
      "consumer run failed: ${_exe} (rc=${_rc})\nstdout:\n${_out}\nstderr:\n${_err}")
  endif()
  message(STATUS "install consumer: ${_exe} OK (${_path})")
endforeach()

message(STATUS "install consumer: PASS")
