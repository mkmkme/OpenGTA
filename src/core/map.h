#pragma once

#include <map>
#include <string>

#include <physfs.h>

#include <core/numeric-types.h>

#include <common/bitwise.h>
#include <util/file-manager.h>

constexpr std::size_t GTA_MAP_MAXDIMENSION = 256;

namespace OpenGTA {

class NavData; // see navdata.h

/** the wrapper for the CMP (compressed map) files */
class Map {
public:
    explicit Map(const std::string &filename);
    ~Map();

    struct BlockInfo {
        UInt16 typeMap;
        UInt8 typeMapExt;
        UInt8 left, right, top, bottom, lid;

        [[nodiscard]] bool upOk() const noexcept { return Util::getBit(typeMap, 1); }
        [[nodiscard]] bool downOk() const noexcept { return Util::getBit(typeMap, 2); }
        [[nodiscard]] bool leftOk() const noexcept { return Util::getBit(typeMap, 3); }
        [[nodiscard]] bool rightOk() const noexcept { return Util::getBit(typeMap, 4); }
        [[nodiscard]] uint8_t blockType() const noexcept { return Util::getRangeBit(typeMap, 5, 7); }
        [[nodiscard]] bool isFlat() const noexcept { return Util::getBit(typeMap, 8); }
        [[nodiscard]] uint8_t slopeType() const noexcept { return Util::getRangeBit(typeMap, 9, 14); }
        [[nodiscard]] uint8_t rotation() const noexcept { return Util::getRangeBit(typeMap, 15, 16); }
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

        void setUpOk(bool v) noexcept { Util::setBit(&typeMap, 1, v); }
        void setDownOk(bool v) noexcept { Util::setBit(&typeMap, 2, v); }
        void setLeftOk(bool v) noexcept { Util::setBit(&typeMap, 3, v); }
        void setRightOk(bool v) noexcept { Util::setBit(&typeMap, 4, v); }
        void setIsFlat(bool v) noexcept { Util::setBit(&typeMap, 8, v); }
        void setBlockType(uint8_t v) noexcept { Util::copyRangeBit(&typeMap, 5, 7, v); }
        void setSlopeType(uint8_t v) noexcept { Util::copyRangeBit(&typeMap, 9, 14, v); }
        void setRotation(uint8_t v) noexcept { Util::copyRangeBit(&typeMap, 15, 16, v); }

        [[nodiscard]] bool trafficLights() const noexcept { return Util::getBit(typeMapExt, 1); }
        [[nodiscard]] bool railEndTurn() const noexcept { return Util::getBit(typeMapExt, 3); }
        [[nodiscard]] bool railStartTurn() const noexcept
        {
            return Util::getBit(typeMapExt, 3) && Util::getBit(typeMapExt, 1);
        }
        [[nodiscard]] bool railStation() const noexcept { return Util::getBit(typeMapExt, 3) && Util::getBit(typeMapExt, 2); }
        [[nodiscard]] bool railStationTrain() const noexcept
        {
            return Util::getBit(typeMapExt, 3) && Util::getBit(typeMapExt, 2) && Util::getBit(typeMapExt, 1);
        }
        [[nodiscard]] uint8_t remapIndex() const noexcept { return Util::getRangeBit(typeMapExt, 4, 5); }
        [[nodiscard]] bool flipTopBottom() const noexcept { return Util::getBit(typeMapExt, 6); }
        [[nodiscard]] bool flipLeftRight() const noexcept { return Util::getBit(typeMapExt, 7); }
        [[nodiscard]] bool railway() const noexcept { return Util::getBit(typeMapExt, 8); }
    };
    struct ObjectPosition {
        UInt16 x, y, z;
        UInt8 type;
        UInt8 remap;
        UInt16 rotation; // see: cds.doc
        UInt16 pitch;
        UInt16 roll;
    };
    struct Location {
        UInt8 x = 0;
        UInt8 y = 0;
        UInt8 z = 0;
    };
    using LocationMap = std::multimap<UInt8, Location>;
    //...
    UInt16 getNumBlocksAt(UInt8 x, UInt8 y);
    UInt16 getNumBlocksAtNew(UInt8 x, UInt8 y);
    BlockInfo *getBlockAt(UInt8 x, UInt8 y, UInt8 z);
    BlockInfo *getBlockAtNew(UInt8 x, UInt8 y, UInt8 z);
    BlockInfo *getBlockByInternalId(UInt16 id);
    UInt16 getInternalIdAt(UInt8 x, UInt8 y, UInt8 z);
    void dump();
    NavData *nav;
    ObjectPosition *objects {};
    UInt16 numObjects {};
    const Location &getNearestLocationByType(uint8_t t, uint8_t x, uint8_t y);
    [[nodiscard]] const LocationMap &getLocationMap() const noexcept { return locations; }

protected:
    UInt32 base[GTA_MAP_MAXDIMENSION][GTA_MAP_MAXDIMENSION] {};
    UInt16 *column {};
    BlockInfo *block {};
    LocationMap locations;

private:
    Util::PhysFSFile pf;

    UInt8 styleNumber {};
    UInt32 routeSize {};
    UInt32 objectPosSize {};
    UInt32 columnSize {};
    UInt32 blockSize {};
    UInt32 navDataSize {};

    int loadHeader();
    int loadBase();
    int loadColumn();
    int loadBlock();
    void loadObjects();
    void loadRoutes();
    void loadLocations();
    void loadNavData(size_t level_num);
};

} // namespace OpenGTA
