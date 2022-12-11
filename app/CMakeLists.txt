# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(final_proj)
add_subdirectory(cab)

target_sources(app PRIVATE src/main.c)

#add_executable(app src/main.c)
# Generate variables with set of libs and include folders
# Dependencies are automatically set
list(APPEND EXTRA_LIBS cab)
list(APPEND EXTRA_INCLUDES "${PROJECT_SOURCE_DIR}/cab")
# Specify libraries or flags to use when linking a given target and/or its dependents
target_link_libraries(app PUBLIC ${EXTRA_LIBS})
# Specifies include directories to use when compiling a given target
target_include_directories(app PUBLIC ${EXTRA_INCLUDES})
target_link_libraries(cab PUBLIC zephyr_interface)
