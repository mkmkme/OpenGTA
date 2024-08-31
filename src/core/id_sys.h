#pragma once

#include <core/game_objects.h>

#include <util/errors.h>

namespace OpenGTA {

class TypeIdBlackBox {
public:
    static TypeIdBlackBox &Instance()
    {
        static TypeIdBlackBox instance;
        return instance;
    }

    uint32_t requestId()
    {
        if (nextId + 1 >= firstPlayerId) {
            throw Util::OutOfRange("Player id range reached!");
        }
        return nextId++;
    }
    constexpr uint32_t getPlayerId() const noexcept { return lastPlayerId; }

private:
    TypeIdBlackBox() = default;
    TypeIdBlackBox(const TypeIdBlackBox &) = delete;
    TypeIdBlackBox &operator=(const TypeIdBlackBox &) = delete;

    uint32_t nextId = 0;
    constexpr static uint32_t firstPlayerId = 0xffffffff - 32;
    constexpr static uint32_t lastPlayerId = 0xffffffff;
};

} // namespace OpenGTA
