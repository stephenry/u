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

#include <string_view>
#include <vector>
#include <iostream>
#include <memory>
#include <string>

#include "Vu.h"

struct Options {
    static Options from_args(const std::vector<std::string_view>& vs);

    std::size_t n;
};

Options Options::from_args(const std::vector<std::string_view>& vs) {
    auto help = []() {
        std::cout << R"(
Usage: Unary-/Thermometer admission circuit testbench driver.

Arguments:

  -n        : (Integer) Number of random trials
  -h/--help : Print Options.
    )";
    };
    Options o;
    for (std::size_t i = 1; i < vs.size(); i++) {
        if (vs[i] == "-n") {
            o.n = stoull(std::string{vs.at(++i)});
        } else if (vs[i] == "-h" || vs[i] == "--help") {
            help();
            std::exit(1);
        } else {
            std::cout << "Invalid command line option: " << vs[i] << "\n";
            help();
            std::exit(1);
        }
    }
    return o;
}

class TB {
public:
    explicit TB(const Options& opts);

    int run();

private:
    std::unique_ptr<Vu> u_;
    Options opts_;
};

TB::TB(const Options& opts)
    : opts_(opts)
{}

int TB::run() {
    return 0;
}

int main(int arg, const char** argv) {
    int ret = 0;
    try {
        const std::vector<std::string_view> vs{argv, argv + arg};
        const Options opts{Options::from_args(vs)};
        TB tb{opts};
        ret = tb.run();
    } catch (std::exception& ex) {
        ret = 1;
    }
    return ret;
}