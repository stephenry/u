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

#include <iostream>
#include <memory>
#include <vector>
#include <iostream>

#include "designs.h"
#include "random.h"
#include "tb.h"

namespace tb {

class DesignRunner {
 public:
  explicit DesignRunner() = default;
  virtual ~DesignRunner() = default;

  virtual void run(DesignBase* d) = 0;
};

class Test {
 public:
  void run() {
    // opts_.program->run(d.get());
  }

};

struct OptionsInitializer {
  explicit OptionsInitializer(int argc, const char** argv, std::ostream& os = std::cerr);

 private:
  void build(std::vector<std::string_view>& args, std::ostream& os);
  void help() const;
};

OptionsInitializer::OptionsInitializer(int argc, const char** argv, std::ostream& os) {
  std::vector<std::string_view> vs{argv, argv + argc};
  build(vs, os);
}

void OptionsInitializer::build(std::vector<std::string_view>& args, std::ostream& os) {
  for (auto it = args.begin(); it != args.end(); ++it) {
    std::string_view arg{*it};

    // Helper lambda to check presence of next argument and fail if absent.
    auto check_next_argument = [&](auto it) {
      if (++it == args.end()) {
        os << "Argument " << arg << " expects an argument." << std::endl;
        std::exit(1);
      }
    };

    // Parse arguments.
    if (arg == "--list_designs") {
      for (const std::string& design : DESIGN_REGISTRY.designs()) {
        std::cout << design << std::endl;
      }
      std::exit(1);
    } else if (arg == "-s" || arg== "--seed") {
      check_next_argument(it);
      RANDOM.seed(std::stoull(std::string{*++it}));
    } else if (arg == "-v" || arg == "--verbose") {
      OPTIONS.verbose = true;
    } else if (arg == "--adcomp") {
      OPTIONS.admits_compliment = true;
    } else if (arg == "-h" || arg == "--help") {
      help();
    } else {
      os << "Invalid command line option: " << arg << "\n";
      help();
    }
  }
}

void OptionsInitializer::help() const {
  std::cout << R"(
Usage: Unary-/Thermometer admission circuit testbench driver.

Arguments:

  -h/--help            : Print Options.
     --list_designs    : List available designs
     --adcomp          : Admits compliment
  -s/--seed <integer>  : (Integer) Randomization seed
  -v/--verbose         : Verbosity
  )";
  std::exit(1);
}

}  // namespace tb

int main(int argc, const char** argv) {
  tb::OptionsInitializer ob{argc, argv};
  return 0;
}
