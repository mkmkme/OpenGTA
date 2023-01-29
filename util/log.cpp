#include "log.h"

#include "config.h"

#include <SDL2/SDL_opengl.h>
#include <assert.h>

namespace Util {
LogLevel Log::level_ = LogLevel::info;

const char *Log::glErrorName(int k)
{
    switch (k) {
        case GL_NO_ERROR:
            return "GL_NO_ERROR";
        case GL_INVALID_VALUE:
            return "GL_INVALID_VALUE";
        case GL_INVALID_OPERATION:
            return "GL_INVALID_OPERATION";
        case GL_STACK_OVERFLOW:
            return "GL_STACK_OVERFLOW";
        case GL_STACK_UNDERFLOW:
            return "GL_STACK_UNDERFLOW";
        case GL_OUT_OF_MEMORY:
            return "GL_OUT_OF_MEMORY";
    }
    return "Unknown-Error";
}

const char *level_name(LogLevel level)
{
    switch (level) {
        case LogLevel::info:
            return "Info";
        case LogLevel::warn:
            return "Warning";
        case LogLevel::error:
            return "Error";
        case LogLevel::debug:
            return "Debug";
    }
    assert(false);
}

fmt::color Log::level_color(LogLevel level)
{
    switch (level) {
        case LogLevel::info:
            return fmt::color::green;
        case LogLevel::warn:
            return fmt::color::yellow;
        case LogLevel::error:
            return fmt::color::red;
        case LogLevel::debug:
            return fmt::color::blue;
    }
    assert(false);
}

void Log::prefix(LogLevel level, const char *file, int line)
{
    if (isatty(STDERR_FILENO))
        fmt::print(stderr,
                   fg(level_color(level)),
                   "{} ({}:{}): ",
                   level_name(level),
                   file,
                   line);
    else
        fmt::print(stderr, "{} ({}:{}): ", level_name(level), file, line);
}
} // namespace Util
