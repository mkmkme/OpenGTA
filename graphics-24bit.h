#pragma once

#include "graphics-base.h"

namespace OpenGTA {

class Graphics24Bit : public GraphicsBase {
public:
    Graphics24Bit(const std::string &style);
    ~Graphics24Bit();

    unsigned char *getSide(unsigned int idx, unsigned int palIdx, bool rgba);
    unsigned char *getLid(unsigned int idx, unsigned int palIdx, bool rgba);
    unsigned char *getAux(unsigned int idx, unsigned int palIdx, bool rgba);

    std::unique_ptr<unsigned char[]> getSpriteBitmap(size_t id,
                                                     int remap,
                                                     uint32_t delta) override;

    void dumpClut(const char *fname);

protected:
    void loadHeader();
    void loadClut();
    void loadPalIndex();
    void loadObjectInfo();
    void loadCarInfo();
    void loadSpriteInfo();
    void loadSpriteGraphics();
    void loadSpriteNumbers();

    void applyClut(unsigned char *src,
                   unsigned char *dst,
                   const size_t &len,
                   const PHYSFS_uint16 &clutIdx,
                   bool rgba);

private:
    PHYSFS_uint32 clutSize;
    PHYSFS_uint32 pagedClutSize;
    PHYSFS_uint32 tileclutSize;
    PHYSFS_uint32 spriteclutSize;
    PHYSFS_uint32 newcarclutSize;
    PHYSFS_uint32 fontclutSize;
    PHYSFS_uint32 paletteIndexSize;

    unsigned char *rawClut;
    PHYSFS_uint16 *palIndex;
};

} // namespace OpenGTA
