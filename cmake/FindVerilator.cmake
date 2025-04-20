## ==================================================================== ##
## Copyright (c) 2025, Stephen Henry
## All rights reserved.
##
## Redistribution and use in source and binary forms, with or without
## modification, are permitted provided that the following conditions
## are met:
##
## * Redistributions of source code must retain the above copyright
##   notice, this list of conditions and the following disclaimer.
##
## * Redistributions in binary form must reproduce the above copyright
##   notice, this list of conditions and the following disclaimer in
##   the documentation and/or other materials provided with the
##   distribution.
##
## THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
## "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
## LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
## FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
## COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
## INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
## (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
## SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
## HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
## STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
## ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
## OF THE POSSIBILITY OF SUCH DAMAGE.
## ==================================================================== ##

if (EXISTS $ENV{VERILATOR_ROOT})
  set(VERILATOR_ROOT $ENV{VERILATOR_ROOT})
  set(VERILATOR_EXE ${VERILATOR_ROOT}/bin/verilator)
  execute_process(COMMAND ${VERILATOR_EXE} --version
    OUTPUT_VARIABLE Verilator_VER)
  string(REGEX REPLACE "Verilator ([0-9]).([0-9]+).*" "\\1"
    VERILATOR_VERSION_MAJOR ${Verilator_VER})
  string(REGEX REPLACE "Verilator ([0-9]).([0-9]+).*" "\\2"
    VERILATOR_VERSION_MINOR ${Verilator_VER})
  set(VERILATOR_VERSION
    ${VERILATOR_VERSION_MAJOR}.${VERILATOR_VERSION_MINOR})
  message(STATUS "Found Verilator version: ${VERILATOR_VERSION}")

  add_library(vlib STATIC
    "${VERILATOR_ROOT}/include/verilated.h"
    "${VERILATOR_ROOT}/include/verilated.cpp"
    "${VERILATOR_ROOT}/include/verilated_dpi.h"
    "${VERILATOR_ROOT}/include/verilated_dpi.cpp"
    "${VERILATOR_ROOT}/include/verilated_save.h"
    "${VERILATOR_ROOT}/include/verilated_save.cpp"
    "${VERILATOR_ROOT}/include/verilated_threads.h"
    "${VERILATOR_ROOT}/include/verilated_threads.cpp"
    "${VERILATOR_ROOT}/include/verilated_vcd_c.h"
    "${VERILATOR_ROOT}/include/verilated_vcd_c.cpp")
  target_include_directories(vlib PUBLIC
    "${VERILATOR_ROOT}/include"
    "${VERILATOR_ROOT}/include/vltstd")
  set_target_properties(vlib PROPERTIES CXX_STANDARD 14)

  macro(verilate_bool_to_logic var out)
    if (${var})
      set(${out} "1'b1")
    else ()
      set(${out} "1'b0")
    endif ()
  endmacro()

  macro(verilate design rtl_sources command_list target_out)
    set(command_file ${CMAKE_CURRENT_BINARY_DIR}/${design}_vc.f)
    set(out_dir ${CMAKE_CURRENT_BINARY_DIR}/VObj_${design})
    # Output directory does not exist until Verilator has run; but
    # IMPORTED_INCLUDE_DIRECTORIES below requires that the directory
    # exist at the time configuration is performed.
    file(MAKE_DIRECTORY ${out_dir})
    set(generated_header ${out_dir}/V${design}.h)

    set(generated_library_name V${design}__ALL)
    set(generated_library ${out_dir}/${generated_library_name}.a)

    # Construct verilator argument list
    set(verilator_commands ${command_list})
    list(APPEND verilator_commands "--Mdir ${out_dir}")
    if (${OPT_VCD_ENABLE})
      list(APPEND verilator_commands "--trace")
    endif ()
    
    # Render Verilator command file.
    file(REMOVE ${command_file})
    foreach (arg ${verilator_commands})
      file(APPEND ${command_file} "${arg}\n")
    endforeach ()

    foreach (rtl_source ${rtl_sources})
      file(APPEND ${command_file} "${rtl_source}\n")
    endforeach ()

    # Verilator command/target definitions.
    add_custom_command(
      COMMAND ${VERILATOR_EXE} -f ${command_file}
      OUTPUT ${generated_header} ${generated_library}
      DEPENDS ${rtl_sources}
      DEPENDS ${command_file}
      )
    add_custom_target(
      verilate_${design}
      DEPENDS ${generated_header} ${generated_library})

    # Create imported library denoting the static library compiled
    # by Verilator.
    add_library(${generated_library_name} IMPORTED STATIC GLOBAL)
    set_target_properties(${generated_library_name} PROPERTIES
      IMPORTED_LOCATION "${generated_library}"
      INTERFACE_INCLUDE_DIRECTORIES "${out_dir}"
      INTERFACE_LINK_LIBRARIES vlib)

    # Generated library has dependency on Verilator runtime.
    add_dependencies(${generated_library_name} verilate_${design} vlib)

    # Set output library variable
    set(${target_out} ${generated_library_name})
  endmacro ()
else ()
  # Configuration script expects and requires that the VERILATOR_ROOT
  # variable be set in the current environment.
  message(WARNING [[
    "VERILATOR_ROOT has not been defined in the environment. "
    "Verilator not found! Simulation is not supported"
    ]])
endif ()
