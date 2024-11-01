#pragma once

#include <core/graphics-base.h>

namespace OpenGTA {

class Graphics24Bit : public GraphicsBase {
public:
    Graphics24Bit(const std::string &style);
    ~Graphics24Bit();

    unsigned char *getSide(unsigned int idx, unsigned int palIdx, bool rgba);
    unsigned char *getLid(unsigned int idx, unsigned int palIdx, bool rgba);
    unsigned char *getAux(unsigned int idx, unsigned int palIdx, bool rgba);

    std::unique_ptr<unsigned char[]> getSpriteBitmap(size_t id, int remap, uint32_t delta) override;

    friend void dumpClut(Graphics24Bit &g24, const char *fname);

protected:
    void loadHeader();
    void loadClut();
    void loadPalIndex();
    void loadObjectInfo();
    void loadCarInfo();
    void loadSpriteInfo();
    void loadSpriteGraphics();
    void loadSpriteNumbers();

    void applyClut(
        unsigned char *src,
        unsigned char *dst,
        const size_t &len,
        const UInt16 &clutIdx,
        bool rgba
    );

private:
    UInt32 clutSize {};
    UInt32 pagedClutSize {};
    UInt32 tileclutSize {};
    UInt32 spriteclutSize {};
    UInt32 newcarclutSize {};
    UInt32 fontclutSize {};
    UInt32 paletteIndexSize {};

    unsigned char *rawClut;
    UInt16 *palIndex;
};

} // namespace OpenGTA
