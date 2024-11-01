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
    size_t num = 0;
    std::string file2 { Util::string_lower(file) };
    if (file2.find("nyc.cmp") != std::string::npos)
        num = 1;
    else if (file2.find("sanb.cmp") != std::string::npos)
        num = 2;
    else if (file2.find("miami.cmp") != std::string::npos)
        num = 3;
    else
        ERROR("unknown level: {}", file);
    return num;
}

} // namespace

Map::Map(const std::string &filename)
    : pf { filename }
{
    nav = nullptr;
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
Map::~Map()
{
    if (column)
        delete[] column;
    if (block)
        delete[] block;
    if (objects)
        delete[] objects;
    if (nav)
        delete nav;
}
int Map::loadHeader()
{
    UInt32 vc;
    pf.read(vc);
    UInt8 sn;
    pf.read(styleNumber);
    pf.read(sn);
    UInt16 reserved;
    pf.read(reserved);
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

    column = new UInt16[columnSize / 2];
    block = new BlockInfo[blockSize / sizeof(BlockInfo)];

    objects = new ObjectPosition[objectPosSize / sizeof(ObjectPosition)];

    return 0;
}
int Map::loadBase()
{
    pf.seek(topHeaderSize);
    for (int y = 0; y < GTA_MAP_MAXDIMENSION; y++) {
        for (int x = 0; x < GTA_MAP_MAXDIMENSION; x++) {
            pf.read(base[x][y]);
        }
    }
    return 0;
}
int Map::loadColumn()
{
    for (unsigned int i = 0; i < columnSize / 2; i++) {
        pf.read(column[i]);
    }
    return 0;
}
int Map::loadBlock()
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
    return 0;
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
        Location loc;
        pf.read(loc.x);
        pf.read(loc.y);
        pf.read(loc.z);
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
        // std::cout << int(loc_type) <<": " << int(loc.x) << ", " << int(loc.y) << ", " << int(loc.z) << std::endl;
        locations.insert({ loc_type, std::move(loc) });
    }
}
void Map::loadNavData(const size_t levelNum)
{
    UInt32 _si = baseSize + columnSize + topHeaderSize + objectPosSize + routeSize + 3 * 6 * 6 + blockSize;
    pf.seek(_si);
    nav = new NavData(navDataSize, pf, levelNum);
    assert(nav);
}
UInt16 Map::getNumBlocksAt(UInt8 x, UInt8 y)
{
    return column[base[x][y] / 2];
}
UInt16 Map::getNumBlocksAtNew(UInt8 x, UInt8 y)
{
    return 6 - column[base[x][y] / 2];
}
Map::BlockInfo *Map::getBlockAt(UInt8 x, UInt8 y, UInt8 z)
{
    UInt16 v = column[base[x][y] / 2 + z];
    return &block[v];
}
Map::BlockInfo *Map::getBlockAtNew(UInt8 x, UInt8 y, UInt8 z)
{
    UInt16 idx0 = 6 - column[base[x][y] / 2];
    if (idx0 > z)
        idx0 -= z;
    else
        assert(idx0 > z);
    idx0 = column[base[x][y] / 2 + idx0];
    return &block[idx0];
}
UInt16 Map::getInternalIdAt(UInt8 x, UInt8 y, UInt8 z)
{
    return column[base[x][y] / 2 + z];
}
Map::BlockInfo *Map::getBlockByInternalId(UInt16 id)
{
    return &block[id];
}
void Map::dump()
{
    for (int y = 0; y < GTA_MAP_MAXDIMENSION; y++) {
        for (int x = 0; x < GTA_MAP_MAXDIMENSION; x++) {
            std::cout << x << "," << y << ":" << column[base[x][y] / 2] << "||";
            UInt16 ts = column[base[x][y] / 2];
            std::cout << "(";
            for (int t = 1; t <= (6 - ts); t++) {
                BlockInfo *info = &block[column[base[x][y] / 2 + t]];
                std::cout << int(info->slopeType()) << ", ";
            }
            std::cout << ")" << std::endl;
        }
    }
}
const Map::Location &Map::getNearestLocationByType(uint8_t t, uint8_t x, uint8_t y)
{
    INFO("{} at {} {}", int(t), int(x), int(y));
    auto i = locations.find(t);
    auto j = i;
    if (i == locations.end())
        throw Util::UnknownKey("location-type " + std::to_string(int(t)) + " not found in map");
    int _x(x);
    int _y(y);
    int min_d = 255 * 255;

    while (i != locations.end()) {
        INFO("{}: {} {}", int(i->first), int(i->second.x), int(i->second.y));
        int d = abs(_x - i->second.x) + abs(_y - i->second.y);
        if (d < min_d) {
            min_d = d;
            j = i;
        }
    }
    return j->second;
}

} // namespace OpenGTA
