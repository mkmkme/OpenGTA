#ifndef LOCAL_EXCEPTIONS_OH_WHAT_JOY
#define LOCAL_EXCEPTIONS_OH_WHAT_JOY

#include <exception>
#include <iostream>
#include <fmt/format.h>

namespace Util {

struct LocalException : public std::exception {
    LocalException(const std::string &f,
                   const size_t l,
                   const std::string &n,
                   const std::string &m)
        : msg { fmt::format("{} ({}:{}): {}", n, f, l, m) }
    {}
    ~LocalException() noexcept override = default;
    [[nodiscard]] const char *what() const noexcept override { return msg.c_str(); }
    std::string msg;
};

struct FileNotFound : public LocalException {
    FileNotFound(const char *f, const size_t l, const std::string &_msg)
        : LocalException(f, l, "FileNotFound", _msg)
    {}
};

struct IOError : public LocalException {
    IOError(const char *f, const size_t l, const std::string &_msg)
        : LocalException(f, l, "IOError", _msg)
    {}
};

struct InvalidFormat : public LocalException {
    InvalidFormat(const char *f, const size_t l, const std::string &_msg)
        : LocalException(f, l, "InvalidFormat", _msg)
    {}
};

struct UnknownKey : public LocalException {
    UnknownKey(const char *f, const size_t l, const std::string &_msg)
        : LocalException(f, l, "UnknownKey", _msg)
    {}
};

struct OutOfRange : public LocalException {
    OutOfRange(const char *f, const size_t l, const std::string &_msg)
        : LocalException(f, l, "OutOfRange", _msg)
    {}
};

struct OutOfMemory : public LocalException {
    OutOfMemory(const char *f, const size_t l, const std::string &_msg)
        : LocalException(f, l, "OutOfMemory", _msg)
    {}
};

struct ScriptError : public LocalException {
    ScriptError(const char *f, const size_t l, const std::string &_msg)
        : LocalException(f, l, "ScriptError", _msg)
    {}
};

struct NotSupported : public LocalException {
    NotSupported(const char *f, const size_t l, const std::string &_msg)
        : LocalException(f, l, "NotSupported", _msg)
    {}
};
} // namespace Util

// to avoid the need for the namespace when writing catch-all blocks
typedef Util::LocalException Exception;

#ifdef _WIN32
#undef E_OUTOFMEMORY
#endif

// to autofill line+file information where the exception was created
#define E_FILENOTFOUND(m) Util::FileNotFound(__FILE__, __LINE__, m)
#define E_IOERROR(m) Util::IOError(__FILE__, __LINE__, m)
#define E_INVALIDFORMAT(m) Util::InvalidFormat(__FILE__, __LINE__, m)
#define E_UNKNOWNKEY(m) Util::UnknownKey(__FILE__, __LINE__, m)
#define E_OUTOFRANGE(m) Util::OutOfRange(__FILE__, __LINE__, m)
#define E_OUTOFMEMORY(m) Util::OutOfMemory(__FILE__, __LINE__, m)
#define E_SCRIPTERROR(m) Util::ScriptError(__FILE__, __LINE__, m)
#define E_NOTSUPPORTED(m) Util::NotSupported(__FILE__, __LINE__, m)

#endif
