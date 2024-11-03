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
#include <cassert>
#include <map>

#include <physfs.h>

#include <core/config.h>

#include <util/errors.h>
#include <util/file_helper.h>
#include <util/log.h>
#include <util/string_helpers.h>

namespace {

std::string getEnvSafe(const char *envname, std::string def_value)
{
    const char *value = getenv(envname);
    return (value != nullptr) ? value : def_value;
}

} // namespace

namespace Util::FileHelper {

const std::string &BaseDataPath()
{
    static std::string value = getEnvSafe("OGTA_DATA", OGTA_DEFAULT_DATA_PATH);
    return value;
}

const std::string &ModDataPath()
{
    static std::string value = getEnvSafe("OGTA_MOD", OGTA_DEFAULT_MOD_PATH);
    return value;
}

const std::string &UserHomeDir()
{
    static std::string value = getEnvSafe("OGTA_HOME", OGTA_DEFAULT_HOME_PATH);
    return value;
}

std::string Lang2MsgFilename(std::string_view l)
{
    static std::map<std::string_view, std::string> langmap {
        { "en", "ENGLISH.FXT" },
        { "de", "GERMAN.FXT" },
        { "fr", "FRENCH.FXT" },
        { "it", "ITALIAN.FXT" },
    };
    if (l.size() > 2)
        l = l.substr(0, 2);
    auto it = langmap.find(l);
    if (it != langmap.end())
        return it->second;
    WARN("Unknown language: {} - falling back to english", l);
    return "ENGLISH.FXT";
}

} // namespace Util::FileHelper
