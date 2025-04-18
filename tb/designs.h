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

#ifndef TB_DESIGNS_H
#define TB_DESIGNS_H

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "stimulus.h"

namespace tb {

class DesignBase {
 public:
  explicit DesignBase(const std::string& name) : name_(name) {}

  virtual ~DesignBase() = default;

  // Design name
  virtual const std::string& name() const noexcept { return name_; }

  // Evaluate verilated module with stimulus 'v' and return admission
  // decision.
  virtual bool is_unary(const StimulusVector& v) noexcept = 0;

 private:
  // Design name.
  std::string name_;
};

inline class DesignRegistry {
 public:
  class DesignBuilderBase {
   public:
    explicit DesignBuilderBase() = default;
    virtual ~DesignBuilderBase() = default;

    virtual std::unique_ptr<DesignBase> construct() const = 0;
  };

  explicit DesignRegistry() = default;

  std::vector<std::string> designs() const;

  void add(const std::string& name, std::unique_ptr<DesignBuilderBase>&& d) {
    if (designs_.find(name) == designs_.end()) {
      designs_[name] = std::move(d);
    }
  }

  std::unique_ptr<DesignBase> construct_design(const std::string& name);
  std::unique_ptr<DesignBase> construct_design(const std::string_view& name);

 private:
  std::unordered_map<std::string, std::unique_ptr<DesignBuilderBase>> designs_;
} DESIGN_REGISTRY;

}  // namespace tb

#endif
