#pragma once

#include <map>
#include <physfs.h>
#include <string>

namespace {
static constexpr std::size_t GTA_MAP_MAXDIMENSION = 256;
}

namespace OpenGTA {

class NavData; // see navdata.h

/** the wrapper for the CMP (compressed map) files */
class Map {
    friend class MapViewGL;

public:
    Map(const std::string &filename);
    ~Map();

    struct BlockInfo {
        PHYSFS_uint16 typeMap;
        PHYSFS_uint8 typeMapExt;
        PHYSFS_uint8 left, right, top, bottom, lid;

        inline bool upOk() const noexcept { return (typeMap & 1); }
        inline bool downOk() const noexcept { return (typeMap & 2); }
        inline bool leftOk() const noexcept { return (typeMap & 4); }
        inline bool rightOk() const noexcept { return (typeMap & 8); }
        inline uint8_t blockType() const noexcept
        {
            return ((typeMap & 16 ? 1 : 0) + (typeMap & 32 ? 2 : 0)
                    + (typeMap & 64 ? 4 : 0));
        }
        inline bool isFlat() const noexcept { return (typeMap & 128); }
        inline uint8_t slopeType() const noexcept
        {
            return ((typeMap & 256 ? 1 : 0) + (typeMap & 512 ? 2 : 0)
                    + (typeMap & 1024 ? 4 : 0) + (typeMap & 2048 ? 8 : 0)
                    + (typeMap & 4096 ? 16 : 0) + (typeMap & 8192 ? 32 : 0));
        }
        inline uint8_t rotation() const noexcept
        {
            return ((typeMap & 16384 ? 1 : 0) + (typeMap & 32768 ? 2 : 0));
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

        inline void setUpOk(bool v) noexcept
        {
            if (v)
                typeMap |= 1;
            else
                typeMap &= ~1;
        }
        inline void setDownOk(bool v) noexcept
        {
            if (v)
                typeMap |= 2;
            else
                typeMap &= ~2;
        }
        inline void setLeftOk(bool v) noexcept
        {
            if (v)
                typeMap |= 4;
            else
                typeMap &= ~4;
        }
        inline void setRightOk(bool v) noexcept
        {
            if (v)
                typeMap |= 8;
            else
                typeMap &= ~8;
        }
        inline void setIsFlat(bool v) noexcept
        {
            if (v)
                typeMap |= 128;
            else
                typeMap &= ~128;
        }
        void setBlockType(uint8_t v);
        void setSlopeType(uint8_t v);
        void setRotation(uint8_t v);

        inline bool trafficLights() const noexcept { return (typeMapExt & 1); }
        inline bool railEndTurn() const noexcept { return (typeMapExt & 4); }
        inline bool railStartTurn() const noexcept
        {
            return ((typeMapExt & 4) && (typeMapExt & 1));
        }
        inline bool railStation() const noexcept
        {
            return ((typeMapExt & 4) && (typeMapExt & 2));
        }
        inline bool railStationTrain() const noexcept
        {
            return ((typeMapExt & 4) && (typeMapExt & 2) && (typeMapExt & 1));
        }
        inline uint8_t remapIndex() const noexcept
        {
            return ((typeMapExt & 8 ? 1 : 0) + (typeMapExt & 16 ? 2 : 0));
        }
        inline bool flipTopBottom() const noexcept { return (typeMapExt & 32); }
        inline bool flipLeftRight() const noexcept { return (typeMapExt & 64); }
        inline bool railway() const noexcept { return (typeMapExt & 128); }
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
    BlockInfo *getBlockByInternalId(PHYSFS_uint16 id);
    PHYSFS_uint16 getInternalIdAt(PHYSFS_uint8 x,
                                  PHYSFS_uint8 y,
                                  PHYSFS_uint8 z);
    void dump();
    NavData *nav;
    ObjectPosition *objects;
    PHYSFS_uint16 numObjects;
    const Location &getNearestLocationByType(uint8_t t, uint8_t x, uint8_t y);
    const LocationMap &getLocationMap() const noexcept { return locations; }

protected:
    PHYSFS_uint32 base[GTA_MAP_MAXDIMENSION][GTA_MAP_MAXDIMENSION];
    PHYSFS_uint16 *column;
    BlockInfo *block;
    LocationMap locations;

private:
    PHYSFS_file *fd;

    PHYSFS_uint8 styleNumber;
    PHYSFS_uint32 routeSize;
    PHYSFS_uint32 objectPosSize;
    PHYSFS_uint32 columnSize;
    PHYSFS_uint32 blockSize;
    PHYSFS_uint32 navDataSize;

    int loadHeader();
    int loadBase();
    int loadColumn();
    int loadBlock();
    void loadObjects();
    void loadRoutes();
    void loadLocations();
    void loadNavData(const size_t level_num);
    static constexpr PHYSFS_uint8 _topHeaderSize = 28;
    static constexpr PHYSFS_uint64 _baseSize = 262144;
};

} // namespace OpenGTA
