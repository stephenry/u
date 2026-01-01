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

#include <sstream>

#include "designs.h"
#include "verilated_vcd_c.h"

namespace tb {

std::unique_ptr<DesignBase> DesignRegistry::construct_design(
    const std::string& name) {
  if (auto it = designs_.find(name); it != designs_.end()) {
    return it->second->construct();
  }
  return nullptr;
}

std::unique_ptr<DesignBase> DesignRegistry::construct_design(
    const std::string_view& name) {
  return construct_design(std::string{name});
}

template <typename T>
concept VUnaryModule = requires(T t) {
  { t.eval() } -> std::same_as<void>;
  t.i_x;
  t.o_is_unary;
  t.o_is_compliment;
};

template <VUnaryModule T>
class Design : public DesignBase {
 public:
  explicit Design(const std::string& name) : DesignBase(name) {
    ctxt_ = std::make_unique<VerilatedContext>();
    if constexpr (T::traceCapable) {
      ctxt_->traceEverOn(OPTIONS.vcd_en);
    }
    uut_ = std::make_unique<T>(ctxt_.get(), name.c_str());
    if constexpr (T::traceCapable) {
      construct_trace();
    }
  }

  ~Design() {
    if constexpr (T::traceCapable) {
      destruct_trace();

    }
  }

  std::tuple<bool, bool> is_unary(const StimulusVector& v) noexcept override {
    // Drive input
    v.to_verilated(uut_->i_x);
    // Advance simulator
    step();
    // Return response.
    return {VBit::from_verilated(uut_->o_is_unary).to_bool(),
            VBit::from_verilated(uut_->o_is_compliment).to_bool()};
  }

 private:
  void step(std::size_t n = 1) {
    while (n--) {
      // Advance time
      ctxt_->timeInc(1);
      // Evaluate
      uut_->eval();

      if constexpr (T::traceCapable) {
        if (OPTIONS.vcd_en) {
          vcd_->dump(ctxt_->time());
        }
      }
    }
  }
  void construct_trace() {
    if (!OPTIONS.vcd_en) {
      return;
    }

    vcd_ = std::make_unique<VerilatedVcdC>();
    uut_->trace(vcd_.get(), 99);
    std::ostringstream ss;
    ss << name() << ".vcd";
    vcd_->open(ss.str().c_str());
  }

  void destruct_trace() {
    if (!vcd_) {
      return;
    }

    // Append some sort rundown period at end-of-simulation to
    // emit final parts of trace.
    step(2);
  
    vcd_->close();
  }

  std::unique_ptr<VerilatedContext> ctxt_;
  std::unique_ptr<VerilatedVcdC> vcd_;
  std::unique_ptr<T> uut_;
};

template <typename T>
class DesignBuilder : public tb::DesignRegistry::DesignBuilderBase {
 public:
  explicit DesignBuilder(const std::string& name) : name_(name) {}
  std::unique_ptr<DesignBase> construct() const override {
    return std::unique_ptr<DesignBase>(new Design<T>(name_));
  }

 private:
  std::string name_;
};

}  // namespace tb

// clang-format off
#define DECLARE_DESIGN(__name)                            \
  static const struct DesignRegister##__name {            \
    explicit DesignRegister##__name() {                   \
      auto b = std::unique_ptr<tb::DesignRegistry::DesignBuilderBase>(\
        new tb::DesignBuilder<V##__name>(#__name)); \
      tb::DESIGN_REGISTRY.add(#__name, std::move(b)); \
    }                                                     \
  } __register_##__name {}
// clang-format on

#include "VObj_u/Vu.h"
DECLARE_DESIGN(u);

#include "VObj_e/Ve.h"
DECLARE_DESIGN(e);

#include "VObj_p/Vp.h"
DECLARE_DESIGN(p);

#include "VObj_c/Vc.h"
DECLARE_DESIGN(c);

#include "VObj_o/Vo.h"
DECLARE_DESIGN(o);

#undef DECLARE_DESIGN
