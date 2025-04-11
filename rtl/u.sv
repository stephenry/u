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

module u #(
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

logic [W - 2:0]              match_lo_v;
logic [W - 2:0]              match_hi_v;
logic [W - 2:0]              match_v;
logic [W - 2:0]              match_lo_n_v;
logic [W - 2:0]              match_hi_n_v;
logic [W - 2:0]              match_n_v;
logic [W - 2:0]              is_unary_v;
logic                        is_unary;

// ========================================================================= //
//                                                                           //
// Logic.                                                                    //
//                                                                           //
// ========================================================================= //

for (genvar i = 0; i < (W - 1); i++) begin : is_unary_i_GEN

// Match: xxxxxx[1]11111, where [] is pivot 'i'
u_mask #(.W(W), .I(i), .MATCH_BIT(1'b1), .LSB(1'b1))
   u_u_mask_lsb (.i_x(i_x), .o_match(match_lo_v[i]));

// Match: 000000[x]xxxxx, where [] is pivot 'i'
u_mask #(.W(W), .I(i + 1), .MATCH_BIT(1'b0), .LSB(1'b0))
   u_u_mask_msb (.i_x(i_x), .o_match(match_hi_v[i]));

if (P_ADMIT_COMPLIMENT_EN) begin : admit_compliment_GEN

  // Match: xxxxxx[0]000000, where [] is pivot 'i'.
  u_mask #(.W(W), .I(i), .MATCH_BIT(1'b0), .LSB(1'b1))
     u_u_mask_lsb_n (.i_x(i_x), .o_match(match_lo_n_v[i]));

  // Match: 111111[x]xxxxxx, where [] is pivot 'i'.
  u_mask #(.W(W), .I(i + 1), .MATCH_BIT(1'b1), .LSB(1'b0))
     u_u_mask_msb_n (.i_x(i_x), .o_match(match_hi_n_v[i]));

end : admit_compliment_GEN
else begin : not_admit_compliment

  // Otherwise, disabled.
  assign match_lo_n_v[i] = 1'b0;
  assign match_hi_n_v[i] = 1'b0;

end : not_admit_compliment

// Match on upper and lower segments of unary code.
assign match_v[i]   = (match_lo_v[i] & match_hi_v[i]);
// Similarly, match on compliment.
assign match_n_v[i] = (match_lo_n_v[i] & match_hi_n_v[i]);

end : is_unary_i_GEN

// Handle all-zero (0) boundary condition as a special-case.
assign match_v[W - 2]   = (i_x == '0);

// When compliment is considered, handle all-one (1) boundary condition
// as a special-case.
assign match_n_v[W - 2] = P_ADMIT_COMPLIMENT_EN ? (i_x == '1) : 1'b0;

// Match vector on unary code or its compliment. 
assign is_unary_v = (match_v | match_n_v);

// Admit is unary-/thermometer- code if valid encoding is found at any
// pivot index in the input bit-vector. 
assign is_unary = (is_unary_v != 0);

// ========================================================================= //
//                                                                           //
// Output(s)                                                                 //
//                                                                           //
// ========================================================================= //

assign o_is_unary = is_unary;

endmodule : u
