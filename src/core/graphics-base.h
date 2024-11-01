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

#include <memory>
#include <vector>

#include <physfs.h>

#include <core/car-info.h>
#include <core/numeric-types.h>
#include <core/object-info.h>

#include "util/file-manager.h"
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
    uint8_t getFormat();

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

        enum SpriteTypes {
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

        UInt16 reIndex(const UInt16 &id, const enum SpriteTypes &st) const;
        UInt16 countByType(const SpriteTypes &t) const;
    };

    bool isAnimatedBlock(uint8_t area_code, uint8_t id);

    void prepareSideTexture(unsigned int idx, unsigned char *dst);
    void prepareLidTexture(unsigned int idx, unsigned char *dst);
    void prepareAuxTexture(unsigned int idx, unsigned char *dst);
    unsigned int getRandomPedRemapNumber() const;
    unsigned int getPedRemapNumberType(unsigned int _type);

    SpriteNumbers spriteNumbers {};

    CarInfo &findCarByModel(UInt8);
    [[maybe_unused]] [[nodiscard]] inline size_t getNumCarModels() const noexcept { return carInfos.size(); }
    unsigned char *getTmpBuffer(bool rgba);
    SpriteInfo *getSprite(size_t id) { return spriteInfos[id]; }

    virtual unsigned char *getSide(unsigned int idx, unsigned int palIdx, bool rgba) = 0;
    virtual unsigned char *getLid(unsigned int idx, unsigned int palIdx, bool rgba) = 0;
    virtual unsigned char *getAux(unsigned int idx, unsigned int palIdx, bool rgba) = 0;

    virtual std::vector<UInt8> getSpriteBitmap(size_t id, int remap, uint32_t delta) = 0;

    std::vector<LoadedAnim> animations;
    std::vector<SpriteInfo *> spriteInfos;
    std::vector<ObjectInfo> objectInfos;
    std::vector<CarInfo> carInfos;

    bool getDeltaHandling();
    void setDeltaHandling(bool delta_as_set);

    bool isBlockingSide(uint8_t id);
    void setupBlocking();

protected:
    void loadTileTextures();
    void loadAnim();

    void loadObjectInfo_shared(UInt64 offset);
    void loadSpriteNumbers_shared(UInt64 offset);
    void loadCarInfo_shared(UInt64 offset);
    // void loadSpriteInfo_shared(UInt64 offset);

    void handleDeltas(const SpriteInfo &spriteinfo, unsigned char *buffer, uint32_t delta);
    void applyDelta(
        const SpriteInfo &spriteInfo,
        unsigned char *buffer,
        uint32_t offset,
        const DeltaInfo &deltaInfo,
        bool mirror = false
    );

    Util::PhysFSFile styleFile;
    unsigned char *rawTiles;
    unsigned char *rawSprites;

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

    unsigned char tileTmp[4096] {};
    unsigned char tileTmpRGB[4096 * 3] {};
    unsigned char tileTmpRGBA[4096 * 4] {};

    bool delta_is_a_set;

    Util::Set sideTexBlockMove;

    unsigned int firstValidPedRemap {};
    unsigned int lastValidPedRemap {};
};

} // namespace OpenGTA
#endif
