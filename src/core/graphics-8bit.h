#pragma once

#include <memory>

#include <core/graphics-base.h>

namespace Util {
class PhysFSFile;
}

namespace OpenGTA {

class CityView;

/** Loader for STYLE*.GRY files.
 *
 * Implements loading the 8-bit graphic files.
 */
class Graphics8Bit : public GraphicsBase {
    /** allow renderer direct access to members */
    friend class CityView;

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
        unsigned char data[256 * 3] {};

    public:
        /** Empty constructor.
         * You HAVE to call loadFromFile() function when using this
         * constructor!.
         */
        RGBPalette();
        /** Formerly private member, now exposed for Font class; take care.
         * @param styileFile Util::PhysFSFile handle.
         */
        int loadFromFile(Util::PhysFSFile &styleFile);
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

    std::span<const UInt8> getSide(UInt8 idx, unsigned int palIdx, bool rgba) override;
    std::span<const UInt8> getLid(UInt8 idx, unsigned int palIdx, bool rgba) override;
    std::span<const UInt8> getAux(UInt8 idx, unsigned int palIdx, bool rgba) override;

    std::vector<UInt8> getSpriteBitmap(size_t id, int remap, uint32_t delta) override;

    void dump();

private:
    UInt32 paletteSize {};
    UInt32 remapSize {};
    UInt32 remapIndexSize {};

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
    UInt8 remapTables[256][256] {};
    UInt8 remapIndex[256][4] {};
};

} // namespace OpenGTA
