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

#include "common.h"

namespace tb {

std::vector<std::string_view> split(const std::string_view& s,
                                    std::string_view::value_type sep) {
  using size_type = std::string_view::size_type;

  std::vector<std::string_view> vs;

  auto try_add_string_piece = [&](size_type begin, size_type end) {
    if (begin == end) {
      return;
    }
    vs.push_back(s.substr(begin, end - begin + 1));
  };

  size_type begin = 0, end = s.find(sep);
  while (end != std::string_view::npos) {
    try_add_string_piece(begin, end);

    end = s.find(sep, begin);
  }
  try_add_string_piece(begin, end);

  return vs;
}

std::tuple<bool, std::string_view, std::string_view> split_kv(
    const std::string_view& sv) {
  const std::vector<std::string_view> vs{split(sv, '=')};

  if (vs.size() != 2) {
    return {false, "", ""};
  }

  return {true, vs[0], vs[1]};
}

}  // namespace tb
