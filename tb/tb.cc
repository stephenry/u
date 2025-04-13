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
#include <array>
#include <tuple>
#include <random>
#include <limits>

#include "tb.h"
#include "verilated.h"

constexpr std::size_t ceil(std::size_t n, std::size_t d) {
    return (n + (d - 1)) / d;
}

template<typename T, std::size_t W>
constexpr T mask() {
    return (T{1} << W) - 1;
}

static class Random {
public:
    using seed_type = std::mt19937::result_type;
   
    explicit Random(seed_type s = seed_type{}) { seed(s); }
   
    // Set seed of randomization engine.
    void seed(seed_type s) { mt_.seed(s); }
   
    // Generate a random integral type in range [lo, hi]
    template<typename T>
    T uniform(T hi = std::numeric_limits<T>::max(), T lo = std::numeric_limits<T>::min()) {
        static_assert(std::is_integral_v<T> || std::is_floating_point_v<T>);
        if constexpr (std::is_integral_v<T>) {
            // Integral type
            std::uniform_int_distribution<T> d(lo, hi);
            return d(mt_);
        } else {
            // Floating-point type
            std::uniform_real_distribution<T> d(lo, hi);
            return d(mt_);     
        }
    }

    bool random_bool(float t_prob = 0.5f) {
        std::bernoulli_distribution b(t_prob);
        return b(mt_);
    }
private:
    std::mt19937 mt_;
} RANDOM;

template<std::size_t W, typename T = vluint8_t>
class VBitVector {
    static constexpr std::size_t size_in_bits_n = W;
    static constexpr std::size_t size_in_bytes_n = ceil(size_in_bits_n, 8);
    static constexpr std::size_t bits_in_tail_n = (W % sizeof(T));
public:
    using value_type = T;

    static VBitVector<W> all_zeros() {
        return VBitVector<W>{};
    }

    static VBitVector<W> all_ones() {
        VBitVector<W> v{};
        for (std::size_t i = 0; i < W; i++) {
            v.bit(i, true);
        }
        return v;
    }

    explicit VBitVector() {
        clear();
    }

    explicit VBitVector(vluint8_t *d, std::size_t n) {
        clear();
        std::copy_n(d, std::min(n, size_bytes_n()), v_.data());
    }

    static constexpr std::size_t size() noexcept { return size_in_bits_n; }
    static constexpr std::size_t size_bytes_n() noexcept { return size_in_bytes_n; }

    void clear() noexcept {
        std::fill(v_.begin(), v_.end(), 0);
    }

    void clean() noexcept {
        if constexpr (bits_in_tail_n > 0) {
            v_.back() = v_.back() & mask<vluint8_t, bits_in_tail_n>();
        }
    }

    void bit(std::size_t i, bool b = true) noexcept {
        vluint8_t mask = (1 << (i & 0x7));
        if (b) {
            // set bit
            v_[i >> 3] |=   mask;
        } else {
            // clear bit
            v_[i >> 3] &= (~mask);
        }
    }

    bool bit(std::size_t i) const noexcept {
        const std::size_t byte = (i >> 3);
        if (v_.size() <= byte) {
            // Infinite zero-extend.
            return false;
        }
        return (v_[byte] & (1ul << (i & 0x7))) != 0;
    }

    void value(std::size_t i, value_type v) {
        v_[i] = v;
    }

#define VERILATOR_PORT_TYPES(__func) \
    __func(vluint32_t)

#define DECLARE_VERILATOR_PORT_CONVERTER(__type) \
    void to_verilated(__type& t) const noexcept { \
        to_verilated_impl(reinterpret_cast<vluint8_t*>(std::addressof(t))); \
    }
    VERILATOR_PORT_TYPES(DECLARE_VERILATOR_PORT_CONVERTER)

#undef DECLARE_VERILATOR_PORT_CONVERTER

protected:
    void to_verilated_impl(value_type* b) const noexcept {
        std::copy(v_.begin(), v_.end(), b);
    }
    std::array<value_type, size_in_bytes_n> v_;
};

class VBit : public VBitVector<1> {
public:
    static VBit from_verilated(vluint8_t t) {
        return VBit{t != 0};
    }

    explicit VBit(bool b) {
        v_[0] = b ? 0b1 : 0b0;
    }

    bool to_bool() const noexcept {
        return (v_[0] != 0);
    }
};

using StimulusVector = VBitVector<RTL_PARAM__W>;

template<std::size_t W>
std::tuple<bool, bool> is_unary(const VBitVector<W>& b) {
    std::size_t edges = 0, zeros = 0, ones = 0;
    for (std::size_t i = 1; i < b.size(); ++i) {
        if (b.bit(i)) {
            ++ones;
        } else {
            ++zeros;
        }

        if (b.bit(i) ^ b.bit(i - 1)) {
            ++edges;
        }
    }

    bool is_compliment = b.bit(W - 1);
    bool is_unary = 
        (is_compliment ? (ones == W) : (zeros == W)) || (edges == 1);

    return {is_unary, is_compliment};
}

StimulusVector generate_unary(std::size_t n, bool compliment = false) {
    StimulusVector v;
    for (std::size_t i = 0; i < v.size(); i++) {
        if (i < n) {
            v.bit(i, !compliment);
        } else {
            v.bit(i,  compliment);
        }
    }
    return v;
}

std::tuple<bool, StimulusVector> generate_non_unary(std::size_t rounds_n = 1) {
    while (rounds_n--) {
        StimulusVector v;
        for (std::size_t i = 0; i < v.size_bytes_n(); i++) {
            v.value(i, RANDOM.uniform<StimulusVector::value_type>());
        }
        v.clean();

        if (auto [unary, compliment] = is_unary(v); unary) {
            return {true, v};
        }

        // Otherwise, we've somehow hit a unary integer. Repeat until success.
    }

    // Pathological case where we've been unable to generate a non-unary case.
    // Unlikely to ever to occur outside of exceptional cases.
    return {false, StimulusVector{}};
}

struct DesignBase {
    virtual ~DesignBase() = default;

    // Evaluate verilated module with stimulus 'v' and return admission
    // decision.
    virtual bool is_unary(const StimulusVector& v) noexcept = 0;
};

template<typename T>
struct Design : DesignBase {
public:
    explicit Design() = default;

    bool is_unary(const StimulusVector& v) noexcept override {
        // Drive input
        v.to_verilated(uut_->i_x);
        // Evaluate
        uut_->eval();
        // Return response.
        return VBit::from_verilated(uut_->o_is_unary).to_bool();
    }
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

    std::vector<std::string> designs() const {
        std::vector<std::string> vs;
        vs.reserve(designs_.size());
        for (auto& [k, v] : designs_) {
            vs.push_back(k);
        }
        return vs;
    }

    template<typename T>
    void add_design(const std::string& name) {
        if (designs_.find(name) == designs_.end()) {
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
    std::unordered_map<std::string,
        std::unique_ptr<DesignBuilderBase>> designs_;
} DESIGN_REGISTRY;

#define DECLARE_DESIGN(__name) \
    static const struct DesignRegister##__name { \
        explicit DesignRegister##__name() { \
            DESIGN_REGISTRY.add_design<V##__name>(#__name); \
        } \
    } __register_##__name{}

#include "VObj_u/Vu.h"
DECLARE_DESIGN(u);

#include "VObj_e/Ve.h"
DECLARE_DESIGN(e);

class TestCase {
public:
    explicit TestCase()
        : mismatches_(0)
    {}

    virtual ~TestCase() = default;

    virtual bool pass() const noexcept { return (mismatches_ != 0); };
    virtual bool fail() const noexcept { return !pass(); }

    virtual bool run(DesignBase* b) = 0;

protected:
    bool check(DesignBase* b, const StimulusVector& v) {
        bool rtl_is_unary = b->is_unary(v);
        auto [beh_is_unary, beh_is_compliment] = is_unary(v);

        if (rtl_is_unary == beh_is_unary) {
            ++mismatches_;
            return false;
        }

        // TODO: Check compliment.
        return true;
    }

private:
    std::size_t mismatches_;
};

class FullyRandomizedTestCase : public TestCase {
public:
    explicit FullyRandomizedTestCase() = default;

    // Parameters:

    // Trial count
    std::size_t param_n = 0;

    // Probability of a unary-encoding value.
    float param_unary_prob = 0.1f;

    // Probability of a complimented unary-encoding value.
    float param_compliment_prob = 0.5f;

    bool pass() const noexcept override { return false; }

    bool run(DesignBase* b) {
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

class DirectedExhaustiveTestCase : public TestCase {
public:
    explicit DirectedExhaustiveTestCase(bool is_compliment = false)
        : is_compliment_(is_compliment)
    {}

    bool run(DesignBase* b) override {
        // Check boundary all-one/-zero case.
        if (!zero_case(b))
            return false;

        // Exhaustively check all possible unary encodings.
        if (!all_valid_unary_cases(b))
            return false;

        // Pass
        return true;
    }

private:
    bool zero_case(DesignBase* b) {
        const StimulusVector zero_case = is_compliment_
            ? StimulusVector::all_ones() : StimulusVector::all_zeros();
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

class DesignRunner {
public:
    explicit DesignRunner() = default;
    virtual ~DesignRunner() = default;

    virtual void run(DesignBase* d) = 0;

    std::vector<std::unique_ptr<TestCase> > tcs_;
};

struct Options {
    std::unique_ptr<DesignRunner> r;
    std::string design_name = "e";
    std::size_t n = 1000;
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
    : vs_(argv, argv + argc)
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
    bool terminate = false;
    for (std::size_t i = 1; i < vs_.size(); i++) {
        if (vs_[i] == "-n") {
            opts.n = std::stoull(std::string{vs_.at(++i)});
        } else if (vs_[i] == "--list_designs") {
            for (const std::string& design : DESIGN_REGISTRY.designs()) {
                std::cout << design << std::endl;
            }
            terminate = true;
            std::exit(1);
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
        
          -n                : (Integer) Number of random trials
          -h/--help         : Print Options.
             --list_designs : List available designs
            )";
    std::exit(1);
}


int main(int argc, const char** argv) {
    int ret = 0;
    try {
        const OptionsBuilder ob{argc, argv};
        auto b{ob.build()};
    } catch (const OptionsBuilder::Exception& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        ret = 1;
    }
    return ret;
}