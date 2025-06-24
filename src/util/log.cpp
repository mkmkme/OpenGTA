
#include <SDL2/SDL_opengl.h>

#include <util/log.h>

#ifdef _WIN32
#include <io.h>
#define STDERR_FILENO (_fileno(stderr))
#define isatty        (_isatty)
#else
#include <unistd.h>
#endif

namespace Util::Log {
const char *glErrorName(int k)
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

} // namespace Util::Log
