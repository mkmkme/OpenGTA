#pragma once

#include <vector>

#include <core/numeric-types.h>

namespace Util {
class PhysFSFile;
}

namespace OpenGTA {

struct LoadedAnim {
    explicit LoadedAnim(Util::PhysFSFile &file);
    UInt8 block {};
    UInt8 which {};
    UInt8 speed {};
    UInt8 frameCount {};
    std::vector<UInt8> frame;
};

} // namespace OpenGTA
