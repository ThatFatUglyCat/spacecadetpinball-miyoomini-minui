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

set(SCP_UPSTREAM_DIR ${CMAKE_SOURCE_DIR}/third_party/SpaceCadetPinball/SpaceCadetPinball)

include(${CMAKE_SOURCE_DIR}/cmake/patches.cmake)
scp_apply_patches()

set(SCP_UPSTREAM_SOURCES
  ${SCP_UPSTREAM_DIR}/EmbeddedData.cpp
  ${SCP_UPSTREAM_DIR}/GroupData.cpp
  ${SCP_UPSTREAM_DIR}/Sound.cpp
  ${SCP_UPSTREAM_DIR}/TBall.cpp
  ${SCP_UPSTREAM_DIR}/TBlocker.cpp
  ${SCP_UPSTREAM_DIR}/TBumper.cpp
  ${SCP_UPSTREAM_DIR}/TCircle.cpp
  ${SCP_UPSTREAM_DIR}/TCollisionComponent.cpp
  ${SCP_UPSTREAM_DIR}/TComponentGroup.cpp
  ${SCP_UPSTREAM_DIR}/TDemo.cpp
  ${SCP_UPSTREAM_DIR}/TDrain.cpp
  ${SCP_UPSTREAM_DIR}/TEdgeManager.cpp
  ${SCP_UPSTREAM_DIR}/TEdgeSegment.cpp
  ${SCP_UPSTREAM_DIR}/TFlagSpinner.cpp
  ${SCP_UPSTREAM_DIR}/TFlipper.cpp
  ${SCP_UPSTREAM_DIR}/TFlipperEdge.cpp
  ${SCP_UPSTREAM_DIR}/TGate.cpp
  ${SCP_UPSTREAM_DIR}/THole.cpp
  ${SCP_UPSTREAM_DIR}/TKickback.cpp
  ${SCP_UPSTREAM_DIR}/TKickout.cpp
  ${SCP_UPSTREAM_DIR}/TLight.cpp
  ${SCP_UPSTREAM_DIR}/TLightBargraph.cpp
  ${SCP_UPSTREAM_DIR}/TLightGroup.cpp
  ${SCP_UPSTREAM_DIR}/TLightRollover.cpp
  ${SCP_UPSTREAM_DIR}/TLine.cpp
  ${SCP_UPSTREAM_DIR}/TOneway.cpp
  ${SCP_UPSTREAM_DIR}/TPinballComponent.cpp
  ${SCP_UPSTREAM_DIR}/TPinballTable.cpp
  ${SCP_UPSTREAM_DIR}/TPlunger.cpp
  ${SCP_UPSTREAM_DIR}/TPopupTarget.cpp
  ${SCP_UPSTREAM_DIR}/TRamp.cpp
  ${SCP_UPSTREAM_DIR}/TRollover.cpp
  ${SCP_UPSTREAM_DIR}/TSink.cpp
  ${SCP_UPSTREAM_DIR}/TSoloTarget.cpp
  ${SCP_UPSTREAM_DIR}/TSound.cpp
  ${SCP_UPSTREAM_DIR}/TTableLayer.cpp
  ${SCP_UPSTREAM_DIR}/TTextBox.cpp
  ${SCP_UPSTREAM_DIR}/TTextBoxMessage.cpp
  ${SCP_UPSTREAM_DIR}/TTimer.cpp
  ${SCP_UPSTREAM_DIR}/TTripwire.cpp
  ${SCP_UPSTREAM_DIR}/TWall.cpp
  ${SCP_UPSTREAM_DIR}/control.cpp
  ${SCP_UPSTREAM_DIR}/fullscrn.cpp
  ${SCP_UPSTREAM_DIR}/gdrv.cpp
  ${SCP_UPSTREAM_DIR}/high_score.cpp
  ${SCP_UPSTREAM_DIR}/loader.cpp
  ${SCP_UPSTREAM_DIR}/maths.cpp
  ${SCP_UPSTREAM_DIR}/midi.cpp
  ${SCP_UPSTREAM_DIR}/nudge.cpp
  ${SCP_UPSTREAM_DIR}/options.cpp
  ${SCP_UPSTREAM_DIR}/partman.cpp
  ${SCP_UPSTREAM_DIR}/pb.cpp
  ${SCP_UPSTREAM_DIR}/proj.cpp
  ${SCP_UPSTREAM_DIR}/render.cpp
  ${SCP_UPSTREAM_DIR}/score.cpp
  ${SCP_UPSTREAM_DIR}/timer.cpp
  ${SCP_UPSTREAM_DIR}/translations.cpp
  ${SCP_UPSTREAM_DIR}/zdrv.cpp
)

add_library(spacecadetpinball_libretro SHARED
  src/libretro/libretro.cpp
  src/libretro/content.cpp
  src/libretro/errorscreen.cpp
  src/libretro/input.cpp
  src/libretro/log.cpp
  src/libretro/clock.cpp
  src/libretro/game.cpp
  src/libretro/video.cpp
  src/libretro/audio.cpp
  src/shim/sdl_shim.cpp
  src/shim/imgui_shim.cpp
  src/shim/winmain_stub.cpp
  src/shim/debug_overlay_stub.cpp
  ${SCP_UPSTREAM_SOURCES}
)

set_target_properties(spacecadetpinball_libretro PROPERTIES
  OUTPUT_NAME "spacecadetpinball_libretro"
  PREFIX ""
)

target_include_directories(spacecadetpinball_libretro
  PRIVATE
    ${CMAKE_SOURCE_DIR}/include/shim
    ${CMAKE_SOURCE_DIR}/src/libretro
    ${CMAKE_SOURCE_DIR}/third_party/libretro
    ${SCP_UPSTREAM_DIR}
)

target_compile_definitions(spacecadetpinball_libretro PRIVATE
  __LIBRETRO__
  SCP_LIBRETRO=1
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(spacecadetpinball_libretro PRIVATE
    -Wall -Wextra -Wno-unused-parameter -fvisibility=hidden
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

  add_executable(scp_video_test tests/unit/video_test.cpp src/libretro/video.cpp)
  target_include_directories(scp_video_test PRIVATE
    ${CMAKE_SOURCE_DIR}/src/libretro
    ${CMAKE_SOURCE_DIR}/third_party/libretro
  )
  add_test(NAME video_unit COMMAND scp_video_test)

  # Edge translator only: stub game symbols so input.cpp links without the engine.
  add_executable(scp_input_test
    tests/unit/input_test.cpp
    src/libretro/input.cpp
    tests/unit/input_test_game_stubs.cpp
  )
  target_include_directories(scp_input_test PRIVATE
    ${CMAKE_SOURCE_DIR}/src/libretro
    ${CMAKE_SOURCE_DIR}/third_party/libretro
  )
  add_test(NAME input_unit COMMAND scp_input_test)

  add_executable(scp_audio_test
    tests/unit/audio_test.cpp
    src/libretro/audio.cpp
  )
  target_include_directories(scp_audio_test PRIVATE
    ${CMAKE_SOURCE_DIR}/src/libretro
    ${CMAKE_SOURCE_DIR}/include/shim
    ${CMAKE_SOURCE_DIR}/third_party/libretro
  )
  add_test(NAME audio_unit COMMAND scp_audio_test)
endif()

install(TARGETS spacecadetpinball_libretro LIBRARY DESTINATION .)
