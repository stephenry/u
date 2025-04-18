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

#ifndef TB_TESTS_H
#define TB_TESTS_H

#include <memory>
#include <string>
#include <unordered_map>

#include "stimulus.h"

namespace tb {

// Forwards:
class DesignBase;

class TestCase {
 public:
  explicit TestCase(const std::string& name)
    :  name_(name), mismatches_(0)
  {}
  virtual ~TestCase() = default;

  virtual const std::string& name() const noexcept { return name_; }
  virtual bool pass() const noexcept { return (mismatches_ != 0); };
  virtual bool fail() const noexcept { return !pass(); }

  virtual void config(const std::string_view& sv) {}

  virtual bool run(DesignBase* b) = 0;

 protected:
  bool check(DesignBase* b, const StimulusVector& v);

 private:
  std::string name_;
  std::size_t mismatches_;
};

inline class TestCaseRegistry {
  struct TestCaseBuilderBase {
    explicit TestCaseBuilderBase() = default;
    virtual ~TestCaseBuilderBase() = default;

    virtual std::unique_ptr<TestCase> construct() const noexcept = 0;
  };

  template <typename T>
  struct TestCaseBuilder : public TestCaseBuilderBase {
    std::unique_ptr<TestCase> construct() const noexcept override {
      return std::unique_ptr<TestCase>(new T{});
    }
  };

 public:
  explicit TestCaseRegistry() = default;

  std::unique_ptr<TestCase> construct_test(const std::string& name);
  std::unique_ptr<TestCase> construct_test(const std::string_view& name);

  template <typename T>
  void add_testcase(const std::string& name) {
    if (b_.find(name) == b_.end()) {
      b_[name] = std::unique_ptr<TestCaseBuilderBase>(new TestCaseBuilder<T>{});
    }
  }

 private:
  std::unordered_map<std::string, std::unique_ptr<TestCaseBuilderBase> > b_;
} TEST_REGISTRY;

}  // namespace tb

#endif