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

include(rtl)

# 'U' Design
#
set(U_VERILATOR_ARGS
    "-cc"
    "-Wall"
    "--build"
    "-GW=${RTL_PARAM__W}"
    "-GP_ADMIT_COMPLIMENT_EN=1'b1"
    "-I${CMAKE_SOURCE_DIR}/rtl"
    "--top-module u")

verilate(u "${U_RTL_SOURCES}" "${U_VERILATOR_ARGS}")

set(E_VERILATOR_ARGS
    "-cc"
    "-Wall"
    "--build"
    "-GW=${RTL_PARAM__W}"
    "-GP_ADMIT_COMPLIMENT_EN=1'b1"
    "-I${CMAKE_SOURCE_DIR}/rtl"
    "--top-module e")

verilate(e "${E_RTL_SOURCES}" "${E_VERILATOR_ARGS}")

# Generate TB driver
add_executable(tb tb.cc)
target_include_directories(tb PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(tb Vu__ALL Ve__ALL)
set_target_properties(tb PROPERTIES
    CXX_STANDARD 20)

configure_file(tb.h.in tb.h)