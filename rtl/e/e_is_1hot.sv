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

module e_is_1hot #(
// ------------------------------------------------------------------------- //
// Bit-Width
  parameter int W
) (
// ------------------------------------------------------------------------- //
// Input vector
  input wire logic [W - 1:0]                     i_x

// Admission Decision
, output wire logic                              o_is_1hot
);

// ========================================================================= //
//                                                                           //
// Wire(s)                                                                   //
//                                                                           //
// ========================================================================= //

logic [W - 1:0][W - 1:0]                hot2_v;
logic [W - 1:0]                         hot1_v;

// ========================================================================= //
//                                                                           //
// Logic.                                                                    //
//                                                                           //
// ========================================================================= //

// ------------------------------------------------------------------------- //
//
for (genvar j = 0; j < W; j++) begin : hot1_j_GEN

  for (genvar i = 0; i < W; i++) begin : hot1_i_GEN

assign hot2_v[j][i] = (j == i) ? i_x[i] : ((j > i) : 1'b1 : ~i_x[i]);

  end : hot1_i_GEN

end : hot1_j_GEN

// ------------------------------------------------------------------------- //
//

for (genvar i = 0; i < W; i++) begin : hot1_i_GEN

assign hot1_v[i] = (hot2_v[i] == '1);

end : hot1_i_GEN

// ========================================================================= //
//                                                                           //
// Output(s)                                                                 //
//                                                                           //
// ========================================================================= //

assign o_is_1hot = (hot1_v != '0);

endmodule : e_is_1hot
