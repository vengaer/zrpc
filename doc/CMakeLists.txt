# Copyright (c) 2026 Vilhelm Engström
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
project(zrpc-docs LANGUAGES NONE)

find_package(Sphinx 8.2.3 REQUIRED)

set(HTML_OUT ${CMAKE_CURRENT_BINARY_DIR}/html)

set(ZRPC_BASE ${CMAKE_CURRENT_LIST_DIR}/..)
file(REAL_PATH "${ZRPC_BASE}" ZRPC_BASE)


set(WEST_TOPDIR "${ZRPC_BASE}/.." CACHE STRING "Path to the west topdir")
file(REAL_PATH "${WEST_TOPDIR}" WEST_TOPDIR)

# -----------------------------------------------------------------
# Doxygen

find_program(DOXYGEN_EXECUTABLE doxygen)

set(DOXYFILE_IN ${CMAKE_CURRENT_LIST_DIR}/doxyfile.in)
set(DOXYFILE_OUT ${CMAKE_CURRENT_BINARY_DIR}/doxyfile)

# Substituted in when configuring doxyfile
set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/doxygen)
set(ZRPC_VERSION "0.0.1")

configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)

add_custom_target(
    doxygen
    COMMAND ${DOXYGEN_EXECUTABLE} "${DOXYFILE_OUT}"
)

set_target_properties(
    doxygen
    PROPERTIES
      ADDITIONAL_CLEAN_FILES "${DOXYGEN_OUT}"
)

add_custom_target(
    doxygen-deploy
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${DOXYGEN_OUT}/html"
        "${HTML_OUT}/doxygen"
    DEPENDS doxygen
)

set_target_properties(
    doxygen-deploy
    PROPERTIES
        ADDITIONAL_CLEAN_FILES "${HTML_OUT}/doxygen"
)

# ----
# pdoc

find_package(Python3 REQUIRED COMPONENTS Interpreter)
set(scripts "${ZRPC_BASE}/scripts")

file(GLOB pymodules "${scripts}/*/__init__.py")
foreach(mod ${pymodules})
    get_filename_component(mod_path "${mod}" DIRECTORY)
    list(APPEND pdoc_input "${mod_path}")
endforeach()

add_custom_target(
    pdoc
    COMMAND "${Python3_EXECUTABLE}" -m pdoc
                ${pdoc_input}
                -o "${HTML_OUT}/pdoc"
                --show-source
                --template-directory=${CMAKE_CURRENT_LIST_DIR}/_templates/pdoc
)

set_target_properties(
    pdoc
    PROPERTIES
        ADDITIONAL_CLEAN_FILES "${HTML_OUT}/pdoc"
)

# -----------
# HTML target

list(APPEND DOC_ENV "ZRPC_BASE=${ZRPC_BASE}")

sphinx_add_docs(html ALL
    DEPENDS doxygen-deploy pdoc
    ENVIRONMENT "${DOC_ENV}"
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/html"
)
