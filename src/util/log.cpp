
#include <SDL2/SDL_opengl.h>

#include <util/log.h>

#ifdef _WIN32
#include <io.h>
#define STDERR_FILENO (_fileno(stderr))
#define isatty        (_isatty)
#else
#include <unistd.h>
#endif

namespace {
std::string_view glErrorName(int k)
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
        default:
            return "Unknown-GL-Error";
    }
}
} // namespace

namespace Util::Log {
void glCheckError(std::source_location loc)
{
    auto err = glGetError();
    if (err != GL_NO_ERROR) {
        OpenGTA::log::error(loc, "OpenGL error: {}", glErrorName(err));
    }
}

} // namespace Util::Log
