#pragma once

#include <compare>
#include <filesystem>
#include <source_location>

#include <fmt/base.h>
#include <fmt/color.h>
#include <fmt/core.h>

#ifdef WIN32
#undef ERROR
#endif

// Credit: This class is inspired by Nathan Baggs' ugame logging
// https://github.com/nathan-baggs/ugame/blob/de9b6d69b7b7eccb785be3de2db4b2e3809f98b1/src/utils/log.h

namespace OpenGTA::Log {

enum class Level : uint8_t { error,
                             warn,
                             info,
                             debug,
};

inline std::strong_ordering operator<=>(Level lhs, Level rhs) noexcept
{
    return static_cast<int>(lhs) <=> static_cast<int>(rhs);
}

inline Level level = Level::info;

template <Level lvl, typename... Args>
struct Logger {
    explicit Logger(fmt::format_string<Args...> fmt, Args &&...args, std::source_location loc = std::source_location::current())
    {
        if (level < lvl)
            return;

        auto symbol = '?';
        auto color = fmt::color::white;

        if constexpr (lvl == Level::error) {
            symbol = 'E';
            color = fmt::color::red;
        } else if constexpr (lvl == Level::warn) {
            symbol = 'W';
            color = fmt::color::yellow;
        } else if constexpr (lvl == Level::info) {
            symbol = 'I';
            color = fmt::color::green;
        } else if constexpr (lvl == Level::debug) {
            symbol = 'D';
            color = fmt::color::blue;
        }

        const auto path = std::filesystem::path(loc.file_name());

        fmt::println("[{}]({}:{}) {}", fmt::styled(symbol, fmt::fg(color)), path.filename().string(), loc.line(), fmt::format(fmt, std::forward<Args>(args)...));
    }
};

// Deduction guide for Logger
template <Level L = {}, typename... Args>
Logger(fmt::format_string<Args...>, Args &&...)
    -> Logger<L, Args...>;

template <typename... Args>
using error = Logger<Level::error, Args...>;
template <typename... Args>
using warn = Logger<Level::warn, Args...>;
template <typename... Args>
using info = Logger<Level::info, Args...>;
template <typename... Args>
using debug = Logger<Level::debug, Args...>;

} // namespace OpenGTA::Log

namespace Util::Log {
const char *glErrorName(int k);
}

// TODO: Remove these aliases in the future
template <typename... Args>
using ERROR = OpenGTA::Log::Logger<OpenGTA::Log::Level::error, Args...>;
template <typename... Args>
using WARN = OpenGTA::Log::Logger<OpenGTA::Log::Level::warn, Args...>;
template <typename... Args>
using INFO = OpenGTA::Log::Logger<OpenGTA::Log::Level::info, Args...>;
template <typename... Args>
using DEBUG = OpenGTA::Log::Logger<OpenGTA::Log::Level::debug, Args...>;

#define GL_CHECKERROR                                               \
    do {                                                            \
        auto err = glGetError();                                    \
        if (err != GL_NO_ERROR) {                                   \
            ERROR("OpenGL error: {}", Util::Log::glErrorName(err)); \
        }                                                           \
    } while (0)
