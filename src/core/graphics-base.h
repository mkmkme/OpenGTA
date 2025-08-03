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
#ifndef OPENGTA_MAIN_H
#define OPENGTA_MAIN_H

#include <cstdint>
#include <vector>

#include <physfs.h>

#include <core/car-info.h>
#include <core/object-info.h>
#include <core/sprite-info.h>

#include <util/file-manager.h>
#include <util/set.h>

namespace OpenGTA {

struct DeltaInfo;
struct LoadedAnim;
struct SpriteInfo;

#define SPRITE_TYPES(DECLARE) \
    DECLARE(arrow)            \
    DECLARE(digits)           \
    DECLARE(boat)             \
    DECLARE(box)              \
    DECLARE(bus)              \
    DECLARE(car)              \
    DECLARE(object)           \
    DECLARE(ped)              \
    DECLARE(speedo)           \
    DECLARE(tank)             \
    DECLARE(traffic_lights)   \
    DECLARE(train)            \
    DECLARE(trdoors)          \
    DECLARE(bike)             \
    DECLARE(tram)             \
    DECLARE(wbus)             \
    DECLARE(wcar)             \
    DECLARE(ex)               \
    DECLARE(tumcar)           \
    DECLARE(tumtruck)         \
    DECLARE(ferry)

/** The common class for all graphics wrappers.
 * Contains a number of common variables; does essentially nothing.
 */
class GraphicsBase {
public:
    explicit GraphicsBase(const std::string &style);
    virtual ~GraphicsBase();
    [[nodiscard]] uint8_t getFormat() const;

    struct SpriteNumbers {

#define DECLARE_SPRITE_NUMBER(name) uint16_t name;
        SPRITE_TYPES(DECLARE_SPRITE_NUMBER)

        enum class SpriteType : uint8_t {

#define DECLARE_SPRITE_TYPE(name) name,
            SPRITE_TYPES(DECLARE_SPRITE_TYPE)

        };

        [[nodiscard]] uint16_t reIndex(uint16_t id, SpriteType st) const;
        [[nodiscard]] uint16_t countByType(const SpriteType &t) const;
    };

    bool isAnimatedBlock(uint8_t area_code, uint8_t id);

    static const char *getSpriteName(int t);
    static const char *getSpriteName(SpriteNumbers::SpriteType t) { return getSpriteName(static_cast<int>(t)); }

    void prepareSideTexture(uint32_t idx, std::span<uint8_t> dst);
    void prepareLidTexture(uint32_t idx, std::span<uint8_t> dst);
    void prepareAuxTexture(uint32_t idx, std::span<uint8_t> dst);
    [[nodiscard]] uint16_t getRandomPedRemapNumber() const;
    uint16_t getPedRemapNumberType(unsigned int _type);

    SpriteNumbers spriteNumbers {};

    CarInfo &findCarByModel(uint8_t);
    // [[nodiscard]] inline size_t getNumCarModels() const noexcept { return carInfos.size(); }
    [[nodiscard]] std::span<const uint8_t> getTmpBuffer(bool rgba) const;
    SpriteInfo &getSprite(size_t id) { return spriteInfos[id]; }

    virtual std::span<const uint8_t> getSide(uint8_t idx, unsigned int palIdx, bool rgba) = 0;
    virtual std::span<const uint8_t> getLid(uint8_t idx, unsigned int palIdx, bool rgba) = 0;
    virtual std::span<const uint8_t> getAux(uint8_t idx, unsigned int palIdx, bool rgba) = 0;

    virtual std::vector<uint8_t> getSpriteBitmap(size_t id, int remap, uint32_t delta) = 0;

    std::vector<LoadedAnim> animations;
    std::vector<SpriteInfo> spriteInfos;
    std::vector<ObjectInfo> objectInfos;
    std::vector<CarInfo> carInfos;

    [[nodiscard]] bool getDeltaHandling() const;
    void setDeltaHandling(bool delta_as_set);

    [[nodiscard]] bool isBlockingSide(uint8_t id) const;
    void setupBlocking();

protected:
    void loadTileTextures();
    void loadAnim();

    void loadObjectInfo_shared(uint64_t offset);
    void loadSpriteNumbers_shared(uint64_t offset);
    void loadCarInfo_shared(uint64_t offset);
    // void loadSpriteInfo_shared(uint64_t offset);

    void handleDeltas(const SpriteInfo &spriteinfo, unsigned char *buffer, uint32_t delta);
    void applyDelta(
        const SpriteInfo &spriteInfo,
        unsigned char *buffer,
        uint32_t offset,
        const DeltaInfo &deltaInfo,
        bool mirror = false
    );

    Util::PhysFSFile styleFile;
    std::vector<uint8_t> rawTiles;
    std::vector<uint8_t> rawSprites;

    uint32_t sideSize {};
    uint32_t lidSize {};
    uint32_t auxSize {};
    uint32_t animSize {};
    uint32_t objectInfoSize {};
    uint32_t carInfoSize {};
    uint32_t spriteInfoSize {};
    uint32_t spriteGraphicsSize {};
    uint32_t spriteNumberSize {};

    uint32_t auxBlockTrailSize {};

    uint8_t _topHeaderSize {};

    std::array<uint8_t, 4096> tileTmp {};
    std::array<uint8_t, 4096 * 3> tileTmpRGB {};
    std::array<uint8_t, 4096 * 4> tileTmpRGBA {};

    bool delta_is_a_set;

    Util::Set sideTexBlockMove;

    uint16_t firstValidPedRemap {};
    uint16_t lastValidPedRemap {};
};

} // namespace OpenGTA
#endif
