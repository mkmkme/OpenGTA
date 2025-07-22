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

#include <vector>

#include <physfs.h>

#include <core/car-info.h>
#include <core/numeric-types.h>
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
    [[nodiscard]] UInt8 getFormat() const;

    struct SpriteNumbers {

#define DECLARE_SPRITE_NUMBER(name) UInt16 name;
        SPRITE_TYPES(DECLARE_SPRITE_NUMBER)

        enum class SpriteTypes : UInt8 {

#define DECLARE_SPRITE_TYPE(name) name,
            SPRITE_TYPES(DECLARE_SPRITE_TYPE)

        };

        [[nodiscard]] UInt16 reIndex(const UInt16 &id, const enum SpriteTypes &st) const;
        [[nodiscard]] UInt16 countByType(const SpriteTypes &t) const;
    };

    bool isAnimatedBlock(UInt8 area_code, UInt8 id);

    static const char *getSpriteName(int t);
    static const char *getSpriteName(SpriteNumbers::SpriteTypes t) { return getSpriteName(static_cast<int>(t)); }

    void prepareSideTexture(UInt32 idx, std::span<UInt8> dst);
    void prepareLidTexture(UInt32 idx, std::span<UInt8> dst);
    void prepareAuxTexture(UInt32 idx, std::span<UInt8> dst);
    [[nodiscard]] unsigned int getRandomPedRemapNumber() const;
    unsigned int getPedRemapNumberType(unsigned int _type);

    SpriteNumbers spriteNumbers {};

    CarInfo &findCarByModel(UInt8);
    // [[nodiscard]] inline size_t getNumCarModels() const noexcept { return carInfos.size(); }
    [[nodiscard]] std::span<const UInt8> getTmpBuffer(bool rgba) const;
    SpriteInfo &getSprite(size_t id) { return spriteInfos[id]; }

    virtual std::span<const UInt8> getSide(UInt8 idx, unsigned int palIdx, bool rgba) = 0;
    virtual std::span<const UInt8> getLid(UInt8 idx, unsigned int palIdx, bool rgba) = 0;
    virtual std::span<const UInt8> getAux(UInt8 idx, unsigned int palIdx, bool rgba) = 0;

    virtual std::vector<UInt8> getSpriteBitmap(size_t id, int remap, UInt32 delta) = 0;

    std::vector<LoadedAnim> animations;
    std::vector<SpriteInfo> spriteInfos;
    std::vector<ObjectInfo> objectInfos;
    std::vector<CarInfo> carInfos;

    [[nodiscard]] bool getDeltaHandling() const;
    void setDeltaHandling(bool delta_as_set);

    [[nodiscard]] bool isBlockingSide(UInt8 id) const;
    void setupBlocking();

protected:
    void loadTileTextures();
    void loadAnim();

    void loadObjectInfo_shared(UInt64 offset);
    void loadSpriteNumbers_shared(UInt64 offset);
    void loadCarInfo_shared(UInt64 offset);
    // void loadSpriteInfo_shared(UInt64 offset);

    void handleDeltas(const SpriteInfo &spriteinfo, unsigned char *buffer, UInt32 delta);
    void applyDelta(
        const SpriteInfo &spriteInfo,
        unsigned char *buffer,
        UInt32 offset,
        const DeltaInfo &deltaInfo,
        bool mirror = false
    );

    Util::PhysFSFile styleFile;
    std::vector<UInt8> rawTiles;
    std::vector<UInt8> rawSprites;

    UInt32 sideSize {};
    UInt32 lidSize {};
    UInt32 auxSize {};
    UInt32 animSize {};
    UInt32 objectInfoSize {};
    UInt32 carInfoSize {};
    UInt32 spriteInfoSize {};
    UInt32 spriteGraphicsSize {};
    UInt32 spriteNumberSize {};

    UInt32 auxBlockTrailSize {};

    UInt8 _topHeaderSize {};

    std::array<UInt8, 4096> tileTmp {};
    std::array<UInt8, 4096 * 3> tileTmpRGB {};
    std::array<UInt8, 4096 * 4> tileTmpRGBA {};

    bool delta_is_a_set;

    Util::Set sideTexBlockMove;

    unsigned int firstValidPedRemap {};
    unsigned int lastValidPedRemap {};
};

} // namespace OpenGTA
#endif
