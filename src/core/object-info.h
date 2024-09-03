#pragma once

#include <core/numeric-types.h>

namespace Util {
class PhysFSFile;
}

namespace OpenGTA {

struct ObjectInfo {
    explicit ObjectInfo(Util::PhysFSFile &file);
    UInt32 width {}, height {}, depth {};
    UInt16 sprNum {}, weight {}, aux {};
    Int8 status {};
    UInt8 numInto {};
    // PHYSFS_uint16 into[255]; // FIXME: MAX_INTO ???
};

} // namespace OpenGTA
