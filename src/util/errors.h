#pragma once

#include <format>

namespace Util {

struct Exception : public std::runtime_error {
    using std::runtime_error::runtime_error;

    template <typename... Args>
    explicit Exception(const std::format_string<Args...> &fmt, Args &&...args)
        : std::runtime_error(std::format(fmt, std::forward<Args>(args)...))
    {
    }
};

struct FileNotFound : public Exception {
    explicit FileNotFound(const std::string &file)
        : Exception(std::format("File not found: {}", file)) {}
};

struct IOError : public Exception {
    using Exception::Exception;
};

struct InvalidFormat : public Exception {
    using Exception::Exception;
};

struct UnknownKey : public Exception {
    using Exception::Exception;
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
} // namespace Util
