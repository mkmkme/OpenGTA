#pragma once

#include <cstdint>
namespace Util {
class PhysFSFile;
}

namespace OpenGTA {

struct ObjectInfo {
    explicit ObjectInfo(Util::PhysFSFile &file);
    uint32_t width {}, height {}, depth {};
    uint16_t sprNum {}, weight {}, aux {};
    int8_t status {};
    uint8_t numInto {};
    // PHYSFS_uint16 into[255]; // FIXME: MAX_INTO ???
};

} // namespace OpenGTA
