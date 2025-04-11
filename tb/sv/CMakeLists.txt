##========================================================================== //
## Copyright (c) 2025, Stephen Henry
## All rights reserved.
##
## Redistribution and use in source and binary forms, with or without
## modification, are permitted provided that the following conditions are met:
##
## * Redistributions of source code must retain the above copyright notice, this
##   list of conditions and the following disclaimer.
##
## * Redistributions in binary form must reproduce the above copyright notice,
##   this list of conditions and the following disclaimer in the documentation
##   and/or other materials provided with the distribution.
##
## THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
## AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
## IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
## ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
## LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
## CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
## SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
## INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
## CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
## ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
## POSSIBILITY OF SUCH DAMAGE.
##========================================================================== //


# ---------------------------------------------------------------------------- #
# Source definitions:
#
include(rtl)
rtl_render_and_copy()

# ---------------------------------------------------------------------------- #
# Technology definitions (sim):
#
include(tech)
tech_render_and_copy("sim")

# ---------------------------------------------------------------------------- #
# Emit configuration files:
#
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/tb_pkg.vh.in" "tb_pkg.vh")

set(VERILATED_ARGS
  "-cc"
  "-Wall"
  "--Mdir ${VERILATED_DIR}"
  "--build"
  "-DTB_HAS_CMD_ID"
  "--unused-regexp \"UNUSED_*\""
  "--top-module ${VERILATED_TOP}"
  "-I${CMAKE_CURRENT_BINARY_DIR}"
  "--trace")

# ---------------------------------------------------------------------------- #
# Compile file-list:
#
set(TB_SOURCES
  "${RTL_BUILD_SOURCES}"
  "${TECH_BUILD_SOURCES}"
  # TB Sources:
  "${CMAKE_CURRENT_SOURCE_DIR}/hash/hash.sv"
  "${CMAKE_CURRENT_SOURCE_DIR}/hash/hash_pathological.sv"
  "${CMAKE_CURRENT_SOURCE_DIR}/hash/hash_parity.sv"
  "${CMAKE_CURRENT_SOURCE_DIR}/tb.sv")

# ---------------------------------------------------------------------------- #
# Enable SV-Assertions:
#
if (ENABLE_SVA)
  # Building with SV-Assertions (SVA); add in relevant sourcefiles as necessary.
  list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/sva")
  include(sva)
  sva_render_and_copy()
  list(APPEND TB_SOURCES "${SVA_BUILD_SOURCES}")
endif ()

# ---------------------------------------------------------------------------- #
# Consolidate include directories:
#
set(VERILATED_COMMAND_LIST
  "${VERILATED_ARGS}"
  "${TB_SOURCES}")

macro(render_verilator_command_list arg_list output_filename)
  file(REMOVE ${output_filename})
  foreach (arg ${arg_list})
    file(APPEND ${output_filename} "${arg}\n")
  endforeach ()
endmacro ()

# ---------------------------------------------------------------------------- #
# Generate Verilator Command File:
#
set(VERILATED_COMMAND_FILE "${CMAKE_CURRENT_BINARY_DIR}/vc.f")
render_verilator_command_list(
  "${VERILATED_COMMAND_LIST}" ${VERILATED_COMMAND_FILE})

# ---------------------------------------------------------------------------- #
# Verilation:
#
add_custom_target(verilate
  COMMAND ${VERILATOR_EXE} -f ${VERILATED_COMMAND_FILE}
  DEPENDS ${TB_SOURCES}
  COMMENT "Verilating (${CMAKE_CURRENT_BINARY_DIR})...")