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

module c #(
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

logic [W - 1:-1]                       x;
logic [W - 1:-1]                       admit_v;
logic [W - 1:-1]                       admit_n_v;
logic [W - 1:-1]                       edge_seen_v;
logic [W - 1:-1]                       edge_seen_n_v;
logic [W - 1:-1]                       all_set_v;
logic [W - 1:-1]                       all_set_n_v;
logic [W - 1:0]                        is_unary_v;
logic [W - 1:0]                        is_unary_n_v;

logic                                  is_unary_cell;
logic                                  is_unary;
logic                                  is_compliment;

// ========================================================================= //
//                                                                           //
// Logic.                                                                    //
//                                                                           //
// ========================================================================= //

// Initial values
assign x = {i_x, 1'b0};

assign admit_v[-1] = 1'b1;
assign admit_n_v[-1] = 1'b1;

assign edge_seen_v[-1] = 1'b0;
assign edge_seen_n_v[-1] = 1'b0;

assign all_set_v[-1] = 1'b0;
assign all_set_n_v[-1] = 1'b0;

for (genvar i = 0; i < W; i++) begin : cell_GEN

localparam bit P_IS_FIRST = (i == 0);

c_cell #(
  .P_IS_FIRST                (P_IS_FIRST)
, .P_IS_COMPLIMENT           (1'b0)
) u_c_cell(
// Input
  .i_x                       (x[i])
, .i_x_prev                  (x[i - 1])
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

if (P_ADMIT_COMPLIMENT_EN) begin : cell_compliment_GEN

c_cell #(
  .P_IS_FIRST                (P_IS_FIRST)
, .P_IS_COMPLIMENT           (1'b0)
) u_c_cell_compliment (
// Input
  .i_x                       (x[i])
, .i_x_prev                  (x[i - 1])
// Prior State
, .i_prior_admit             (admit_n_v[i - 1])
, .i_prior_edge_seen         (edge_seen_n_v[i - 1])
, .i_prior_all_set           (all_set_n_v[i - 1])
// Future State
, .o_admit                   (admit_n_v[i])
, .o_edge_seen               (edge_seen_n_v[i])
, .o_all_set                 (all_set_n_v[i])
// Admission Decision
, .o_is_unary                (is_unary_n_v[i])
);

end : cell_compliment_GEN
else begin : not_cell_compliment_GEN

assign admit_n_v[i] = 1'b0;
assign edge_seen_n_v[i] = 1'b0;
assign all_set_n_v[i] = 1'b0;
assign is_unary_n_v[i] = 1'b0;

end : not_cell_compliment_GEN

end : cell_GEN

// ------------------------------------------------------------------------- //
// Result of final-cell
assign is_unary_cell = (is_unary_v[W - 1] | is_unary_n_v[W - 1]);

// is_unary iff:
//
//   1) cells concensus is normal-form unary encoding.
//
//   2.1) Or, the all-zeros bounary-case vector.
//
//   2.2) Or, when P_ADMIT_COMPLIMENT_EN, the all-ones boundary-case vector.
//
assign is_unary = is_unary_cell |                                // (1)
                  all_set_v[W - 1] |                             // (2.1)
                  (P_ADMIT_COMPLIMENT_EN & all_set_n_v[W - 1]);  // (2.2)

// ------------------------------------------------------------------------- //
// is_compliment whenever configured to detect such encodings and MSB is set.
assign is_compliment = (P_ADMIT_COMPLIMENT_EN & i_x[W - 1]);

// ========================================================================= //
//                                                                           //
// Output(s)                                                                 //
//                                                                           //
// ========================================================================= //

assign o_is_unary = is_unary;
assign o_is_compliment = is_compliment;

// ========================================================================= //
//                                                                           //
// UNUSED                                                                    //
//                                                                           //
// ========================================================================= //

logic UNUSED__tie_off;
assign UNUSED__tie_off = &{ is_unary_v[W - 2:0],
                            is_unary_n_v[W - 2:0],
                            edge_seen_v[W - 1],
                            edge_seen_n_v[W - 1],
                            admit_v[W - 1:-1],
                            admit_n_v[W - 1:-1]
                          };

endmodule : c
