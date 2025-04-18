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

#ifndef TB_LOG_H
#define TB_LOG_H

#include <iostream>
#include <sstream>
#include <type_traits>

namespace tb {

// forwards:
class MessageRenderer;
template <typename T>
class MessageFormatter;

class Log {
  friend class Scope;
 public:
  struct Scope {
    static constexpr std::size_t step_n = 2;
    explicit Scope();
    ~Scope();
  };
  
  enum class Level {
    Debug,
    Info,
    Warning,
    Error,
    Fatal,
  };

  struct Message {
    Level l;
    std::ostringstream msg;
  };

  explicit Log(std::ostream& os = std::cout)
   : os_(os), debug_(false), scope_(0) {}

  void set_debug(bool en) { debug_ = en; }

  // Dispatch message to logger;
  void message(const Message& m);

 private:
  std::ostream& os_;
  bool debug_;
  std::size_t scope_;
};

template <typename T>
class MessageFormatter {
  template <typename U>
  friend class MessageFormatter;

 public:
  explicit MessageFormatter(MessageRenderer& r, const T& t);

  void render() const;
};

class MessageRenderer {
  template <typename T>
  friend class MessageFormatter;

 public:
  explicit MessageRenderer(Log::Level l) { msg_.l = l; }

  const Log::Message& msg() const noexcept { return msg_; }

  template <typename... T>
  void append(T&&... args) {
    append_mf(MessageFormatter<std::decay_t<T>>{*this, args}...);
  }

  Log::Message& msg() noexcept { return msg_; }

 private:
  template <typename T, typename... U>
  void append_mf(const MessageFormatter<T>& f,
                 const MessageFormatter<U>&... fs) {
    append_mf(f);
    append_mf(fs...);
  }

  template <typename T>
  void append_mf(const MessageFormatter<T>& f) {
    f.render();
  }
  Log::Message msg_;
};

template <>
class MessageFormatter<const char*> {
 public:
  explicit MessageFormatter(MessageRenderer& r, const char* t) : r_(r), t_(t) {}

  void render() const {
    std::ostringstream& os{r_.msg().msg};
    os << t_;
  }

 private:
  MessageRenderer& r_;
  const char* t_;
};

template <>
class MessageFormatter<std::string> {
 public:
  explicit MessageFormatter(MessageRenderer& r, const std::string& s)
      : r_(r), s_(s) {}

  void render() const {
    std::ostringstream& os{r_.msg().msg};
    os << s_;
  }

 private:
  MessageRenderer& r_;
  const std::string& s_;
};

template <>
class MessageFormatter<bool> {
 public:
  explicit MessageFormatter(MessageRenderer& r, bool b) : r_(r), b_(b) {}

  void render() const {
    std::ostringstream& os{r_.msg().msg};
    os << (b_ ? "true" : "false");
  }

 private:
  MessageRenderer& r_;
  bool b_;
};

// clang-format off
#define U_LOG_LEVEL(__level, ...)         \
  U_MACRO_BEGIN                           \
  if (::tb::OPTIONS.log) {                \
    ::tb::MessageRenderer r{__level};     \
    r.append(__VA_ARGS__);                \
    ::tb::OPTIONS.log->message(r.msg());  \
  }                                       \
  U_MACRO_END
  
#define U_LOG_SCOPE(__id) ::tb::Log::Scope __log_scope##__id{}

#define U_LOG_DEBUG(...) U_LOG_LEVEL(Log::Level::Debug, __VA_ARGS__)
#define U_LOG_INFO(...) U_LOG_LEVEL(Log::Level::Info, __VA_ARGS__)
#define U_LOG_WARNING(...) U_LOG_LEVEL(Log::Level::Warning, __VA_ARGS__)
#define U_LOG_ERROR(...) U_LOG_LEVEL(Log::Level::Error, __VA_ARGS__)
#define U_LOG_FATAL(...) U_LOG_LEVEL(Log::Level::Fatal, __VA_ARGS__)

// clang-format on

}  // namespace tb

#endif
