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

#include "tb.h"

#include <iostream>
#include <memory>
#include <vector>
#include <iterator>
#include "verilated_vcd_c.h"

#include "designs.h"
#include "random.h"
#include "tests.h"

namespace tb {

class Scenario {
 public:
  explicit Scenario() = default;

  bool has_design() const noexcept { return (d_ != nullptr); }

  bool has_test() const noexcept { return !ts_.empty(); }

  bool is_valid() const noexcept { return (has_design() && has_test()); }

  void set(std::unique_ptr<DesignBase>&& d) { d_ = std::move(d); }

  void add(std::unique_ptr<TestCase>&& t) { ts_.push_back(std::move(t)); }

  void run();

  TestCase* head() const {
    if (ts_.empty())
      return nullptr;

    return ts_.back().get();
  }

 private:
  // Unit under test.
  std::unique_ptr<DesignBase> d_;

  // Tests to run on design.
  std::vector<std::unique_ptr<TestCase> > ts_;
};

void Scenario::run() {
  for (auto& t : ts_) {
    U_LOG_INFO("Scenario: design=\"", d_->name(), "\" test=\"", t->name(), "\"");
    t->run(d_.get());
  }
}

class Program {
 public:
  explicit Program() = default;

  void add(std::unique_ptr<Scenario>&& s) { s_.push_back(std::move(s)); }

  void run();

 private:
  void run_scenario(Scenario* s);

  std::vector<std::unique_ptr<Scenario> > s_;
};

void Program::run() {
  for (std::unique_ptr<Scenario>& s : s_) {
    run_scenario(s.get());
  }
}

void Program::run_scenario(Scenario* s) {
  // Run-test on current design.
  s->run();
}

struct DriverRuntime {
  explicit DriverRuntime(int argc, const char** argv,
                         std::ostream& os = std::cerr);

  int run() const;
  int status() const { return 0; }

 private:
  void build(std::vector<std::string_view>& args, std::ostream& os);
  void help() const;
  void parse_test_arg_string(const std::string_view vs);

  std::unique_ptr<Program> p_;
};

DriverRuntime::DriverRuntime(int argc, const char** argv, std::ostream& os) {
  p_ = std::make_unique<Program>();
  std::vector<std::string_view> vs{argv, argv + argc};
  build(vs, os);
  U_LOG_INFO("Command line: ", join(vs.begin(), vs.end()));
}

int DriverRuntime::run() const {
  p_->run();
  return status();
}

void DriverRuntime::build(std::vector<std::string_view>& args,
                          std::ostream& os) {
  for (std::size_t i = 1; i < args.size(); ++i) {
    const std::string_view arg{args[i]};

    // Helper lambda to check presence of next argument and fail if absent.
    auto check_next_argument = [&]() {
      if ((i + 1) < args.size()) return;

      os << "Argument " << arg << " expects an argument." << std::endl;
      std::exit(1);
    };

    // Parse arguments.
    if (arg == "--list_designs") {
      std::vector<std::string> vs;
      DESIGN_REGISTRY.designs(std::back_inserter(vs));
      for (const std::string& design : vs) {
        std::cout << design << std::endl;
      }
      std::exit(0);
    } else if (arg == "--list_tests") {
      std::vector<std::string> vs;
      TEST_REGISTRY.tests(std::back_inserter(vs));
      for (const std::string& test : vs) {
        std::cout << test << std::endl;
      }
      std::exit(0);
    } else if (arg == "-s" || arg == "--seed") {
      check_next_argument();
      RANDOM.seed(std::stoull(std::string{args[++i]}));
    } else if (arg == "-v" || arg == "--verbose") {
      check_next_argument();
      OPTIONS.verbosity_n = stoull(std::string{args[++i]});
    } else if (arg == "-d" || arg == "--debug") {
      OPTIONS.log = std::make_unique<Log>();
      OPTIONS.log->set_debug(true);
      OPTIONS.debug = true;
    } else if (arg == "-t" || arg == "--test") {
      check_next_argument();
      parse_test_arg_string(args[++i]);
    } else if (arg == "-h" || arg == "--help") {
      help();
    } else if (arg == "--vcd") {
      OPTIONS.vcd_en = true;
    } else {
      os << "Invalid command line option: " << arg << "\n";
      help();
    }
  }
}

void DriverRuntime::parse_test_arg_string(const std::string_view vs) {
  std::unique_ptr<Scenario> s = std::make_unique<Scenario>();
  const std::vector<std::string_view>& vss{split(vs, ',')};
  for (auto it = vss.begin(); it != vss.end(); ++it) {
    if (it->starts_with("d=") || it->starts_with("design=")) {
      // Construct new design
      auto [ok, k, v] = split_kv(*it);
      if (!ok) {
        // throw: malformed argument list.
      }
      auto design = DESIGN_REGISTRY.construct_design(v);
      if (!design) {
        // throw: unknown design name.
      }
      // Design is constructed, add to current scenario.
      s->set(std::move(design));
    } else if (it->starts_with("t=") || it->starts_with("test=")) {
      // Construct new test for design.
      if (!s->has_design()) {
        // Throw: no design defined. Cannot attach test.
      }
      auto [ok, k, v] = split_kv(*it);
      if (!ok) {
        // throw: malformed argument list.
      }
      auto test = TEST_REGISTRY.construct_test(v);
      if (!test) {
        // throw: unknown testname.
      }
      // Test is constructed, add to current scenario.
      s->add(std::move(test));
    } else if (it->starts_with("o=") || it->starts_with("options=")) {
      if (!s->has_design()) {
        // throw: no design present in scenario.
      }
      if (!s->has_test()) {
        // throw: no test present in scenario.
      }
      auto [ok, k, v] = split_kv(*it);
      if (!ok) {
        // throw: malformed argument list.
      }

      // Otherwise, pass arguments to test
      s->head()->config(v);
    } else {
      // throw: unknown argument.
    }
  }
  if (!s->is_valid()) {
    // throw: malformed scenario.
  }
  // Otherwise, add this to the list of scenarios to run.
  p_->add(std::move(s));
}

void DriverRuntime::help() const {
  std::cout << R"(
Usage: Unary-/Thermometer admission circuit testbench driver.

Arguments:

  -h/--help            : Print Options.
     --list_designs    : List available designs
  -s/--seed <integer>  : (Integer) Randomization seed
  -v/--verbose         : Verbosity
     --vcd             : Enable VCD tracing.
  )";
  std::exit(1);
}

}  // namespace tb

int main(int argc, const char** argv) {
  tb::DriverRuntime rt{argc, argv};
  return rt.run();
}
