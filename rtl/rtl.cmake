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

set(COMMON_RTL_SOURCES
    ${CMAKE_SOURCE_DIR}/rtl/common_defs.vh)

set(E_RTL_SOURCES
    ${COMMON_RTL_SOURCES}
    ${CMAKE_SOURCE_DIR}/rtl/e/e_is_1hot.sv
    ${CMAKE_SOURCE_DIR}/rtl/e/e.sv)

set(U_RTL_SOURCES
    ${COMMON_RTL_SOURCES}
    ${CMAKE_SOURCE_DIR}/rtl/u/u_mask.sv
    ${CMAKE_SOURCE_DIR}/rtl/u/u.sv)

set(P_RTL_SOURCES
    ${COMMON_RTL_SOURCES}
    ${CMAKE_SOURCE_DIR}/rtl/p/p_ffs.sv
    ${CMAKE_SOURCE_DIR}/rtl/p/p.sv)

