#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include <core/graphics-8bit.h>
#include <core/numeric-types.h>

namespace Util {
class PhysFSFile;
}

namespace OpenGTA {

class Font {
public:
    class Character {
    public:
        Character(Util::PhysFSFile &, uint8_t);
        uint8_t width {};
        std::vector<UInt8> rawData;
    };
    explicit Font(const std::string &file);
    [[nodiscard]] uint8_t getCharHeight() const noexcept { return charHeight; }
    [[nodiscard]] size_t getIdByChar(char c) const noexcept;
    uint8_t getMoveWidth(char c);

    void addMapping(unsigned char c, size_t num);

    std::vector<UInt8> getCharacterBitmap(size_t num, unsigned int *width, unsigned int *height);

private:
    // Returns the number of characters in the font
    void loadMapping(const std::string &name);
    UInt8 readHeader(Util::PhysFSFile &pf);
    UInt8 charHeight {};
    std::vector<Character> chars;
    std::map<char, size_t> mapping;
    Graphics8Bit::RGBPalette palette;
};

} // namespace OpenGTA
