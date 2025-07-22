#pragma once

#include <core/graphics-base.h>

namespace OpenGTA {

class Graphics24Bit final : public GraphicsBase {
public:
    explicit Graphics24Bit(const std::string &style);
    ~Graphics24Bit() override;

    std::span<const uint8_t> getSide(uint8_t idx, unsigned int palIdx, bool rgba) override;
    std::span<const uint8_t> getLid(uint8_t idx, unsigned int palIdx, bool rgba) override;
    std::span<const uint8_t> getAux(uint8_t idx, unsigned int palIdx, bool rgba) override;

    std::vector<uint8_t> getSpriteBitmap(size_t id, int remap, uint32_t delta) override;

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

    void applyClut(unsigned char *src, unsigned char *dst, size_t len, uint16_t clutIdx, bool rgba);

private:
    uint32_t clutSize {};
    uint32_t pagedClutSize {};
    uint32_t tileclutSize {};
    uint32_t spriteclutSize {};
    uint32_t newcarclutSize {};
    uint32_t fontclutSize {};
    uint32_t paletteIndexSize {};

    unsigned char *rawClut;
    uint16_t *palIndex;
};

} // namespace OpenGTA
