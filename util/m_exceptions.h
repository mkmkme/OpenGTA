#ifndef LOCAL_EXCEPTIONS_OH_WHAT_JOY
#define LOCAL_EXCEPTIONS_OH_WHAT_JOY

#include <exception>
#include <format>

namespace Util {

struct Exception : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

struct FileNotFound : public Exception {
    explicit FileNotFound(const std::string &file)
        : Exception(std::format("File not found: {}", file))
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

#endif
