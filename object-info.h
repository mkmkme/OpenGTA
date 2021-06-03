#pragma once

#include <physfs.h>

namespace OpenGTA {

struct ObjectInfo {
    ObjectInfo(PHYSFS_file *fd);
    PHYSFS_uint32 width, height, depth;
    PHYSFS_uint16 sprNum, weight, aux;
    PHYSFS_sint8 status;
    PHYSFS_uint8 numInto;
    // PHYSFS_uint16 into[255]; // FIXME: MAX_INTO ???
};

} // namespace OpenGTA
