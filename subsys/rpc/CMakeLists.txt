# Copyright (c) 2026 Vilhelm Engström
#
# SPDX-License-Identifier: Apache-2.0

find_package(Python3 REQUIRED COMPONENTS Interpreter)

zephyr_library()
zephyr_library_property(ALLOW_EMPTY TRUE)

zephyr_library_sources(zrpc.c)
zephyr_library_sources(zrpc-tlv.c)
zephyr_library_sources_ifdef(CONFIG_ZRPC_BACKEND_VIRTIO zrpc-virtio.c)

set(ZRPC_CONFIGURATION ${CONFIG_ZRPC_CONFIGURATION})

if (NOT IS_ABSOLUTE ${ZRPC_CONFIGURATION})
    set(ZRPC_CONFIGURATION ${WEST_TOPDIR}/${ZRPC_CONFIGURATION})
endif()

if (NOT EXISTS ${ZRPC_CONFIGURATION})
    message(FATAL_ERROR "CONFIG_ZRPC_CONFIGURATION does not refer to a file")
endif()

set(zrpc_lib_gen "${CMAKE_CURRENT_BINARY_DIR}/zrpc_lib.cmake")

execute_process(
    COMMAND "${Python3_EXECUTABLE}" -m zrpc
                "${ZRPC_CONFIGURATION}" generate
                    -o "${CMAKE_CURRENT_BINARY_DIR}"
                    --cmake-out "${zrpc_lib_gen}"
    WORKING_DIRECTORY "${ZEPHYR_CURRENT_MODULE_DIR}/scripts"
    RESULT_VARIABLE ret
)

if (ret AND NOT ret EQUAL 0)
    message(WARNING "zRPC generation failed")
else ()
    include("${zrpc_lib_gen}")
endif()

