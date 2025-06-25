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
#include <cassert>

#include <core/main-msg-lookup.h>
#include <core/navdata.h>
#include <fmt/format.h>

#include <util/errors.h>
#include <util/file-manager.h>
#include <util/log.h>

namespace OpenGTA {

bool Rect2D::isInside(UInt8 _x, UInt8 _y)
{
    if ((_x >= x) && (_y >= y) &&
        (UInt16(_x) <= UInt16(x) + w) &&
        (UInt16(_y) <= UInt16(y) + h)) {
        lastSubLocation = subLocation(_x, _y);
        return true;
    }
    return false;
}

UInt16 Rect2D::getSize() const noexcept
{
    return w * h;
}

// 0 = central, 1 = north, 2 = south, 4 = east, 8 = west
UInt8 Rect2D::subLocation(UInt8 _x, UInt8 _y) const
{
    UInt8 in_x = _x - x; // offset in rect; assume: x <= _x
    UInt8 in_y = _y - y;
    float rel_x = float(in_x) / w;
    float rel_y = float(in_y) / h;
    UInt8 res = 0;
    const float oneThird = 1.0f / 3.0f;
    const float twoThirds = 2.0f / 3.0f;
    if (rel_x <= oneThird)
        // INFO << "west" << std::endl;
        res |= 8;
    else if (rel_x >= twoThirds)
        // INFO << "east" << std::endl;
        res |= 4;
    if (rel_y <= oneThird)
        res |= 1;
    // INFO << "north" << std::endl;
    else if (rel_y >= twoThirds)
        res |= 2;
    // INFO << "south" << std::endl;
    return res;
}

NavData::Sector::Sector(Util::PhysFSFile &pf)
{
    // memset(name2, 0, 30);
    pf.read(x);
    pf.read(y);
    pf.read(w);
    pf.read(h);
    pf.read(sam);
    // seek over the name embedded in the mapfile; use sample-num to
    // lookup in msg-db
    // PHYSFS_read(fd, static_cast<void*>(&name2), 30, 1);
    pf.seek(pf.tell() + 30);
}

NavData::Sector::Sector()
{
    x = 0;
    y = 0;
    w = 255;
    h = 255;
    sam = 0;
    isADummy = true;
}

std::string NavData::Sector::getFullName() const
{
    std::string n;
    if (isADummy)
        return n;
    switch (lastSubLocation) {
        case 0:
            // n.append("Central ");
            n.append(_c);
            break;
        case 1:
            // n.append("North ");
            n.append(_n);
            break;
        case 2:
            // n.append("South ");
            n.append(_s);
            break;
        case 4:
            // n.append("East ");
            n.append(_e);
            break;
        case 5:
            // n.append("Northeast ");
            n.append(_ne);
            break;
        case 6:
            // n.append("Southeast ");
            n.append(_se);
            break;
        case 8:
            // n.append("West ");
            n.append(_w);
            break;
        case 9:
            // n.append("Northwest ");
            n.append(_nw);
            break;
        case 10:
            // n.append("Southwest ");
            n.append(_sw);
            break;
    }

    n.append(" ");
    n.append(name);
    return n;
}

std::string NavData::_c;
std::string NavData::_n;
std::string NavData::_s;
std::string NavData::_w;
std::string NavData::_e;
std::string NavData::_nw;
std::string NavData::_ne;
std::string NavData::_sw;
std::string NavData::_se;

NavData::NavData(UInt32 size, Util::PhysFSFile &pf, size_t level_num)
{
    if (size % 35) {
        throw Util::InvalidFormat("Navdata size: {} % 35 != 0", size);
        // throw std::string("Invalid NavData size in mapfile");
    }
    UInt32 c = size / 35;

    MessageDB &msg = MainMsgLookup::Instance().get();
    _c = msg.getText("c");
    _n = msg.getText("n");
    _s = msg.getText("s");
    _w = msg.getText("w");
    _e = msg.getText("e");
    _nw = msg.getText("nw");
    _ne = msg.getText("ne");
    _sw = msg.getText("sw");
    _se = msg.getText("se");
    for (UInt32 i = 0; i < c; ++i) {
        Sector sec { pf };
        if (sec.getSize() == 0) { // workaround for 'NYC.CMP' (empty sectors)
            WARN("skipping zero size sector");
            continue;
        }
        sec.name = msg.getText(fmt::format("{:03}area{:03}", level_num, int(sec.sam)));

        const auto size = sec.getSize();

        areas.insert({ size, std::move(sec) });
    }
    // dummy catch-all sector for gta london maps
    areas.insert({ 255 * 255, Sector() });

    /*
    std::cout << "map areas (by size)" << std::endl;
    SectorMapType::iterator i = areas.begin();
    while (i != areas.end()) {
      std::cout << " " << i->first << " : " << i->second->name2 << " @ " <<
      int(i->second->x) << "," << int(i->second->y)  << "  " << int(i->second->w) << "x" <<
      int(i->second->h) <<  " sample " << int(i->second->sam) << std::endl;
      ++i;
    }
    */
}

NavData::~NavData() = default;

NavData::Sector *NavData::getSectorAt(UInt8 x, UInt8 y)
{
    for (auto &area : areas) {
        if (area.second.isInside(x, y))
            return &area.second;
    }
    throw Util::OutOfRange("Querying invalid sector at {}, {}", int(x), int(y));
}

void NavData::clear()
{
    areas.clear();
}
} // namespace OpenGTA
