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
#ifndef UTIL_CELLITERATOR_H
#define UTIL_CELLITERATOR_H
#include <cassert>
#include <cmath>

#include <core/active-map.h>
#include <glm/ext/vector_float3.hpp>

namespace Util {
float xz_angle(const glm::vec3 &from, const glm::vec3 &to);
float xz_turn_angle(const glm::vec3 &from, const glm::vec3 &to);

class CellIterator {
public:
    explicit CellIterator(const glm::vec3 &p)
        : x(int(floor(p.x)))
        , y(int(floor(p.z)))
        , z(int(floor(p.y)))
        , mapRef(OpenGTA::ActiveMap::Instance().get())
    {
    }

    CellIterator(OpenGTA::Map &map, int _x, int _y, int _z)
        : x(_x), y(_y), z(_z), mapRef(map) {}

    CellIterator(const CellIterator &o) = default;

    [[nodiscard]] bool isValid() const;
    [[nodiscard]] int distance(const CellIterator &o) const;

    bool operator==(const CellIterator &o) const { return x == o.x && y == o.y && z == o.z; }

    CellIterator &operator=(const CellIterator &o)
    {
        if (this == &o)
            return *this;

        mapRef = o.mapRef;
        x = o.x;
        y = o.y;
        z = o.z;
        return *this;
    }

    [[nodiscard]] CellIterator left() const
    {
        CellIterator p(*this);
        p.x -= 1;
        return p;
    }
    [[nodiscard]] CellIterator right() const
    {
        CellIterator p(*this);
        p.x += 1;
        return p;
    }
    [[nodiscard]] CellIterator top() const
    {
        CellIterator p(*this);
        p.y += 1;
        return p;
    }
    [[nodiscard]] CellIterator bottom() const
    {
        CellIterator p(*this);
        p.y -= 1;
        return p;
    }
    [[nodiscard]] CellIterator up() const
    {
        CellIterator p(*this);
        p.z += 1;
        return p;
    }
    [[nodiscard]] CellIterator down() const
    {
        CellIterator p(*this);
        p.z -= 1;
        return p;
    }
    [[nodiscard]] bool isBlockType(uint8_t t) const;
    [[nodiscard]] std::pair<bool, CellIterator> findTypeInCol(uint8_t t) const;
    [[nodiscard]] std::pair<bool, CellIterator> findNeighbourWithType(uint8_t t, float angle_hint) const;
    int x, y, z;
    [[nodiscard]] OpenGTA::Map::BlockInfo &getBlock() const;

private:
    OpenGTA::Map &mapRef;
};

} // namespace Util

#endif
