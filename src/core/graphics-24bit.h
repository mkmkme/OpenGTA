#pragma once

#include <core/graphics-base.h>

namespace OpenGTA {

class Graphics24Bit final : public GraphicsBase {
public:
    explicit Graphics24Bit(const std::string &style);
    ~Graphics24Bit() override;

    std::span<const UInt8> getSide(UInt8 idx, unsigned int palIdx, bool rgba) override;
    std::span<const UInt8> getLid(UInt8 idx, unsigned int palIdx, bool rgba) override;
    std::span<const UInt8> getAux(UInt8 idx, unsigned int palIdx, bool rgba) override;

    std::vector<UInt8> getSpriteBitmap(size_t id, int remap, UInt32 delta) override;

    friend void dumpClut(const Graphics24Bit &g24, const char *fname);

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
        size_t len,
        UInt16 clutIdx,
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
