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

#include "tests.h"

#include "designs.h"
#include "log.h"
#include "random.h"
#include "stimulus.h"

namespace tb {

bool TestCase::check(DesignBase* b, const StimulusVector& v) {
  U_LOG_INFO("Trial: ", v);
 
  auto [rtl_is_unary, rtl_is_compliment] = b->is_unary(v);
  auto [beh_is_unary, beh_is_compliment] = is_unary(v);

  U_LOG_INFO("RTL: is_unary=", rtl_is_unary, ", rtl_is_compliment=", rtl_is_compliment);
  U_LOG_INFO("BEH: is_unary=", beh_is_unary, ", beh_is_compliment=", beh_is_compliment);

  if (rtl_is_unary != beh_is_unary) {
    U_LOG_ERROR("Mismatch on unary-encoding admission.");
    ++mismatches_;
    return false;
  }

  if (has_compliment_) {
    if (rtl_is_compliment != beh_is_compliment) {
      U_LOG_ERROR("Mismatch on compliment detection.");
      ++mismatches_;
      return false;
    }
  } else if (rtl_is_compliment) {
    U_LOG_ERROR("RTL asserts compliment, but not has been configured with feature");
    ++mismatches_;
    return false;
  }

  // Pass
  return true;
}

std::unique_ptr<TestCase> TestCaseRegistry::construct_test(
    const std::string& name) {
  auto it = b_.find(name);
  if (it == b_.end()) {
    return nullptr;
  }
  return it->second->construct();
}

std::unique_ptr<TestCase> TestCaseRegistry::construct_test(
    const std::string_view& name) {
  return construct_test(std::string{name});
}

// clang-format off
#define DECLARE_TESTCASE(__name)                    \
  static const struct TestCaseRegisterer_##__name { \
    explicit TestCaseRegisterer_##__name() {        \
      TEST_REGISTRY.add_testcase<__name>(#__name);    \
    }                                               \
  } __tc_register_##__name {}
// clang-format on

class FullyRandomizedTestCase : public TestCase {
 public:
  explicit FullyRandomizedTestCase() : TestCase("FullyRandomizedTestCase") {}

  // Parameters:

  // Trial count
  std::size_t param_n = 100;

  // Probability of a unary-encoding value.
  float param_unary_prob = 0.1f;

  // Probability of a complimented unary-encoding value.
  float param_compliment_prob = 0.5f;

  bool pass() const noexcept override { return false; }

  bool run(DesignBase* b) override {
    for (std::size_t i = 0; i < param_n; i++) {
      if (!run_one_trial(b)) {
        return false;
      }
    }
    // generate_non_unary
    return true;
  }

 private:
  bool run_one_trial(DesignBase* b) {
    bool pass = false;
    if (RANDOM.random_bool(param_unary_prob)) {
      // Unary-vector

      // TODO: double-check this.
      bool compliment = RANDOM.random_bool(param_compliment_prob);
      std::size_t n = RANDOM.uniform(StimulusVector::size() - 2);
      pass = check(b, generate_unary(n, compliment));
    } else {
      // Random, non-unary vector.
      auto [success, v] = generate_non_unary();
      if (success) {
        pass = check(b, v);
      }
    }
    return pass;
  }
};
DECLARE_TESTCASE(FullyRandomizedTestCase);

class DirectedExhaustiveTestCase : public TestCase {
 public:
  explicit DirectedExhaustiveTestCase(bool is_compliment = false)
      : TestCase("DirectedExhaustiveTestCase"), is_compliment_(is_compliment) {}

  bool run(DesignBase* b) override {
    // Check boundary all-one/-zero case.
    if (!zero_case(b)) return false;

    // Exhaustively check all possible unary encodings.
    if (!all_valid_unary_cases(b)) return false;

    // Pass
    return true;
  }

 private:
  bool zero_case(DesignBase* b) {
    const StimulusVector zero_case = 
      is_compliment_ ? StimulusVector::all_ones() : StimulusVector::all_zeros();
    return check(b, zero_case);
  }

  bool all_valid_unary_cases(DesignBase* b) {
    for (std::size_t i = 0; i < StimulusVector::size(); ++i) {
      const StimulusVector v{generate_unary(i, is_compliment_)};
      if (!check(b, v)) {
        return false;
      }
    }
    return true;
  }

  bool is_compliment_;
};
DECLARE_TESTCASE(DirectedExhaustiveTestCase);

}  // namespace tb