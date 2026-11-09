# SPDX-License-Identifier: MIT
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

if(NOT DEFINED ENV{CROSS_COMPILE})
  message(FATAL_ERROR "CROSS_COMPILE is not set; run inside the union-miyoomini-toolchain container (make shell).")
endif()

set(CMAKE_C_COMPILER   "$ENV{CROSS_COMPILE}gcc")
set(CMAKE_CXX_COMPILER "$ENV{CROSS_COMPILE}g++")

# Flags aligned with MinUI miyoomini platform defaults.
set(ARCH_FLAGS "-mcpu=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard")
set(CMAKE_C_FLAGS_INIT   "${ARCH_FLAGS} -fvisibility=hidden -ffunction-sections -fdata-sections")
set(CMAKE_CXX_FLAGS_INIT "${ARCH_FLAGS} -fvisibility=hidden -ffunction-sections -fdata-sections")

set(CMAKE_SHARED_LINKER_FLAGS_INIT
    "-static-libstdc++ -static-libgcc -Wl,--gc-sections -Wl,-z,defs")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
