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

#include "designs.h"

namespace tb {

std::vector<std::string> DesignRegistry::designs() const {
  std::vector<std::string> vs;
  vs.reserve(designs_.size());
  for (auto& [k, v] : designs_) {
    vs.push_back(k);
  }
  return vs;
}

std::unique_ptr<DesignBase> DesignRegistry::construct_design(
    const std::string& name) {
  if (auto it = designs_.find(name); it != designs_.end()) {
    return it->second->construct();
  }
  return nullptr;
}

}  // namespace tb

// clang-format off
#define DECLARE_DESIGN(__name)                            \
  static const struct DesignRegister##__name {            \
    explicit DesignRegister##__name() {                   \
      tb::DESIGN_REGISTRY.add_design<V##__name>(#__name); \
    }                                                     \
  } __register_##__name {}
// clang-format on

#include "VObj_u/Vu.h"
DECLARE_DESIGN(u);

#include "VObj_e/Ve.h"
DECLARE_DESIGN(e);

#undef DECLARE_DESIGN