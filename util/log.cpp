#include "log.h"
#include "config.h"
#include <SDL_opengl.h>

#define ANSI_COLOR_OFF "\033[0m"
#define ANSI_COLOR_INFO "\033[32m"
#define ANSI_COLOR_WARN "\033[33m"
#define ANSI_COLOR_ERR  "\033[31m"

namespace Util {
  unsigned int Log::level = 0;
  std::ostream Log::emptyStream(0);

  void Log::setOutputLevel(unsigned int newLevel) {
    level = newLevel;
  }

  const char* Log::glErrorName(int k) {
    switch(k) {
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

  std::ostream &Log::info(const char *f, int l)
  {
    if (level) return emptyStream;
    std::cout
#ifdef LOG_USE_ANSI_COLORS
    << ANSI_COLOR_INFO
#endif
    << "Info (" << f << ":" << l << "): "
#ifdef LOG_USE_ANSI_COLORS
    << ANSI_COLOR_OFF
#endif
    ;
    return std::cout;
  }

  std::ostream &Log::warn(const char *f, int l)
  {
    if (level > 1) return emptyStream;
    std::cerr
#ifdef LOG_USE_ANSI_COLORS
    << ANSI_COLOR_WARN
#endif
    << "Warning (" << f << ":" << l << "): "
#ifdef LOG_USE_ANSI_COLORS
    << ANSI_COLOR_OFF
#endif
    ;
    return std::cerr;
  }

  std::ostream &Log::error(const char* f, int l) {
    std::cerr
#ifdef LOG_USE_ANSI_COLORS
    << ANSI_COLOR_ERR
#endif
    << "Error (" << f << ":" << l << "): "
#ifdef LOG_USE_ANSI_COLORS
    << ANSI_COLOR_OFF
#endif
    ;
    return std::cerr;
  }
}
