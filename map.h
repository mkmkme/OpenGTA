#pragma once

#include <map>
#include <physfs.h>
#include <string>

#include "bitwise.hpp"

namespace {
constexpr std::size_t GTA_MAP_MAXDIMENSION = 256;
}

namespace OpenGTA {

class NavData; // see navdata.h

/** the wrapper for the CMP (compressed map) files */
class Map {
private:
    explicit Map(std::string filename);
public:
    ~Map();

    static std::unique_ptr<Map> create(std::string filename);
    
    struct BlockInfo {
        PHYSFS_uint16 typeMap;
        PHYSFS_uint8 typeMapExt;
        PHYSFS_uint8 left, right, top, bottom, lid;

        inline bool upOk() const noexcept { return Util::getBit(typeMap, 1); }
        inline bool downOk() const noexcept { return Util::getBit(typeMap, 2); }
        inline bool leftOk() const noexcept { return Util::getBit(typeMap, 3); }
        inline bool rightOk() const noexcept { return Util::getBit(typeMap, 4); }
        inline uint8_t blockType() const noexcept { return Util::getRangeBit(typeMap, 5, 7); }
        inline bool isFlat() const noexcept { return Util::getBit(typeMap, 8); }
        inline uint8_t slopeType() const noexcept
        {
            return Util::getRangeBit(typeMap, 9, 14);
        }
        inline uint8_t rotation() const noexcept
        {
            return Util::getRangeBit(typeMap, 15, 16);
        }
        /* m1win seems to indicate:
         * 000 - Nothing
         * 001 - traffic lights
         * 010 - invalid
         * 011 - invalid
         * 100 - railway end turn
         * 101 - railway start turn
         * 110 - railway station
         * 111 - railway station train
         */

        inline void setUpOk(bool v) noexcept { Util::setBit(&typeMap, 1, v); }
        inline void setDownOk(bool v) noexcept { Util::setBit(&typeMap, 2, v); }
        inline void setLeftOk(bool v) noexcept { Util::setBit(&typeMap, 3, v); }
        inline void setRightOk(bool v) noexcept { Util::setBit(&typeMap, 4, v); }
        inline void setIsFlat(bool v) noexcept { Util::setBit(&typeMap, 8, v); }
        inline void setBlockType(uint8_t v) noexcept { Util::copyRangeBit(&typeMap, 5, 7, v); }
        inline void setSlopeType(uint8_t v) noexcept { Util::copyRangeBit(&typeMap, 9, 14, v); }
        inline void setRotation(uint8_t v) noexcept { Util::copyRangeBit(&typeMap, 15, 16, v); }

        inline bool trafficLights() const noexcept { return Util::getBit(typeMapExt, 1); }
        inline bool railEndTurn() const noexcept { return Util::getBit(typeMapExt, 3); }
        inline bool railStartTurn() const noexcept
        {
            return Util::getBit(typeMapExt, 3) && Util::getBit(typeMapExt, 1);
        }
        inline bool railStation() const noexcept
        {
            return Util::getBit(typeMapExt, 3) && Util::getBit(typeMapExt, 2);
        }
        inline bool railStationTrain() const noexcept
        {
            return Util::getBit(typeMapExt, 3) && Util::getBit(typeMapExt, 2) &&
                   Util::getBit(typeMapExt, 1);
        }
        inline uint8_t remapIndex() const noexcept
        {
            return Util::getRangeBit(typeMapExt, 4, 5);
        }
        inline bool flipTopBottom() const noexcept { return Util::getBit(typeMapExt, 6); }
        inline bool flipLeftRight() const noexcept { return Util::getBit(typeMapExt, 7); }
        inline bool railway() const noexcept { return Util::getBit(typeMapExt, 8); }
    };
    struct ObjectPosition {
        PHYSFS_uint16 x, y, z;
        PHYSFS_uint8 type;
        PHYSFS_uint8 remap;
        PHYSFS_uint16 rotation; // see: cds.doc
        PHYSFS_uint16 pitch;
        PHYSFS_uint16 roll;
    };
    struct Location {
        PHYSFS_uint8 x = 0;
        PHYSFS_uint8 y = 0;
        PHYSFS_uint8 z = 0;
    };
    using LocationMap = std::multimap<PHYSFS_uint8, Location>;
    //...
    PHYSFS_uint16 getNumBlocksAt(PHYSFS_uint8 x, PHYSFS_uint8 y);
    PHYSFS_uint16 getNumBlocksAtNew(PHYSFS_uint8 x, PHYSFS_uint8 y);
    BlockInfo *getBlockAt(PHYSFS_uint8 x, PHYSFS_uint8 y, PHYSFS_uint8 z);
    BlockInfo *getBlockAtNew(PHYSFS_uint8 x, PHYSFS_uint8 y, PHYSFS_uint8 z);
    NavData *nav;
    ObjectPosition *objects{};
    PHYSFS_uint16 numObjects{};
    const LocationMap &getLocationMap() const noexcept { return locations; }



protected:
    PHYSFS_uint32 base[GTA_MAP_MAXDIMENSION][GTA_MAP_MAXDIMENSION]{};
    PHYSFS_uint16 *column{};
    BlockInfo *block{};
    LocationMap locations;

private:
    PHYSFS_file *fd;

    PHYSFS_uint8 styleNumber{};
    PHYSFS_uint32 routeSize{};
    PHYSFS_uint32 objectPosSize{};
    PHYSFS_uint32 columnSize{};
    PHYSFS_uint32 blockSize{};
    PHYSFS_uint32 navDataSize{};

    int loadHeader();
    int loadBase();
    int loadColumn();
    int loadBlock();
    void loadObjects();
    void loadRoutes();
    void loadLocations();
    void loadNavData(const size_t level_num);
};


} // namespace OpenGTA
