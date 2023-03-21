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
#include <iostream>
#include <cassert>
#include "file_helper.h"
#include "map.h"
#include "navdata.h"
#include "log.h"
#include "m_exceptions.h"
#include "string_helpers.h"

namespace {

static constexpr PHYSFS_uint8 topHeaderSize = 28;
static constexpr PHYSFS_uint64 baseSize = 262144;

}

/* see http://members.aol.com/form1/fixed.htm for fixed point floats:
 * int_var = (long) fixed_var >> 8; // for 8 bits after point
 */
namespace OpenGTA {

namespace {

inline size_t mapFileName2Number(const std::string & file) {
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

}

  Map::Map(const std::string& filename) {
    nav = 0;
    fd = Util::FileHelper::OpenReadVFS(filename);
    if (!fd) {
      throw E_FILENOTFOUND(filename);
    }
    size_t level_as_num = mapFileName2Number(filename);
    loadHeader();
    loadBase();
    loadColumn();
    loadBlock();
    loadObjects();
    loadRoutes();
    loadLocations();
    loadNavData(level_as_num);
    //dump();
  }
  Map::~Map() {
    if (column)  delete [] column;
    if (block)   delete [] block;
    if (objects) delete [] objects;
    if (nav)     delete    nav;
    if (fd)
      PHYSFS_close(fd);
  }
  int Map::loadHeader() {
    PHYSFS_uint32 vc;
    PHYSFS_readULE32(fd, &vc);
    // INFO("Map version code: {}", vc);
    PHYSFS_uint8 sn;
    PHYSFS_readBytes(fd, static_cast<void*>(&styleNumber), 1);
    // INFO("Style number: {}", int(styleNumber));
    PHYSFS_readBytes(fd, static_cast<void*>(&sn), 1);
    // INFO("Sample number: {}", int(n));
    PHYSFS_uint16 reserved;
    PHYSFS_readULE16(fd, &reserved);
    PHYSFS_readULE32(fd, &routeSize);
    PHYSFS_readULE32(fd, &objectPosSize);
    PHYSFS_readULE32(fd, &columnSize);
    PHYSFS_readULE32(fd, &blockSize);
    PHYSFS_readULE32(fd, &navDataSize);
    /*
    INFO << "Route size: " << routeSize << std::endl;
    INFO << "Object size: " << objectPosSize << std::endl;
    INFO << "Column size: " << columnSize << std::endl;
    INFO << "Block size: " << blockSize << " (" <<
      blockSize / sizeof(BlockInfo) << " blocks " << blockSize % sizeof(BlockInfo)
      << " overcount)" << std::endl;
    INFO << "Navdata size: " << navDataSize << std::endl;
    */
    
    column = new PHYSFS_uint16[columnSize/2];
    block = new BlockInfo[blockSize / sizeof(BlockInfo) ];

    objects = new ObjectPosition[objectPosSize / sizeof(ObjectPosition)];


    return 0;
  }
  int Map::loadBase() {
    PHYSFS_seek(fd, static_cast<PHYSFS_uint64>(topHeaderSize));
    for (int y = 0; y < GTA_MAP_MAXDIMENSION; y++) {
      for(int x = 0; x < GTA_MAP_MAXDIMENSION; x++) {
        PHYSFS_readULE32(fd, &base[x][y]);  
        //std::cout << x << "," << y << " : " << base[x][y] << std::endl;
      }
    }
    return 0;
  }
  int Map::loadColumn() {
    if (!PHYSFS_seek(fd, baseSize + topHeaderSize)) {
      //throw std::string("IO Error while seeking in mapfile");
      throw E_IOERROR(PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
    }
    //PHYSFS_uint16 v;
    for (unsigned int i = 0; i < columnSize/2; i++) {
      PHYSFS_readULE16(fd, &column[i]);
      //std::cout << i << ": " << v << std::endl;
    }
    return 0;
  }
  int Map::loadBlock() {
    PHYSFS_seek(fd, baseSize + columnSize + topHeaderSize);
    int i, max;
    max = blockSize / sizeof(BlockInfo);
    //uint8_t tmp;
    for (i = 0; i < max; i++) {
      PHYSFS_readULE16(fd, &block[i].typeMap);
      PHYSFS_readBytes(fd, static_cast<void*>(&block[i].typeMapExt), 1);
      PHYSFS_readBytes(fd, static_cast<void*>(&block[i].left), 1);
      PHYSFS_readBytes(fd, static_cast<void*>(&block[i].right), 1);
      PHYSFS_readBytes(fd, static_cast<void*>(&block[i].top), 1);
      PHYSFS_readBytes(fd, static_cast<void*>(&block[i].bottom), 1);
      PHYSFS_readBytes(fd, static_cast<void*>(&block[i].lid), 1);
      //block[i].animMode = 0;
    }
    return 0;
  }
  void Map::loadObjects() {
    PHYSFS_seek(fd, baseSize + columnSize + topHeaderSize + blockSize);
    int c = objectPosSize / sizeof(ObjectPosition);
    numObjects = c;
    assert(objectPosSize % sizeof(ObjectPosition) == 0);
    for (int i=0; i < c; i++) {
      PHYSFS_readULE16(fd, &objects[i].x);
      PHYSFS_readULE16(fd, &objects[i].y);
      PHYSFS_readULE16(fd, &objects[i].z);
      PHYSFS_readBytes(fd, static_cast<void*>(&objects[i].type), 1);
      PHYSFS_readBytes(fd, static_cast<void*>(&objects[i].remap), 1);
      PHYSFS_readULE16(fd, &objects[i].rotation);
      PHYSFS_readULE16(fd, &objects[i].pitch);
      PHYSFS_readULE16(fd, &objects[i].roll);
      
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
  void Map::loadRoutes() {
    //FIXME: missing
    PHYSFS_uint32 _si = baseSize + columnSize + topHeaderSize +
      objectPosSize + blockSize;
    PHYSFS_seek(fd, _si);
    PHYSFS_uint32 _counted = 0;
    while (_counted < routeSize) {
      PHYSFS_uint8 num_vertices = 0;
      PHYSFS_uint8 route_type = 0;
      PHYSFS_readBytes(fd, static_cast<void*>(&num_vertices), 1);
      PHYSFS_readBytes(fd, static_cast<void*>(&route_type), 1);
      //INFO << "route-t " << int(route_type) << " with " << int(num_vertices) << " vertices" << std::endl;
      PHYSFS_uint8 x, y, z;
      for (int i=0; i < num_vertices; i++) {
        PHYSFS_readBytes(fd, static_cast<void*>(&x), 1);
        PHYSFS_readBytes(fd, static_cast<void*>(&y), 1);
        PHYSFS_readBytes(fd, static_cast<void*>(&z), 1);
        //INFO << int(x) << "," << int(y) << "," << int(z) << std::endl;
        _counted += 3;
      }

      _counted += 2;
    }
  }
  void Map::loadLocations() {
    //FIXME: missing
    PHYSFS_uint32 _si = baseSize + columnSize + topHeaderSize +
      objectPosSize + routeSize + blockSize;
    PHYSFS_seek(fd, _si); 
    // police
    // hospital
    // unused
    // unused
    // fire
    // unused
    PHYSFS_uint8 loc_type = 0;
    for (int i = 0; i < 36; ++i) {
      Location loc;
      PHYSFS_readBytes(fd, static_cast<void*>(&loc.x), 1);
      PHYSFS_readBytes(fd, static_cast<void*>(&loc.y), 1);
      PHYSFS_readBytes(fd, static_cast<void*>(&loc.z), 1);
      // skip dummy entries at 0,0,0
      if ((loc.x == 0) && (loc.y == 0) && (loc.z == 0))
        continue;
      if (i < 6)
        loc_type = 0;
      else if ((i >= 6) && (i < 12))
        loc_type = 1;
      else if ((i >= 24) && (i < 30))
        loc_type = 2;
      else
        continue;
      //std::cout << int(loc_type) <<": " << int(loc.x) << ", " << int(loc.y) << ", " << int(loc.z) << std::endl;
      locations.insert({loc_type, std::move(loc)});
    }

  }
  void Map::loadNavData(const size_t levelNum) {
    PHYSFS_uint32 _si = baseSize + columnSize + topHeaderSize +
      objectPosSize + routeSize + 3 * 6 * 6 + blockSize;
    PHYSFS_seek(fd, _si); 
    nav = new NavData(navDataSize, fd, levelNum);
    assert(nav);
  }
  PHYSFS_uint16 Map::getNumBlocksAt(PHYSFS_uint8 x, PHYSFS_uint8 y) {
    return column[base[x][y] / 2];
  }
  PHYSFS_uint16 Map::getNumBlocksAtNew(PHYSFS_uint8 x, PHYSFS_uint8 y) {
    return 6 - column[base[x][y] / 2];
  }
  Map::BlockInfo* Map::getBlockAt(PHYSFS_uint8 x, PHYSFS_uint8 y, PHYSFS_uint8 z) {
    PHYSFS_uint16 v = column[base[x][y] / 2 + z];
    return &block[v];
  }
  Map::BlockInfo* Map::getBlockAtNew(PHYSFS_uint8 x, PHYSFS_uint8 y, PHYSFS_uint8 z) {
    PHYSFS_uint16 idx0 = 6 - column[base[x][y] / 2];
    if (idx0 > z)
      idx0 -= z;
    else
      assert(idx0 > z);
    idx0 = column[base[x][y] / 2 + idx0];
    return &block[idx0];
  }
  PHYSFS_uint16 Map::getInternalIdAt(PHYSFS_uint8 x, PHYSFS_uint8 y, PHYSFS_uint8 z) {
    return column[base[x][y] / 2 + z];
  }
  Map::BlockInfo* Map::getBlockByInternalId(PHYSFS_uint16 id) {
    return &block[id];
  }
  void Map::dump() {
    for (int y = 0; y < GTA_MAP_MAXDIMENSION; y++) {
      for(int x = 0; x < GTA_MAP_MAXDIMENSION; x++) {
        std::cout << x << "," << y << ":" << column[base[x][y] / 2] << "||";
        PHYSFS_uint16 ts = column[base[x][y] / 2];
        std::cout << "(";
        for(int t=1; t <= (6 - ts); t++) {
          BlockInfo *info = &block[column[base[x][y] / 2 + t]];
          std::cout << int(info->slopeType()) << ", ";
        
        }
        std::cout << ")" << std::endl;
      }
    }
  }
  const Map::Location & Map::getNearestLocationByType(uint8_t t, uint8_t x, uint8_t y) {
    INFO("{} at {} {}", int(t), int(x), int(y));
    LocationMap::iterator i = locations.find(t);
    LocationMap::iterator j;
    if (i == locations.end())
      throw E_UNKNOWNKEY("location-type " + std::to_string(int(t))
                         + " not found in map");
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

}
