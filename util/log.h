#ifndef LOG_FUNCS_H
#define LOG_FUNCS_H
#include <iostream>

#ifdef WIN32
#undef ERROR
#endif

#define INFO  Util::Log::info(__FILE__, __LINE__)
#define WARN  Util::Log::warn(__FILE__, __LINE__)
#define ERROR Util::Log::error(__FILE__, __LINE__)
#define ERROR_AND_EXIT(ec) error_code = ec; exit(ec);
#define GL_CHECKERROR { int _err = glGetError(); if (_err != GL_NO_ERROR) \
Util::Log::error(__FILE__, __LINE__) << "GL error: " << _err << " = " << Util::Log::glErrorName(_err) << std::endl; }

namespace Util {
  class Log {
    public:
      static std::ostream & info(const char* f, int l);
      static std::ostream & warn(const char* f, int l);
      static std::ostream & error(const char* f, int l);
      static void setOutputLevel(unsigned int newLevel);
      static const char* glErrorName(int k);
    private:
      static unsigned int level;
      static std::ostream emptyStream;
  };
}
#endif
