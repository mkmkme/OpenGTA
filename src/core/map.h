#pragma once

#include <map>
#include <optional>
#include <string>
#include <vector>

#include <physfs.h>

#include <core/navdata.h>

#include <common/bitwise.h>
#include <util/file-manager.h>

constexpr std::size_t GTA_MAP_MAXDIMENSION = 256;

namespace OpenGTA {

class NavData; // see navdata.h

/** the wrapper for the CMP (compressed map) files */
class Map {
public:
    explicit Map(const std::string &filename);

    struct BlockInfo {
        uint16_t typeMap;
        uint8_t typeMapExt;
        uint8_t left, right, top, bottom, lid;

        [[nodiscard]] bool upOk() const noexcept { return Util::getBit(typeMap, 1); }
        [[nodiscard]] bool downOk() const noexcept { return Util::getBit(typeMap, 2); }
        [[nodiscard]] bool leftOk() const noexcept { return Util::getBit(typeMap, 3); }
        [[nodiscard]] bool rightOk() const noexcept { return Util::getBit(typeMap, 4); }
        [[nodiscard]] uint8_t blockType() const noexcept
        {
            return static_cast<uint8_t>(Util::getRangeBit(typeMap, 5, 7));
        }
        [[nodiscard]] bool isFlat() const noexcept { return Util::getBit(typeMap, 8); }
        [[nodiscard]] uint8_t slopeType() const noexcept
        {
            return static_cast<uint8_t>(Util::getRangeBit(typeMap, 9, 14));
        }
        [[nodiscard]] uint8_t rotation() const noexcept
        {
            return static_cast<uint8_t>(Util::getRangeBit(typeMap, 15, 16));
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
        [[nodiscard]] bool railStation() const noexcept
        {
            return Util::getBit(typeMapExt, 3) && Util::getBit(typeMapExt, 2);
        }
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
        uint16_t x, y, z;
        uint8_t type;
        uint8_t remap;
        uint16_t rotation; // see: cds.doc
        uint16_t pitch;
        uint16_t roll;
    };
    struct Location {
        uint8_t x;
        uint8_t y;
        uint8_t z;

        explicit Location(Util::PhysFSFile &pf);
    };
    using LocationMap = std::multimap<uint8_t, Location>;
    //...
    uint16_t getNumBlocksAt(uint8_t x, uint8_t y);
    uint16_t getNumBlocksAtNew(uint8_t x, uint8_t y);
    BlockInfo *getBlockAt(uint8_t x, uint8_t y, uint8_t z);
    BlockInfo *getBlockAtNew(uint8_t x, uint8_t y, uint8_t z);
    BlockInfo *getBlockByInternalId(uint16_t id);
    uint16_t getInternalIdAt(uint8_t x, uint8_t y, uint8_t z);
    void dump();
    std::optional<NavData> nav;
    std::vector<ObjectPosition> objects;
    uint16_t numObjects {};
    const Location &getNearestLocationByType(uint8_t t, int x, int y);
    [[nodiscard]] const LocationMap &getLocationMap() const noexcept { return locations; }

protected:
    uint32_t base[GTA_MAP_MAXDIMENSION][GTA_MAP_MAXDIMENSION] {};
    std::vector<uint16_t> column;
    std::vector<BlockInfo> block;
    LocationMap locations;

private:
    Util::PhysFSFile pf;

    uint8_t styleNumber {};
    uint32_t routeSize {};
    uint32_t objectPosSize {};
    uint32_t columnSize {};
    uint32_t blockSize {};
    uint32_t navDataSize {};

    void loadHeader();
    void loadBase();
    void loadColumn();
    void loadBlock();
    void loadObjects();
    void loadRoutes();
    void loadLocations();
    void loadNavData(size_t level_num);
};

} // namespace OpenGTA
