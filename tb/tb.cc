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

#include <string>
#include <string_view>
#include <vector>
#include <iostream>
#include <memory>
#include <string>
#include <algorithm>
#include <unordered_map>

#include "tb.h"

struct DesignBase {
    virtual ~DesignBase() = default;
};

template<typename T>
struct Design : DesignBase {
public:
    explicit Design() = default;
private:
    std::unique_ptr<T> uut_;
};

static class DesignRegistry {
    class DesignBuilderBase {
    public:
        explicit DesignBuilderBase() = default;
        virtual ~DesignBuilderBase() = default;

        virtual std::unique_ptr<DesignBase> construct() const = 0;
    };

    template<typename T>
    class DesignBuilder : public DesignBuilderBase {
    public:
        explicit DesignBuilder() = default;
        std::unique_ptr<DesignBase> construct() const override {
            return std::make_unique<T>();
        }
    };
public:
    explicit DesignRegistry() = default;

    template<typename T>
    void add_design(const std::string& name) {
        if (designs_.find(name) != designs_.end()) {
            designs_[name] = std::unique_ptr<DesignBuilderBase>(
                new DesignBuilder<Design<T>>{});
        }
    }

    std::unique_ptr<DesignBase> construct_design(const std::string& name) {
        if (auto it = designs_.find(name); it != designs_.end()) {
            return it->second->construct();
        }
        return nullptr;
    }

private:
    std::unordered_map<std::string, std::unique_ptr<DesignBuilderBase>> designs_;
} DESIGN_REGISTRY;

#define DECLARE_DESIGN(__name) \
    struct DesignRegister##__name { \
        DesignRegister##__name() { \
            DESIGN_REGISTRY.add_design<V##__name>(#__name); \
        } \
    } register_##__name;

#include "VObj_u/Vu.h"
DECLARE_DESIGN(u);

#include "VObj_e/Ve.h"
DECLARE_DESIGN(e);

class Program {
public:
    explicit Program() = default;
    virtual ~Program() = default;

    virtual void run(DesignBase* d) = 0;
};

struct Options {
    std::unique_ptr<Program> program;
    std::string design_name = "e";
    std::size_t n = 1000;
};

class Test {
public:
    void run() {
        std::unique_ptr<DesignBase> d =
            DESIGN_REGISTRY.construct_design(opts_.design_name);
        opts_.program->run(d.get());
    }

private:
    const Options opts_;
};

struct OptionsBuilder {
    struct Exception {
        explicit Exception(const std::string& what)
            : what_(what) {}
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
    : vs_(argv + 1, argv + argc)
{}

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
        
          -n        : (Integer) Number of random trials
          -h/--help : Print Options.
            )";
    std::exit(1);
}


int main(int argc, const char** argv) {
    int ret = 0;
    try {
        const OptionsBuilder ob{argc, argv};
    } catch (const OptionsBuilder::Exception& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        ret = 1;
    }
    return ret;
}