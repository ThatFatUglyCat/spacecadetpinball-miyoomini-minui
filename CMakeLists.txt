# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.16)
project(spacecadetpinball_libretro LANGUAGES CXX VERSION 0.1.0)

option(SCP_ENABLE_MUSIC "Build optional MIDI music support" OFF)
option(SCP_SANITIZE "Enable ASan/UBSan (host only)" OFF)
option(SCP_BUILD_TESTS "Build unit and smoke tests" ON)
set(SCP_LOG_LEVEL "INFO" CACHE STRING "Core log verbosity")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_library(spacecadetpinball_libretro SHARED
  src/libretro/libretro.cpp
  src/libretro/content.cpp
  src/libretro/errorscreen.cpp
  src/libretro/input.cpp
  src/libretro/log.cpp
)

set_target_properties(spacecadetpinball_libretro PROPERTIES
  OUTPUT_NAME "spacecadetpinball_libretro"
  PREFIX ""
)

target_include_directories(spacecadetpinball_libretro
  PRIVATE
    ${CMAKE_SOURCE_DIR}/src/libretro
    ${CMAKE_SOURCE_DIR}/third_party/libretro
)

target_compile_definitions(spacecadetpinball_libretro PRIVATE
  __LIBRETRO__
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(spacecadetpinball_libretro PRIVATE
    -Wall -Wextra -Wpedantic -fvisibility=hidden
  )
  target_link_options(spacecadetpinball_libretro PRIVATE
    "LINKER:--version-script=${CMAKE_SOURCE_DIR}/src/libretro/link.T"
    "LINKER:--gc-sections"
  )
endif()

if(SCP_SANITIZE AND NOT CMAKE_CROSSCOMPILING)
  target_compile_options(spacecadetpinball_libretro PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
  target_link_options(spacecadetpinball_libretro PRIVATE -fsanitize=address,undefined)
endif()

include(${CMAKE_SOURCE_DIR}/cmake/patches.cmake)

if(SCP_BUILD_TESTS AND NOT CMAKE_CROSSCOMPILING)
  enable_testing()
  add_executable(scp_smoke_test tests/smoke/smoke_test.cpp)
  target_include_directories(scp_smoke_test PRIVATE
    ${CMAKE_SOURCE_DIR}/third_party/libretro
    ${CMAKE_SOURCE_DIR}/src/libretro
  )
  target_link_libraries(scp_smoke_test PRIVATE dl)
  add_dependencies(scp_smoke_test spacecadetpinball_libretro)
  add_test(
    NAME smoke
    COMMAND scp_smoke_test $<TARGET_FILE:spacecadetpinball_libretro>
  )
  set_tests_properties(smoke PROPERTIES WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

  add_executable(scp_content_test tests/unit/content_test.cpp src/libretro/content.cpp)
  target_include_directories(scp_content_test PRIVATE
    ${CMAKE_SOURCE_DIR}/src/libretro
    ${CMAKE_SOURCE_DIR}/third_party/libretro
  )
  add_test(NAME content_unit COMMAND scp_content_test)
endif()

install(TARGETS spacecadetpinball_libretro LIBRARY DESTINATION .)
