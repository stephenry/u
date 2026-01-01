//========================================================================== //
// Copyright (c) 2025, Stephen Henry
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//========================================================================== //

`include "common_defs.vh"

module o #(
// ------------------------------------------------------------------------- //
// Bit-Width
  parameter int W
// Enable admission of complimented unary code
, parameter bit P_ADMIT_COMPLIMENT_EN
) (
// ------------------------------------------------------------------------- //
// Input vector
  input wire logic [W - 1:0]                     i_x

// Admission Decision
, output wire logic                              o_is_unary
// Compliment form unary.
, output wire logic                              o_is_compliment
);

// Circuit to admit an arbitrary lengthed unary-/thermometer-coded bit-vector.
// Circuit admits both a standard unary format and, conditionally, its
// compliment.
//
// Where a unary-/therometer code is defined as (for some bitwidth 'W'):
//
//   0 |   0000_0000_0000_0000         1111_1111_1111_1111
//   1 |   0000_0000_0000_0001         1111_1111_1111_1110
//   2 |   0000_0000_0000_0011         1111_1111_1111_1100
//   3 |   0000_0000_0000_0111         1111_1111_1111_1000
//   4 |   0000_0000_0000_1111         1111_1111_1111_0000
//   5 |   0000_0000_0001_1111         1111_1111_1110_0000
//   . |             .                           .
//   . |             .                           .
//   . |             .                           .
//  14 |   0111_1111_1111_1111         1000_0000_0000_0000
//
//  Circuit does not admit the all-one or all-zero bitvector (respectively)
//  as this is not considered to be a valid unary encoding. 

// ========================================================================= //
//                                                                           //
// Wire(s)                                                                   //
//                                                                           //
// ========================================================================= //

logic                        is_unary_nc;
logic                        is_unary_c;
logic                        is_compliment;

// ========================================================================= //
//                                                                           //
// Logic.                                                                    //
//                                                                           //
// ========================================================================= //

// ------------------------------------------------------------------------- //
// Unary is no overlap between i_x and its increment.
//
// For example:
//
//   unary     -  'b00011111 -> 'b00100000 & 'b00011111 = 'b0
//
//   not-unary -  'b00101001 -> 'b00101010 & 'b00101001 = 'b00101000
//
assign is_unary_nc = ((i_x + 'b1) & i_x) == '0;

// ------------------------------------------------------------------------- //
//
if (P_ADMIT_COMPLIMENT_EN) begin: admit_c_GEN
  assign is_unary_c = (((~i_x) + 'b1) & i_x) == '0;
  assign is_compliemnt = i_x[W - 1];
end: admit_c_GEN
else begin: no_admit_c_GEN
  assign is_unary_c = 'b0;
  assign is_compliment = 'b0;
end: no_admit_c_GEN

// ========================================================================= //
//                                                                           //
// Output(s)                                                                 //
//                                                                           //
// ========================================================================= //

assign o_is_unary = (is_unary_nc | is_unary_c);
assign o_is_compliment = is_compliment;

endmodule : o
