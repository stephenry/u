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

module c_v #(
// ------------------------------------------------------------------------- //
// Bit-Width
  parameter int W
// Enable admission of complimented unary code
, parameter bit P_IS_COMPLIMENT
) (
// ------------------------------------------------------------------------- //
// Input vector
  input wire logic [W - 1:0]                     i_x

// Admission Decision
, output wire logic                              o_is_unary
, output wire logic                              o_all_set
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

logic [W - 1:0]                        admit_v;
logic [W - 1:0]                        edge_seen_v;
logic [W - 1:0]                        all_set_v;
logic [W - 1:0]                        is_unary_v;

logic                                  all_set;
logic                                  is_unary;

// ========================================================================= //
//                                                                           //
// Logic.                                                                    //
//                                                                           //
// ========================================================================= //

c_v_cell #(
  .P_IS_FIRST                (1'b1)
, .P_IS_COMPLIMENT           (P_IS_COMPLIMENT)
) u_c_v_cell(
// Input
  .i_x                       (i_x[0])
, .i_x_prev                  (1'b0)
// Prior State
, .i_prior_admit             (1'b1)
, .i_prior_edge_seen         (1'b0)
, .i_prior_all_set           (1'b0)
// Future State
, .o_admit                   (admit_v[0])
, .o_edge_seen               (edge_seen_v[0])
, .o_all_set                 (all_set_v[0])
// Admission Decision
, .o_is_unary                (is_unary_v[0])
);

for (genvar i = 1; i < W; i++) begin : cell_GEN

c_v_cell #(
  .P_IS_FIRST                (1'b0)
, .P_IS_COMPLIMENT           (P_IS_COMPLIMENT)
) u_c_v_cell (
// Input
  .i_x                       (i_x[i])
, .i_x_prev                  (i_x[i - 1])
// Prior State
, .i_prior_admit             (admit_v[i - 1])
, .i_prior_edge_seen         (edge_seen_v[i - 1])
, .i_prior_all_set           (all_set_v[i - 1])
// Future State
, .o_admit                   (admit_v[i])
, .o_edge_seen               (edge_seen_v[i])
, .o_all_set                 (all_set_v[i])
// Admission Decision
, .o_is_unary                (is_unary_v[i])
);

end : cell_GEN

assign all_set = all_set_v[W - 1];

assign is_unary = is_unary_v[W - 1];

// ========================================================================= //
//                                                                           //
// Output(s)                                                                 //
//                                                                           //
// ========================================================================= //

assign o_is_unary = is_unary;
assign o_all_set = all_set;

// ========================================================================= //
//                                                                           //
// UNUSED                                                                    //
//                                                                           //
// ========================================================================= //

logic UNUSED__tie_off;
assign UNUSED__tie_off = &{ is_unary_v[W - 2:0],
                            edge_seen_v[W - 1],
                            admit_v[W - 1]
                          };

endmodule : c_v
