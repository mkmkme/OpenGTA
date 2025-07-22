#pragma once

#include <cstdint>
#include <vector>

namespace Util {
class PhysFSFile;
}

namespace OpenGTA {

struct LoadedAnim {
    explicit LoadedAnim(Util::PhysFSFile &file);
    uint8_t block {};
    uint8_t which {};
    uint8_t speed {};
    uint8_t frameCount {};
    std::vector<uint8_t> frame;
};

} // namespace OpenGTA
