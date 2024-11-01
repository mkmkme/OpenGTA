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
#include <cstdint>
#include <cstring>

#include <core/fx_sdt.h>

#include "util/file-manager.h"
#include <util/errors.h>
#include <util/file_helper.h>

#ifdef SOUND_DUMPER
#include <iostream>
#endif
namespace OpenGTA {
SoundsDB::Entry::Entry(UInt32 r1, UInt32 r2, UInt32 sr)
{
    rawStart = r1;
    rawSize = r2;
    sampleRate = sr;
}

SoundsDB::SoundsDB(const std::string &sdt_file)
    : dataFile { sdt_file }
{
    auto num_e = dataFile.length();
    if (num_e % 12) {
        throw Util::InvalidFormat("SDT filesize " + std::to_string(uint32_t(num_e)) + " % 12 != 0");
    }
    num_e /= 12;
    UInt32 r1, r2, sr;
    for (UInt32 i = 0; i < num_e; i++) {
        dataFile.read(r1);
        dataFile.read(r2);
        dataFile.read(sr);
#ifdef SOUND_DUMPER
        std::cout << i << " " << r1 << " " << r2 << " " << sr << std::endl;
#endif
        knownEntries.insert(std::make_pair(i, Entry(r1, r2, sr)));
    }

    std::string raw_file(sdt_file);
    raw_file.replace(raw_file.size() - 3, 3, "RAW");
    dataFile = Util::PhysFSFile(raw_file);
}

SoundsDB::~SoundsDB() = default;

SoundsDB::Entry &SoundsDB::getEntry(KeyType key)
{
    auto i = knownEntries.find(key);
    if (i == knownEntries.end()) {
        // throw std::string("Unknown sound-db entry");
        throw Util::UnknownKey("Querying for sound id: " + std::to_string(unsigned(key)));
    }
    return i->second;
}

unsigned char *SoundsDB::getBuffered(KeyType key)
{
    Entry &e = getEntry(key);
    unsigned int t_len = e.rawSize; // + 36 + 8;
    auto *buf = new unsigned char[t_len];
    memset(buf, 0, t_len);
    dataFile.seek(e.rawStart);
    dataFile.read(buf, e.rawSize);
    return buf;
}

} // namespace OpenGTA

#ifdef SOUND_DUMPER
int main(int argc, char *argv[])
{
    PHYSFS_init(argv[0]);
    PHYSFS_mount("gtadata.zip", nullptr, 1);
    try {
        OpenGTA::SoundsDB sounds(argv[1]);
    } catch (std::string &e) {
        std::cerr << e << std::endl;
    }
    PHYSFS_deinit();
}
#endif
