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

#include <core/map.h>
#include <core/navdata.h>

#include <util/errors.h>
#include <util/file_helper.h>
#include <util/log.h>
#include <util/string_helpers.h>

namespace {

constexpr UInt8 topHeaderSize = 28;
constexpr UInt64 baseSize = 262144;

} // namespace

/* see http://members.aol.com/form1/fixed.htm for fixed point floats:
 * int_var = (long) fixed_var >> 8; // for 8 bits after point
 */
namespace OpenGTA {

namespace {

inline size_t mapFileName2Number(const std::string &file)
{
    std::string file2 { Util::string_lower(file) };
    if (file2 == "nyc.cmp")
        return 1;
    if (file2 == "sanb.cmp")
        return 2;
    if (file2 == "miami.cmp")
        return 3;
    ERROR("unknown level: {}", file);
    return 0;
}

} // namespace

Map::Map(const std::string &filename)
    : pf { filename }
{
    size_t level_as_num = mapFileName2Number(filename);
    loadHeader();
    loadBase();
    loadColumn();
    loadBlock();
    loadObjects();
    loadRoutes();
    loadLocations();
    loadNavData(level_as_num);
    // dump();
}

void Map::loadHeader()
{
    auto vc = pf.read<UInt32>();
    pf.read(styleNumber);
    auto sn = pf.read<UInt8>();
    auto reserved = pf.read<UInt16>();
    pf.read(routeSize);
    pf.read(objectPosSize);
    pf.read(columnSize);
    pf.read(blockSize);
    pf.read(navDataSize);
    /*
    INFO << "Route size: " << routeSize << std::endl;
    INFO << "Object size: " << objectPosSize << std::endl;
    INFO << "Column size: " << columnSize << std::endl;
    INFO << "Block size: " << blockSize << " (" <<
      blockSize / sizeof(BlockInfo) << " blocks " << blockSize % sizeof(BlockInfo)
      << " overcount)" << std::endl;
    INFO << "Navdata size: " << navDataSize << std::endl;
    */

    column.resize(columnSize / 2);
    block.resize(blockSize / sizeof(BlockInfo));
    objects.resize(objectPosSize / sizeof(ObjectPosition));
}

void Map::loadBase()
{
    pf.seek(topHeaderSize);
    pf.read(base, sizeof(base));
}

void Map::loadColumn()
{
    std::span<UInt16> columnSpan { column };
    pf.read(columnSpan);
}

void Map::loadBlock()
{
    pf.seek(baseSize + columnSize + topHeaderSize);
    int i, max;
    max = blockSize / sizeof(BlockInfo);
    // uint8_t tmp;
    for (i = 0; i < max; i++) {
        pf.read(block[i].typeMap);
        pf.read(block[i].typeMapExt);
        pf.read(block[i].left);
        pf.read(block[i].right);
        pf.read(block[i].top);
        pf.read(block[i].bottom);
        pf.read(block[i].lid);
        // block[i].animMode = 0;
    }
}
void Map::loadObjects()
{
    pf.seek(baseSize + columnSize + topHeaderSize + blockSize);
    int c = objectPosSize / sizeof(ObjectPosition);
    numObjects = c;
    assert(objectPosSize % sizeof(ObjectPosition) == 0);
    for (int i = 0; i < c; i++) {
        pf.read(objects[i].x);
        pf.read(objects[i].y);
        pf.read(objects[i].z);
        pf.read(objects[i].type);
        pf.read(objects[i].remap);
        pf.read(objects[i].rotation);
        pf.read(objects[i].pitch);
        pf.read(objects[i].roll);

        // shift every coord? or just if any > 255
        /*
        objects[i].x = objects[i].x >> 6;
        objects[i].y = objects[i].y >> 6;
        objects[i].z = objects[i].z >> 6;*/
        /*
        std::cout << objects[i].x << "," << objects[i].y << "," << objects[i].z << " " << int(objects[i].type)
        << " remap " << int(objects[i].remap)
        << " rot " << objects[i].rotation << " " << objects[i].pitch << " " << objects[i].roll << std::endl;
        */
    }
}
void Map::loadRoutes()
{
    // FIXME: missing
    UInt32 _si = baseSize + columnSize + topHeaderSize + objectPosSize + blockSize;
    pf.seek(_si);
    UInt32 _counted = 0;
    while (_counted < routeSize) {
        UInt8 num_vertices = 0;
        UInt8 route_type = 0;
        pf.read(num_vertices);
        pf.read(route_type);
        // INFO << "route-t " << int(route_type) << " with " << int(num_vertices) << " vertices" << std::endl;
        UInt8 x, y, z;
        for (int i = 0; i < num_vertices; i++) {
            pf.read(x);
            pf.read(y);
            pf.read(z);
            // INFO << int(x) << "," << int(y) << "," << int(z) << std::endl;
            _counted += 3;
        }

        _counted += 2;
    }
}
void Map::loadLocations()
{
    // FIXME: missing
    UInt32 _si = baseSize + columnSize + topHeaderSize + objectPosSize + routeSize + blockSize;
    pf.seek(_si);
    // police
    // hospital
    // unused
    // unused
    // fire
    // unused
    UInt8 loc_type = 0;
    for (int i = 0; i < 36; ++i) {
        Location loc(pf);
        // skip dummy entries at 0,0,0
        if ((loc.x == 0) && (loc.y == 0) && (loc.z == 0))
            continue;
        if (i < 6)
            loc_type = 0;
        else if (i < 12)
            loc_type = 1;
        else if ((i >= 24) && (i < 30))
            loc_type = 2;
        else
            continue;
        locations.insert({ loc_type, loc });
    }
}
void Map::loadNavData(size_t level_num)
{
    UInt32 _si = baseSize + columnSize + topHeaderSize + objectPosSize + routeSize + (3 * 6 * 6) + blockSize;
    pf.seek(_si);
    nav.emplace(navDataSize, pf, level_num);
}
UInt16 Map::getNumBlocksAt(UInt8 x, UInt8 y)
{
    return column[base[y][x] / 2];
}
UInt16 Map::getNumBlocksAtNew(UInt8 x, UInt8 y)
{
    return 6 - column[base[y][x] / 2];
}
Map::BlockInfo *Map::getBlockAt(UInt8 x, UInt8 y, UInt8 z)
{
    UInt16 v = column[(base[y][x] / 2) + z];
    return &block[v];
}
Map::BlockInfo *Map::getBlockAtNew(UInt8 x, UInt8 y, UInt8 z)
{
    UInt16 idx0 = 6 - column[base[y][x] / 2];
    if (idx0 > z)
        idx0 -= z;
    else
        assert(idx0 > z);
    idx0 = column[(base[y][x] / 2) + idx0];
    return &block[idx0];
}
UInt16 Map::getInternalIdAt(UInt8 x, UInt8 y, UInt8 z)
{
    return column[(base[y][x] / 2) + z];
}
Map::BlockInfo *Map::getBlockByInternalId(UInt16 id)
{
    return &block[id];
}
void Map::dump()
{
    for (int y = 0; y < GTA_MAP_MAXDIMENSION; y++) {
        for (int x = 0; x < GTA_MAP_MAXDIMENSION; x++) {
            fmt::print("{}, {}: {}||(", x, y, column[base[y][x] / 2]);
            UInt16 ts = column[base[y][x] / 2];
            fmt::print("(");
            for (int t = 1; t <= (6 - ts); t++) {
                BlockInfo *info = &block[column[(base[y][x] / 2) + t]];
                fmt::print("{}, ", int(info->slopeType()));
            }
            fmt::print(")\n");
        }
    }
}
const Map::Location &Map::getNearestLocationByType(UInt8 t, int x, int y)
{
    INFO("{} at {} {}", int(t), x, y);
    auto i = locations.find(t);
    auto j = i;
    if (i == locations.end())
        throw Util::UnknownKey("location-type {} not found in map", int(t));
    int min_d = 255 * 255;

    while (i != locations.end()) {
        INFO("{}: {} {}", int(i->first), int(i->second.x), int(i->second.y));
        int d = abs(x - i->second.x) + abs(y - i->second.y);
        if (d < min_d) {
            min_d = d;
            j = i;
        }
    }
    return j->second;
}

Map::Location::Location(Util::PhysFSFile &pf)
{
    pf.read(x);
    pf.read(y);
    pf.read(z);
}

} // namespace OpenGTA
