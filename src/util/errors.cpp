#include <iostream>

#include <util/errors.h>

namespace Util {
void enableBacktraces()
{
    // Enable stack traces for exceptions
    std::set_terminate([]() {
        try {
            throw;
        } catch (const Exception &e) {
            std::cerr << "Unhandled exception: " << e.what() << '\n';
            std::cerr << "Stack trace:\n"
                      << e.stacktrace() << '\n';
        } catch (const std::exception &e) {
            std::cerr << "Unhandled standard exception: " << e.what() << '\n';
        } catch (...) {
            std::cerr << "Unhandled unknown exception\n";
        }
        std::abort();
    });
}
} // namespace Util
