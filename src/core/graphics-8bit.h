#pragma once

#include <memory>

#include <core/graphics-base.h>

namespace Util {
class PhysFSFile;
}

namespace OpenGTA {

/** Loader for STYLE*.GRY files.
 *
 * Implements loading the 8-bit graphic files.
 */
class Graphics8Bit : public GraphicsBase {
public:
    /** Constructor for graphics loader.
     * @param style a valid filename (maybe uppercase depending on your files)
     */
    explicit Graphics8Bit(const std::string &style);

    /** Helper to apply palettes to various raw bitmaps.
     * @see Graphics8Bit
     * @see Font
     */
    class RGBPalette {
    private:
        std::array<uint8_t, 256 * 3> data {};

    public:
        /** Empty constructor.
         * You HAVE to call loadFromFile() function when using this
         * constructor!.
         */
        RGBPalette() = default;
        /** Formerly private member, now exposed for Font class; take care.
         * @param styileFile Util::PhysFSFile handle.
         */
        void loadFromFile(Util::PhysFSFile &styleFile);
        /** Constructor from PHYFS_file.
         * @param styleFile Util::PhysFSFile handle
         */
        explicit RGBPalette(Util::PhysFSFile &styleFile);
        /** Constructor from filename.
         * @param filename a palette file name
         */
        explicit RGBPalette(const std::string &palette);
        /** Transforms an input buffer using the palette stored in this
         * instance.
         * @param len length of the src buffer (in byte)
         * @param src pointer to src buffer
         * @param dst pointer to dst buffer (must exist and be large enough)
         * @param rgba use 'true' to create a RGBA image, or 'false' (default)
         * for RGB
         */
        void apply(unsigned int len, const unsigned char *src, unsigned char *dst, bool rgba = false);
    };

    std::span<const uint8_t> getSide(uint8_t idx, unsigned int palIdx, bool rgba) override;
    std::span<const uint8_t> getLid(uint8_t idx, unsigned int palIdx, bool rgba) override;
    std::span<const uint8_t> getAux(uint8_t idx, unsigned int palIdx, bool rgba) override;

    std::vector<uint8_t> getSpriteBitmap(size_t id, int remap, uint32_t delta) override;

    void dump();

private:
    uint32_t paletteSize {};
    uint32_t remapSize {};
    uint32_t remapIndexSize {};

protected:
    void loadHeader();
    void loadPalette();
    void loadRemapTables();
    void loadRemapIndex();
    void loadObjectInfo();
    void loadCarInfo();
    void loadSpriteInfo();
    void loadSpriteGraphics();
    void loadSpriteNumbers();
    void applyRemap(unsigned int len, unsigned int which, unsigned char *buffer);
    std::unique_ptr<RGBPalette> masterRGB_;
    uint8_t remapTables[256][256] {};
    uint8_t remapIndex[256][4] {};
};

} // namespace OpenGTA
