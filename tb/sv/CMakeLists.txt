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

set(VERILATED_ARGS
  "-cc"
  "-Wall"
  "--Mdir vobj"
  "--build"
  "-GW=32"
  "-GP_ADMIT_COMPLIMENT_EN=1'b1"
  "--top-module u")
foreach (file ${RTL_SOURCES})
  list(APPEND VERILATED_ARGS ${file})
endforeach ()

macro(render_verilator_command_list arg_list output_filename)
  file(REMOVE ${output_filename})
  foreach (arg ${arg_list})
    file(APPEND ${output_filename} "${arg}\n")
  endforeach ()
endmacro ()

# ---------------------------------------------------------------------------- #
# Generate Verilator Command File:
#
render_verilator_command_list(
  "${VERILATED_ARGS}" ${CMAKE_CURRENT_BINARY_DIR}/vc.f)

# ---------------------------------------------------------------------------- #
# Verilation:
#
add_custom_target(verilate
  COMMAND ${VERILATOR} -f ${CMAKE_CURRENT_BINARY_DIR}/vc.f
  DEPENDS ${RTL_SOURCES}
  COMMENT "Verilating (${CMAKE_CURRENT_BINARY_DIR})...")