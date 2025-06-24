#pragma once

#include <format>
#include <stacktrace>

namespace Util {

struct Exception : public std::exception {

    template <typename... Args>
    explicit Exception(const std::format_string<Args...> &fmt, Args &&...args)
        : what_(std::format(fmt, std::forward<Args>(args)...))
        , stacktrace_(std::stacktrace::current())
    {
    }

    [[nodiscard]] const char *what() const noexcept override
    {
        return what_.c_str();
    }

    [[nodiscard]] const std::stacktrace &stacktrace() const noexcept
    {
        return stacktrace_;
    }

private:
    std::string what_;
    std::stacktrace stacktrace_;
};

struct FileNotFound : public Exception {
    using Exception::Exception;

    explicit FileNotFound(const std::string &file)
        : Exception("File not found: {}", file) {}
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
