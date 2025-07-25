/************************************************************************
 * Copyright (c) 2005-2007 tok@openlinux.org.uk                          *
 *                                                                       *
 * This software is provided as-is, without any express or implied       *
 * warranty. In no event will the authors be held liable for any         *
 * damages arising from the use of this software.                        *
 *                                                                       *
 * Permission is granted to anyone to use this software for any purpose, *
 * including commercial applications, and to alter it and redistribute   *
 * it freely, subject to the following restrictions:                     *
 *                                                                       *
 * 1. The origin of this software must not be misrepresented; you must   *
 * not claim that you wrote the original software. If you use this       *
 * software in a product, an acknowledgment in the product documentation *
 * would be appreciated but is not required.                             *
 *                                                                       *
 * 2. Altered source versions must be plainly marked as such, and must   *
 * not be misrepresented as being the original software.                 *
 *                                                                       *
 * 3. This notice may not be removed or altered from any source          *
 * distribution.                                                         *
 ************************************************************************/

// Prevent SDL from overriding main().
#define SDL_MAIN_HANDLED

#include "viewer-config.h"
#include "viewer.h"

int main(int argc, char *argv[]) // NOLINT(bugprone-exception-escape)
{
    Util::enableBacktraces();
    OpenGTA::Viewer app { argv[0] };

    if (argc > 1) {
        using enum OpenGTA::ViewerConfig::ParseArgsResult;
        const auto result = app.parseArgs(argc, argv);
        switch (result) {
            case ExitSuccess:
                return 0;
            case ExitError:
                return 1;
            default:
                break;
        }
    }

    app.run();
    app.quit();

    return 0;
}
