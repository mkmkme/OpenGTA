#pragma once

#include <format>

#include "base/config.h"
#ifdef OGTA_STD_STACKTRACE_AVAILABLE
#include <stacktrace>
#endif

namespace Util {

struct Exception : public std::exception {

    template <typename... Args>
    explicit Exception(const std::format_string<Args...> &fmt, Args &&...args)
        : what_(std::format(fmt, std::forward<Args>(args)...))
#ifdef OGTA_STD_STACKTRACE_AVAILABLE
        , stacktrace_(std::stacktrace::current())
#endif
    {
    }

    [[nodiscard]] const char *what() const noexcept override { return what_.c_str(); }

#ifdef OGTA_STD_STACKTRACE_AVAILABLE
    [[nodiscard]] const std::stacktrace &stacktrace() const noexcept { return stacktrace_; }
#endif

private:
    std::string what_;
#ifdef OGTA_STD_STACKTRACE_AVAILABLE
    std::stacktrace stacktrace_;
#endif
};

struct FileNotFound : public Exception {
    using Exception::Exception;

    explicit FileNotFound(const std::string &file)
        : Exception("File not found: {}", file)
    {
    }
};

struct IOError : public Exception {
    using Exception::Exception;
};

struct InvalidFormat : public Exception {
    using Exception::Exception;
};

struct UnknownKey : public Exception {
    using Exception::Exception;
    template <typename T>
    explicit UnknownKey(const T &key)
        : Exception("Unknown key: {}", key)
    {
    }
};

struct OutOfRange : public Exception {
    using Exception::Exception;
};

struct ScriptError : public Exception {
    using Exception::Exception;
};

struct NotSupported : public Exception {
    using Exception::Exception;
};

void enableBacktraces();
} // namespace Util
