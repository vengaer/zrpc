# Copyright (c) 2026 Vilhelm Engström
#
# SPDX-License-Identifier: Apache-2.0

add_subdirectory(subsys)

list(APPEND SYSCALL_INCLUDE_DIRS "${ZEPHYR_CURRENT_MODULE_DIR}/include/zephyr/rpc")
set(SYSCALL_INCLUDE_DIRS ${SYSCALL_INCLUDE_DIRS} PARENT_SCOPE)

zephyr_include_directories(${CMAKE_CURRENT_LIST_DIR}/include)
