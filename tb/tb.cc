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

#include "designs.h"
#include "random.h"

namespace tb {

class DesignRunner {
 public:
  explicit DesignRunner() = default;
  virtual ~DesignRunner() = default;

  virtual void run(DesignBase* d) = 0;
};

struct Options {
  std::unique_ptr<DesignRunner> r;
  std::string design_name = "e";
  std::size_t n = 1000;
  bool verbose = false;
};

class Test {
 public:
  void run() {
    std::unique_ptr<DesignBase> d =
        DESIGN_REGISTRY.construct_design(opts_.design_name);
    // opts_.program->run(d.get());
  }

 private:
  const Options opts_;
};

struct OptionsBuilder {
  struct Exception {
    explicit Exception(const std::string& what) : what_(what) {}
    const std::string& what() const { return what_; }

   private:
    std::string what_;
  };

  explicit OptionsBuilder(int argc, const char** argv);

  Options build() const;

 private:
  void help() const;
  void scan_arguments(Options& opts) const;

  std::vector<std::string_view> vs_;
};

OptionsBuilder::OptionsBuilder(int argc, const char** argv)
    : vs_(argv, argv + argc) {}

Options OptionsBuilder::build() const {
  if (vs_.empty()) {
    return Options{};
  }

  Options opts;
  scan_arguments(opts);
  return opts;
}

void OptionsBuilder::scan_arguments(Options& opts) const {
  for (std::size_t i = 1; i < vs_.size(); i++) {
    if (vs_[i] == "-n") {
      opts.n = std::stoull(std::string{vs_.at(++i)});
    } else if (vs_[i] == "--list_designs") {
      for (const std::string& design : DESIGN_REGISTRY.designs()) {
        std::cout << design << std::endl;
      }
      std::exit(1);
    } else if (vs_[i] == "-s" || vs_[i] == "--seed") {
      RANDOM.seed(std::stoull(std::string{vs_.at(++i)}));
    } else if (vs_[i] == "-v" || vs_[i] == "--verbose") {
      opts.verbose = true;
    } else if (vs_[i] == "-h" || vs_[i] == "--help") {
      help();
    } else {
      std::cout << "Invalid command line option: " << vs_[i] << "\n";
      help();
    }
  }
}

void OptionsBuilder::help() const {
  std::cout << R"(
        Usage: Unary-/Thermometer admission circuit testbench driver.
        
        Arguments:
        
          -h/--help            : Print Options.
             --list_designs    : List available designs
          -s/--seed <integer>  : (Integer) Randomization seed
          -v/--verbose         : Verbosity
            )";
  std::exit(1);
}

}  // namespace tb

int main(int argc, const char** argv) {
  int ret = 0;
  try {
    const tb::OptionsBuilder ob{argc, argv};
    auto b{ob.build()};
  } catch (const tb::OptionsBuilder::Exception& ex) {
    std::cout << "Error: " << ex.what() << std::endl;
    ret = 1;
  }
  return ret;
}