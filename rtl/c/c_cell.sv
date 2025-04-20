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

module c_cell #(
// ------------------------------------------------------------------------- //
// First cell of the vector
  parameter bit P_IS_FIRST
// Admit complimented unary code
, parameter bit P_IS_COMPLIMENT
)(
// Input bit
  input wire logic                               i_x
, input wire logic                               i_x_prev

// ------------------------------------------------------------------------- //
// Prior State
, input wire logic                               i_prior_admit
, input wire logic                               i_prior_edge_seen
, input wire logic                               i_prior_all_set

// ------------------------------------------------------------------------- //
// Future State
, output wire logic                              o_admit
, output wire logic                              o_edge_seen
, output wire logic                              o_all_set

// Admission Decision
, output wire logic                              o_is_unary
);

// ========================================================================= //
//                                                                           //
// Wire(s)                                                                   //
//                                                                           //
// ========================================================================= //

logic                                  is_edge;
logic                                  edge_seen;
logic                                  edge_multiple;
logic                                  all_set;
logic                                  admit;
logic                                  is_unary;

// ========================================================================= //
//                                                                           //
// Logic.                                                                    //
//                                                                           //
// ========================================================================= //

// Detect edge on not-first index.
assign is_edge = P_IS_FIRST ? 1'b0 : (i_x ^ i_x_prev);

// Accumulate edge detection across vector-length
assign edge_seen = P_IS_FIRST ? 1'b0 : (is_edge | i_prior_edge_seen);

// Detect duplicate edge-case
assign edge_multiple = (is_edge & i_prior_edge_seen);

// Admit: On first index, expected value; Otherwise, not multiple edge case.
assign admit =
  P_IS_FIRST ? (P_IS_COMPLIMENT ? (~i_x) : i_x) : i_prior_admit & (~edge_multiple);

// Current position is valid unary encoding if at most one edge has been
// seen and the value of vector is a valid terminal value. In the
// degenerate case, the 1-bit vector is not considered to be a valid
// unary vector.
assign is_unary =
  P_IS_FIRST ? 1'b0 : (edge_seen & admit & (P_IS_COMPLIMENT ? i_x : (~i_x)));

// Detect all-set/-clear for boundary case.
assign all_set =
  (P_IS_COMPLIMENT ? i_x : (~i_x)) & (i_prior_all_set | P_IS_FIRST);

// ========================================================================= //
//                                                                           //
// Output(s)                                                                 //
//                                                                           //
// ========================================================================= //

assign o_admit = admit;
assign o_edge_seen = edge_seen;
assign o_all_set = all_set;

assign o_is_unary = is_unary;

endmodule : c_cell
