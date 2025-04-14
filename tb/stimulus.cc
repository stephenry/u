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

#include "stimulus.h"

#include "random.h"

namespace tb {

std::tuple<bool, bool> is_unary(const StimulusVector& b) {
  std::size_t edges = 0, zeros = 0, ones = 0;
  for (std::size_t i = 1; i < b.size(); ++i) {
    if (b.bit(i)) {
      ++ones;
    } else {
      ++zeros;
    }

    if (b.bit(i) ^ b.bit(i - 1)) {
      ++edges;
    }
  }

  bool is_compliment = b.bit(b.size() - 1);
  bool is_unary = (is_compliment ? (ones == b.size()) : (zeros == b.size())) ||
                  (edges == 1);

  return {is_unary, is_compliment};
}

StimulusVector generate_unary(std::size_t n, bool compliment) {
  StimulusVector v;
  for (std::size_t i = 0; i < v.size(); i++) {
    if (i < n) {
      v.bit(i, !compliment);
    } else {
      v.bit(i, compliment);
    }
  }
  return v;
}

std::tuple<bool, StimulusVector> generate_non_unary(std::size_t rounds_n) {
  while (rounds_n--) {
    StimulusVector v;
    for (std::size_t i = 0; i < v.size_bytes_n(); i++) {
      v.value(i, RANDOM.uniform<StimulusVector::value_type>());
    }
    v.clean();

    if (auto [unary, compliment] = is_unary(v); !unary) {
      return {true, v};
    }

    // Otherwise, we've somehow hit a unary integer. Repeat until success.
  }

  // Pathological case where we've been unable to generate a non-unary case.
  // Unlikely to ever to occur outside of exceptional cases.
  return {false, StimulusVector{}};
}

}  // namespace tb