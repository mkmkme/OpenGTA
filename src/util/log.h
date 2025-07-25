#pragma once

#include <filesystem>
#include <source_location>
#include <utility>

#include "base/config.h"

#ifdef OGTA_USE_FMT_COLOR
#include <fmt/color.h>
#endif
#include <fmt/base.h>
#include <fmt/format.h>

#ifdef WIN32
#undef ERROR
#endif

// Credit: This class is inspired by Nathan Baggs' ugame logging
// https://github.com/nathan-baggs/ugame/blob/de9b6d69b7b7eccb785be3de2db4b2e3809f98b1/src/utils/log.h

namespace OpenGTA::log {

enum class Level : uint8_t { error, warn, info, debug };

inline Level level = Level::info;

template <Level lvl, typename... Args>
struct Logger {
    constexpr explicit Logger(
        fmt::format_string<Args...> fmt,
        Args &&...args,
        std::source_location loc = std::source_location::current()
    )
    {
        if (level < lvl)
            return;

#ifdef OGTA_USE_FMT_COLOR
        constexpr fmt::color color = []() constexpr {
            if constexpr (lvl == Level::error) {
                return fmt::color::red;
            } else if constexpr (lvl == Level::warn) {
                return fmt::color::yellow;
            } else if constexpr (lvl == Level::info) {
                return fmt::color::green;
            } else if constexpr (lvl == Level::debug) {
                return fmt::color::blue;
            } else {
                return fmt::color::white;
            }
        }();
#endif

        constexpr char symbol = []() constexpr {
            if constexpr (lvl == Level::error) {
                return 'E';
            } else if constexpr (lvl == Level::warn) {
                return 'W';
            } else if constexpr (lvl == Level::info) {
                return 'I';
            } else if constexpr (lvl == Level::debug) {
                return 'D';
            } else {
                return '?';
            }
        }();

        const auto path = std::filesystem::path(loc.file_name());

        fmt::println(
            "[{}]({}:{}) {}",
#ifdef OGTA_USE_FMT_COLOR
            fmt::styled(symbol, fmt::fg(color)),
#else
            symbol,
#endif
            path.filename().string(),
            loc.line(),
            fmt::format(fmt, std::forward<Args>(args)...)
        );
    }

    constexpr explicit Logger(std::source_location loc, fmt::format_string<Args...> fmt, Args &&...args)
        : Logger(fmt, std::forward<Args>(args)..., loc)
    {
    }
};

// Deduction guide for Logger
template <Level L = {}, typename... Args>
Logger(fmt::format_string<Args...>, Args &&...) -> Logger<L, Args...>;

template <typename... Args>
using error = Logger<Level::error, Args...>;
template <typename... Args>
using warn = Logger<Level::warn, Args...>;
template <typename... Args>
using info = Logger<Level::info, Args...>;
template <typename... Args>
using debug = Logger<Level::debug, Args...>;

} // namespace OpenGTA::log

namespace Util::Log {
void glCheckError(std::source_location loc = std::source_location::current());
} // namespace Util::Log

// TODO: Remove these aliases in the future
template <typename... Args>
using ERROR = OpenGTA::log::error<Args...>;
template <typename... Args>
using WARN = OpenGTA::log::warn<Args...>;
template <typename... Args>
using INFO = OpenGTA::log::info<Args...>;
template <typename... Args>
using DEBUG = OpenGTA::log::debug<Args...>;

#define GL_CHECKERROR Util::Log::glCheckError() // TODO: remove
