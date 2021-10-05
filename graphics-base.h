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

#include "car-info.h"
#include "set.h"

#include <SDL.h>
#include <map>
#include <memory>
#include <physfs.h>
#include <string>
#include <vector>

namespace OpenGTA {

struct DeltaInfo;
struct LoadedAnim;
struct ObjectInfo;
struct SpriteInfo;

/** The common class for all graphics wrappers.
 * Contains a number of common variables; does essentially nothing.
 */
class GraphicsBase {
public:
    GraphicsBase();
    virtual ~GraphicsBase();
    uint8_t getFormat();

    struct SpriteNumbers {
        PHYSFS_uint16 GTA_SPRITE_ARROW;
        PHYSFS_uint16 GTA_SPRITE_DIGITS;
        PHYSFS_uint16 GTA_SPRITE_BOAT;
        PHYSFS_uint16 GTA_SPRITE_BOX;
        PHYSFS_uint16 GTA_SPRITE_BUS;
        PHYSFS_uint16 GTA_SPRITE_CAR;
        PHYSFS_uint16 GTA_SPRITE_OBJECT;
        PHYSFS_uint16 GTA_SPRITE_PED;
        PHYSFS_uint16 GTA_SPRITE_SPEEDO;
        PHYSFS_uint16 GTA_SPRITE_TANK;
        PHYSFS_uint16 GTA_SPRITE_TRAFFIC_LIGHTS;
        PHYSFS_uint16 GTA_SPRITE_TRAIN;
        PHYSFS_uint16 GTA_SPRITE_TRDOORS;
        PHYSFS_uint16 GTA_SPRITE_BIKE;
        PHYSFS_uint16 GTA_SPRITE_TRAM;
        PHYSFS_uint16 GTA_SPRITE_WBUS;
        PHYSFS_uint16 GTA_SPRITE_WCAR;
        PHYSFS_uint16 GTA_SPRITE_EX;
        PHYSFS_uint16 GTA_SPRITE_TUMCAR;
        PHYSFS_uint16 GTA_SPRITE_TUMTRUCK;
        PHYSFS_uint16 GTA_SPRITE_FERRY;

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

        PHYSFS_uint16 reIndex(const PHYSFS_uint16 &id,
                              const enum SpriteTypes &st) const;
        PHYSFS_uint16 countByType(const SpriteTypes &t) const;
    };

    bool isAnimatedBlock(uint8_t area_code, uint8_t id);

    void prepareSideTexture(unsigned int idx, unsigned char *dst);
    void prepareLidTexture(unsigned int idx, unsigned char *dst);
    void prepareAuxTexture(unsigned int idx, unsigned char *dst);
    unsigned int getRandomPedRemapNumber();
    unsigned int getPedRemapNumberType(unsigned int _type);

    SpriteNumbers spriteNumbers;

    CarInfo &findCarByModel(PHYSFS_uint8);
    inline size_t getNumCarModels() const noexcept { return carInfos.size(); }
    unsigned char *getTmpBuffer(bool rgba);
    SpriteInfo *getSprite(size_t id) { return spriteInfos[id]; }

    virtual unsigned char *getSide(unsigned int idx,
                                   unsigned int palIdx,
                                   bool rgba) = 0;
    virtual unsigned char *getLid(unsigned int idx,
                                  unsigned int palIdx,
                                  bool rgba) = 0;
    virtual unsigned char *getAux(unsigned int idx,
                                  unsigned int palIdx,
                                  bool rgba) = 0;

    virtual std::unique_ptr<unsigned char[]> getSpriteBitmap(size_t id,
                                                             int remap,
                                                             Uint32 delta) = 0;

    std::vector<LoadedAnim> animations;
    std::vector<SpriteInfo *> spriteInfos;
    std::vector<ObjectInfo> objectInfos;
    std::vector<CarInfo> carInfos;

    bool getDeltaHandling();
    void setDeltaHandling(bool delta_as_set);

    bool isBlockingSide(uint8_t id);
    void setupBlocking(const std::string &file);

protected:
    void loadTileTextures();
    void loadAnim();

    void loadObjectInfo_shared(PHYSFS_uint64 offset);
    void loadSpriteNumbers_shared(PHYSFS_uint64 offset);
    void loadCarInfo_shared(PHYSFS_uint64 offset);
    void loadSpriteInfo_shared(PHYSFS_uint64 offset);

    void handleDeltas(const SpriteInfo &spriteinfo,
                      unsigned char *buffer,
                      Uint32 delta);
    void applyDelta(const SpriteInfo &spriteInfo,
                    unsigned char *buffer,
                    Uint32 offset,
                    const DeltaInfo &deltaInfo,
                    bool mirror = false);

    PHYSFS_file *fd;
    unsigned char *rawTiles;
    unsigned char *rawSprites;

    PHYSFS_uint32 sideSize;
    PHYSFS_uint32 lidSize;
    PHYSFS_uint32 auxSize;
    PHYSFS_uint32 animSize;
    PHYSFS_uint32 objectInfoSize;
    PHYSFS_uint32 carInfoSize;
    PHYSFS_uint32 spriteInfoSize;
    PHYSFS_uint32 spriteGraphicsSize;
    PHYSFS_uint32 spriteNumberSize;

    PHYSFS_uint32 auxBlockTrailSize;

    /*
    int loadSide();
    int loadLid();
    int loadAux();
    int loadAnim();
    int loadObject();
    int loadCar();
    int loadSpriteInfo();
    int loadSpriteGraphics();
    int loadSpriteNumbers();*/

    PHYSFS_uint8 _topHeaderSize;

    unsigned char tileTmp[4096];
    unsigned char tileTmpRGB[4096 * 3];
    unsigned char tileTmpRGBA[4096 * 4];

    bool delta_is_a_set;

    Util::Set sideTexBlockMove;

    unsigned int firstValidPedRemap;
    unsigned int lastValidPedRemap;
};

} // namespace OpenGTA
#endif
