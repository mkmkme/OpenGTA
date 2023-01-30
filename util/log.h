#ifndef LOG_FUNCS_H
#define LOG_FUNCS_H
#include <fmt/color.h>
#include <fmt/core.h>
#include <iostream>
#include <utility>

#ifdef WIN32
#undef ERROR
#endif

#define DEBUG(...) Util::Log::_debug(__FILE__, __LINE__, __VA_ARGS__)
#define INFO(...) Util::Log::_info(__FILE__, __LINE__, __VA_ARGS__)
#define WARN(...) Util::Log::_warn(__FILE__, __LINE__, __VA_ARGS__)
#define ERROR(...) Util::Log::_error(__FILE__, __LINE__, __VA_ARGS__)
#define ERROR_AND_EXIT(ec) \
    error_code = ec;       \
    exit(ec);
#define GL_CHECKERROR                                        \
    {                                                        \
        int _err = glGetError();                             \
        if (_err != GL_NO_ERROR)                             \
            Util::Log::_error(__FILE__,                      \
                              __LINE__,                      \
                              "GL error: {} = {}",           \
                              _err,                          \
                              Util::Log::glErrorName(_err)); \
    }

namespace Util {

enum class LogLevel { error, warn, info, debug };

class Log {
public:
    inline static void setOutputLevel(LogLevel new_level)
    {
        level_ = new_level;
    }

    static const char *glErrorName(int k);

    template <typename... Args>
    static void _log(LogLevel level, const char *file, int line, Args &&...args)
    {
        if (int(level_) < int(level))
            return;

        prefix(level, file, line);
        fmt::print(stderr, std::forward<Args>(args)...);
        fmt::print(stderr, "\n");
    }

    template <typename... Args>
    static void _debug(const char *file, int line, Args &&...args)
    {
        _log(LogLevel::debug, file, line, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void _info(const char *file, int line, Args &&...args)
    {
        _log(LogLevel::info, file, line, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void _warn(const char *file, int line, Args &&...args)
    {
        _log(LogLevel::warn, file, line, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void _error(const char *file, int line, Args &&...args)
    {
        _log(LogLevel::error, file, line, std::forward<Args>(args)...);
    }

private:
    static LogLevel level_;
    static std::ostream emptyStream;

    static void prefix(LogLevel level, const char *file, int line);
    static fmt::color level_color(LogLevel level);
};

} // namespace Util
#endif
