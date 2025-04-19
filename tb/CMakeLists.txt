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

verilate_bool_to_logic(
    RTL_PARAM__ADMIT_COMPLIMENT, RTL_PARAM__ADMIT_COMPLIMENT_LOGIC)

# 'U' Design
#
set(U_VERILATOR_ARGS
    "-cc"
    "-Wall"
    "--build"
    "-GW=${RTL_PARAM__W}"
    "-GP_ADMIT_COMPLIMENT_EN=${RTL_PARAM__ADMIT_COMPLIMENT_LOGIC}"
    "-I${CMAKE_SOURCE_DIR}/rtl"
    "-unused-regexp UNUSED_*"
    "--top-module u")

verilate(u "${U_RTL_SOURCES}" "${U_VERILATOR_ARGS}" v_u_lib)

set(E_VERILATOR_ARGS
    "-cc"
    "-Wall"
    "--build"
    "-GW=${RTL_PARAM__W}"
    "-GP_ADMIT_COMPLIMENT_EN=${RTL_PARAM__ADMIT_COMPLIMENT_LOGIC}"
    "-I${CMAKE_SOURCE_DIR}/rtl"
    "-unused-regexp UNUSED_*"
    "--top-module e")

verilate(e "${E_RTL_SOURCES}" "${E_VERILATOR_ARGS}" v_e_lib)

set(P_VERILATOR_ARGS
    "-cc"
    "-Wall"
    "--build"
    "-GW=${RTL_PARAM__W}"
    "-GP_ADMIT_COMPLIMENT_EN=${RTL_PARAM__ADMIT_COMPLIMENT_LOGIC}"
    "-I${CMAKE_SOURCE_DIR}/rtl"
    "-unused-regexp UNUSED_*"
    "--top-module p")

verilate(p "${P_RTL_SOURCES}" "${P_VERILATOR_ARGS}" v_p_lib)

set(C_VERILATOR_ARGS
    "-cc"
    "-Wall"
    "--build"
    "-GW=${RTL_PARAM__W}"
    "-GP_ADMIT_COMPLIMENT_EN=${RTL_PARAM__ADMIT_COMPLIMENT_LOGIC}"
    "-I${CMAKE_SOURCE_DIR}/rtl"
    "-unused-regexp UNUSED_*"
    "--top-module c")

verilate(c "${C_RTL_SOURCES}" "${C_VERILATOR_ARGS}" v_c_lib)

set(TB_SOURCES
    "${CMAKE_SOURCE_DIR}/tb/log.h"
    "${CMAKE_SOURCE_DIR}/tb/log.cc"
    "${CMAKE_SOURCE_DIR}/tb/common.cc"
    "${CMAKE_SOURCE_DIR}/tb/common.h"
    "${CMAKE_SOURCE_DIR}/tb/random.h"
    "${CMAKE_SOURCE_DIR}/tb/designs.h"
    "${CMAKE_SOURCE_DIR}/tb/designs.cc"
    "${CMAKE_SOURCE_DIR}/tb/stimulus.h"
    "${CMAKE_SOURCE_DIR}/tb/stimulus.cc"
    "${CMAKE_SOURCE_DIR}/tb/tests.h"
    "${CMAKE_SOURCE_DIR}/tb/tests.cc"
    "${CMAKE_SOURCE_DIR}/tb/tb.h"
    "${CMAKE_SOURCE_DIR}/tb/tb.cc")

# Generate TB driver
add_executable(tb ${TB_SOURCES})
target_include_directories(tb PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(tb ${v_u_lib} ${v_e_lib} ${v_p_lib} ${v_c_lib})
set_target_properties(tb PROPERTIES CXX_STANDARD 20)
target_compile_options(tb PRIVATE -Wall -Werror)
if (${CMAKE_SYSTEM_NAME} STREQUAL Darwin)
  # weak symbols used by Verilator set to dynamic lookup on MAC OS
  # (see: verilated.mk; LDFLAGS)
  target_link_options(tb PRIVATE
    -Wl,-U,__Z15vl_time_stamp64v,-U,__Z13sc_time_stampv)
endif ()

macro (cmake_bool_to_cxx var out)
  if (${var})
    set(${out} "true")
  else ()
    set(${out} "false")
  endif ()
endmacro ()

set(CXX_PARAM__W ${RTL_PARAM__W})

set(CXX_PARAM__ADMIT_COMPLIMENT ${RTL_PARAM__ADMIT_COMPLIMENT})
cmake_bool_to_cxx(${RTL_PARAM__ADMIT_COMPLIMENT}, CXX_PARAM__ADMIT_COMPLIMENT)
configure_file(cfg.h.in cfg.h)

find_program(CLANG_FORMAT
    clang-format
    PATHS /usr/bin)

if (CLANG_FORMAT)
    add_custom_target(clang-format
        COMMAND ${CLANG_FORMAT} -i ${TB_SOURCES})
endif()

add_test(NAME test
  COMMAND $<TARGET_FILE:tb>
    -d
    -t d=u,t=FullyRandomizedTestCase,t=DirectedExhaustiveTestCase
    -t d=e,t=FullyRandomizedTestCase,t=DirectedExhaustiveTestCase
    -t d=p,t=FullyRandomizedTestCase,t=DirectedExhaustiveTestCase
  )