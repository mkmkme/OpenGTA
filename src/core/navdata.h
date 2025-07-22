/************************************************************************
 * Copyright (c) 2005-2007 tok@openlinux.org.uk                          *
 *                                                                       *
 * This file contains code derived from information copyrighted by       *
 * DMA Design. It may not be used in a commercial product.               *
 *                                                                       *
 * See license.txt for details.                                          *
 *                                                                       *
 * This notice may not be removed or altered.                            *
 ************************************************************************/
#ifndef NAVDATA_H
#define NAVDATA_H
#include <cstdint>
#include <map>
#include <string>

namespace Util {
class PhysFSFile;
}

namespace OpenGTA {

/** Helper class for area names.
 *
 * A simple box; beware uint8 overflow!
 *
 * @see NavData
 */
struct Rect2D {
public:
    /** Zero-constructor.
     * Everything is 0
     */
    Rect2D() = default;
    /** Test: point-in-box.
     * @param x
     * @param y
     * @note If the point is inside 'lastSubLocation' is updated before returning.
     */
    bool isInside(uint8_t, uint8_t);
    /** Calculate north/south/east/west/central of point (which has to be inside).
     * @param x
     * @param y
     * @return uint8 bitfield
     */
    [[nodiscard]] uint8_t subLocation(uint8_t, uint8_t) const;
    [[nodiscard]] uint16_t getSize() const noexcept;

protected:
    uint8_t x = 0, y = 0;
    uint8_t w = 0, h = 0;
    /** Last sub-area location.
     * 0 = central
     * 1 = north
     * 2 = south
     * 4 = east
     * 8 = west
     * ... valid combinations of the last four
     */
    uint8_t lastSubLocation {};
};

/** Container of all named sectors.
 * @see Sector
 */
class NavData {
public:
    /** A named sector of the map.
     */
    struct Sector : public Rect2D {
        /** Constructor from valid PHYSFS handle.
         */
        explicit Sector(Util::PhysFSFile &pf);
        Sector();
        /** Sample number.
         * 1) see $LANGUAGE.FXT file for actual name
         * 2) probably sound?
         */
        uint8_t sam {}; // sample number
        std::string name;
        /** Returns the name prefixed with sub-area location.
         */
        [[nodiscard]] std::string getFullName() const;

    private:
        bool isADummy {};
    };
    NavData(uint32_t size, Util::PhysFSFile &pf, size_t level_num);
    ~NavData();
    Sector *getSectorAt(uint8_t, uint8_t);
    static std::string _c, _n, _s, _w, _e, _nw, _ne, _sw, _se;

private:
    void clear();
    using SectorMapType = std::multimap<uint16_t, Sector>;
    SectorMapType areas;
};
} // namespace OpenGTA

#endif
