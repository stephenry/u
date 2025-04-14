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

#ifndef TB_STIMULUS_H
#define TB_STIMULUS_H

#include <algorithm>
#include <array>
#include <tuple>

#include "cfg.h"
#include "common.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#include "verilated.h"
#pragma GCC diagnostic pop

namespace tb {

template <std::size_t W, typename T = vluint8_t>
class VBitVector {
  static constexpr std::size_t size_in_bits_n = W;
  static constexpr std::size_t size_in_bytes_n = ceil(size_in_bits_n, 8);
  static constexpr std::size_t bits_in_tail_n = (W % sizeof(T));

 public:
  using value_type = T;

  static VBitVector<W> all_zeros() { return VBitVector<W>{}; }

  static VBitVector<W> all_ones() {
    VBitVector<W> v{};
    for (std::size_t i = 0; i < W; i++) {
      v.bit(i, true);
    }
    return v;
  }

  explicit VBitVector() { clear(); }

  explicit VBitVector(vluint8_t* d, std::size_t n) {
    clear();
    std::copy_n(d, std::min(n, size_bytes_n()), v_.data());
  }

  static constexpr std::size_t size() noexcept { return size_in_bits_n; }
  static constexpr std::size_t size_bytes_n() noexcept {
    return size_in_bytes_n;
  }

  void clear() noexcept { std::fill(v_.begin(), v_.end(), 0); }

  void clean() noexcept {
    if constexpr (bits_in_tail_n > 0) {
      v_.back() = v_.back() & mask<vluint8_t, bits_in_tail_n>();
    }
  }

  void bit(std::size_t i, bool b = true) noexcept {
    vluint8_t mask = (1 << (i & 0x7));
    if (b) {
      // set bit
      v_[i >> 3] |= mask;
    } else {
      // clear bit
      v_[i >> 3] &= (~mask);
    }
  }

  bool bit(std::size_t i) const noexcept {
    const std::size_t byte = (i >> 3);
    if (v_.size() <= byte) {
      // Infinite zero-extend.
      return false;
    }
    return (v_[byte] & (1ul << (i & 0x7))) != 0;
  }

  void value(std::size_t i, value_type v) { v_[i] = v; }

// clang-format off
#define VERILATOR_PORT_TYPES(__func) \
  __func(vluint32_t)

#define DECLARE_VERILATOR_PORT_CONVERTER(__type)                        \
  void to_verilated(__type& t) const noexcept {                         \
    to_verilated_impl(reinterpret_cast<vluint8_t*>(std::addressof(t))); \
  }
  VERILATOR_PORT_TYPES(DECLARE_VERILATOR_PORT_CONVERTER)
#undef DECLARE_VERILATOR_PORT_CONVERTER
  // clang-format on

 protected:
  void to_verilated_impl(value_type* b) const noexcept {
    std::copy(v_.begin(), v_.end(), b);
  }
  std::array<value_type, size_in_bytes_n> v_;
};

class VBit : public VBitVector<1> {
 public:
  static VBit from_verilated(vluint8_t t) { return VBit{t != 0}; }

  explicit VBit(bool b) { v_[0] = b ? 0b1 : 0b0; }

  bool to_bool() const noexcept { return (v_[0] != 0); }
};

using StimulusVector = VBitVector<tb::cfg::W>;

std::tuple<bool, bool> is_unary(const StimulusVector& b);

StimulusVector generate_unary(std::size_t n, bool compliment = false);

std::tuple<bool, StimulusVector> generate_non_unary(std::size_t rounds_n = 1);

}  // namespace tb

#endif
