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

#include "log.h"
#include "tb.h"

namespace tb {

std::string_view to_string(Log::Level l) {
    switch (l) {
    case Log::Level::Debug: return "Debug";
    case Log::Level::Info: return "Info";
    case Log::Level::Warning: return "Warning";
    case Log::Level::Error: return "Error";
    case Log::Level::Fatal: return "Fatal";
    default: return "Invalid";
    }
}

std::string_view to_string1(Log::Level l) {
    return std::string_view{to_string(l).substr(0,1)};
}

void Log::message(const Message& m) {
    switch (m.l) {
    case Log::Level::Warning:
        ++OPTIONS.warnings_n;
        break;
    case Log::Level::Error:
        ++OPTIONS.errors_n;
        break;
    case Log::Level::Fatal:
        break;
    default:
        break;
    }
    os_ << to_string1(m.l) << ": " << m.msg.str() << std::endl;
}

}  // namespace tb
