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

/** The common class for all graphics wrappers.
 * Contains a number of common variables; does essentially nothing.
 */
class GraphicsBase {
public:
    explicit GraphicsBase(const std::string &style);
    virtual ~GraphicsBase();
    [[nodiscard]] UInt8 getFormat() const;

    struct SpriteNumbers {
        UInt16 GTA_SPRITE_ARROW;
        UInt16 GTA_SPRITE_DIGITS;
        UInt16 GTA_SPRITE_BOAT;
        UInt16 GTA_SPRITE_BOX;
        UInt16 GTA_SPRITE_BUS;
        UInt16 GTA_SPRITE_CAR;
        UInt16 GTA_SPRITE_OBJECT;
        UInt16 GTA_SPRITE_PED;
        UInt16 GTA_SPRITE_SPEEDO;
        UInt16 GTA_SPRITE_TANK;
        UInt16 GTA_SPRITE_TRAFFIC_LIGHTS;
        UInt16 GTA_SPRITE_TRAIN;
        UInt16 GTA_SPRITE_TRDOORS;
        UInt16 GTA_SPRITE_BIKE;
        UInt16 GTA_SPRITE_TRAM;
        UInt16 GTA_SPRITE_WBUS;
        UInt16 GTA_SPRITE_WCAR;
        UInt16 GTA_SPRITE_EX;
        UInt16 GTA_SPRITE_TUMCAR;
        UInt16 GTA_SPRITE_TUMTRUCK;
        UInt16 GTA_SPRITE_FERRY;

        enum SpriteTypes : UInt8 {
            ARROW = 0,
            DIGIT,
            BOAT,
            BOX,
            BUS,
            CAR,
            OBJECT,
            PED,
            SPEEDO,
            TANK,
            TRAFFIC_LIGHT,
            TRAIN,
            TRDOOR,
            BIKE,
            TRAM,
            WBUS,
            WCAR,
            EX,
            TUMCAR,
            TUMTRUCK,
            FERRY
        };

        [[nodiscard]] UInt16 reIndex(const UInt16 &id, const enum SpriteTypes &st) const;
        [[nodiscard]] UInt16 countByType(const SpriteTypes &t) const;
    };

    bool isAnimatedBlock(UInt8 area_code, UInt8 id);

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
