#include "util/errors.h"

#include <iostream>

#include "util/log.h"

namespace Util {
void enableBacktraces()
{
#ifdef OGTA_STD_STACKTRACE_AVAILABLE
    // Enable stack traces for exceptions
    std::set_terminate([]() {
        try {
            throw;
        } catch (const Exception &e) {
            std::cerr << "Unhandled exception: " << e.what() << '\n';
            std::cerr << "Stack trace:\n" << e.stacktrace() << '\n';
        } catch (const std::exception &e) {
            std::cerr << "Unhandled standard exception: " << e.what() << '\n';
        } catch (...) {
            std::cerr << "Unhandled unknown exception\n";
        }
        std::abort();
    });
#else
    OpenGTA::log::warn("Stack traces are not available");
#endif
}
} // namespace Util
