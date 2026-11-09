# SPDX-License-Identifier: MIT
# Apply patches/*.patch to the upstream submodule in an idempotent way.
function(scp_apply_patches)
  set(patch_dir "${CMAKE_SOURCE_DIR}/patches")
  if(NOT EXISTS "${patch_dir}")
    return()
  endif()
  file(GLOB patches "${patch_dir}/*.patch")
  list(SORT patches)
  foreach(p IN LISTS patches)
    execute_process(
      COMMAND git apply --check --reverse "${p}"
      WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/third_party/SpaceCadetPinball"
      RESULT_VARIABLE already
      OUTPUT_QUIET ERROR_QUIET)
    if(NOT already EQUAL 0)
      execute_process(
        COMMAND git apply "${p}"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/third_party/SpaceCadetPinball"
        RESULT_VARIABLE applied)
      if(NOT applied EQUAL 0)
        message(FATAL_ERROR "Failed to apply patch ${p}")
      endif()
      message(STATUS "Applied ${p}")
    else()
      message(STATUS "Already applied ${p}")
    endif()
  endforeach()
endfunction()
