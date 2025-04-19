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
// Enable admission of complimented unary code
  parameter bit P_ADMIT_COMPLIMENT_EN
)(
// Input bit
  input wire logic                               i_x

// ------------------------------------------------------------------------- //
// Prior State
, input wire logic                               i_prior_all_ones
, input wire logic                               i_prior_all_zeros_n
, input wire logic                               i_prior_is_unary
, input wire logic                               i_prior_is_unary_n
, input wire logic                               i_prior_seen_edge

// ------------------------------------------------------------------------- //
// Future State
, output wire logic                              o_all_ones
, output wire logic                              o_all_zeros_n
, output wire logic                              o_seen_edge

// Admission Decision
, output wire logic                              o_is_unary
, output wire logic                              o_is_unary_n
);

// ========================================================================= //
//                                                                           //
// Wire(s)                                                                   //
//                                                                           //
// ========================================================================= //

logic                                  kill_is_unary;
logic                                  kill_is_unary_n;
logic                                  all_ones;
logic                                  all_zeros_n;
logic                                  seen_edge_x;
logic                                  seen_edge;
logic                                  is_unary;
logic                                  is_unary_n;

// ========================================================================= //
//                                                                           //
// Logic.                                                                    //
//                                                                           //
// ========================================================================= //

// ------------------------------------------------------------------------- //
// Boundary cases.

// Detect vector: 0000_0000_0000_0000
assign all_zeros_n = (i_x | i_prior_all_zeros_n);

// Detect vector: 1111_1111_1111_1111
assign all_ones = (i_x & i_prior_all_ones);

assign seen_edge_x = i_x ? (~i_prior_all_zeros_n) : i_prior_all_ones;

// Accumulate edge detection across vector length.
assign seen_edge = (i_prior_seen_edge | seen_edge_x);

assign kill_is_unary = 1'b0;

assign is_unary = (~kill_is_unary) & i_prior_is_unary;

assign kill_is_unary_n = (~P_ADMIT_COMPLIMENT_EN);

assign is_unary_n = (~kill_is_unary_n) & i_prior_is_unary_n;

// ========================================================================= //
//                                                                           //
// Output(s)                                                                 //
//                                                                           //
// ========================================================================= //

assign o_all_ones = all_ones;
assign o_all_zeros_n = all_zeros_n;
assign o_seen_edge = seen_edge;

assign o_is_unary = is_unary;
assign o_is_unary_n = is_unary_n;

endmodule : c_cell
