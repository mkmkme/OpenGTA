#pragma once

#include <fmt/color.h>
#include <fmt/core.h>
#include <format>
#include <iostream>
#include <string_view>
#include <utility>

#ifdef WIN32
#undef ERROR
#endif

namespace Util {

enum class LogLevel { error, warn, info, debug };

class Log {
public:
    inline static void setOutputLevel(LogLevel new_level) { level_ = new_level; }

    static const char *glErrorName(int k);

    template <typename... Args>
    static void _log(LogLevel level, const char *file, int line, std::string_view format, Args &&...args)
    {
        if (int(level_) < int(level))
            return;

        prefix(level, file, line);
        fmt::print(stderr, "{}", std::vformat(format, std::make_format_args(std::forward<Args>(args)...)));
        fmt::print(stderr, "\n");
    }

private:
    static LogLevel level_;
    static std::ostream emptyStream;

    static void prefix(LogLevel level, const char *file, int line);
    static fmt::color level_color(LogLevel level);
};

#define DEBUG(OGTA_FMT, ...) \
    Util::Log::_log(Util::LogLevel::debug, __FILE__, __LINE__, OGTA_FMT __VA_OPT__(, ) __VA_ARGS__)
#define INFO(OGTA_FMT, ...) \
    Util::Log::_log(Util::LogLevel::info, __FILE__, __LINE__, OGTA_FMT __VA_OPT__(, ) __VA_ARGS__)
#define WARN(OGTA_FMT, ...) \
    Util::Log::_log(Util::LogLevel::warn, __FILE__, __LINE__, OGTA_FMT __VA_OPT__(, ) __VA_ARGS__)
#define ERROR(OGTA_FMT, ...) \
    Util::Log::_log(Util::LogLevel::error, __FILE__, __LINE__, OGTA_FMT __VA_OPT__(, ) __VA_ARGS__)
#define ERROR_AND_EXIT(ec) \
    error_code = ec;       \
    exit(ec);
#define GL_CHECKERROR                                                                                              \
    {                                                                                                              \
        int _err = glGetError();                                                                                   \
        if (_err != GL_NO_ERROR)                                                                                   \
            Util::Log::_log(                                                                                       \
                Util::LogLevel::error, __FILE__, __LINE__, "GL error: {} = {}", _err, Util::Log::glErrorName(_err) \
            );                                                                                                     \
    }

} // namespace Util
