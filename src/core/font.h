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
    size_t getIdByChar(char c);
    uint8_t getMoveWidth(char c);

    void addMapping(unsigned char c, size_t num);

    friend void dumpAs(Font &font, const char *filename, size_t id);
    unsigned char *getCharacterBitmap(size_t num, unsigned int *width, unsigned int *height);

private:
    void loadMapping(const std::string &name);
    void readHeader(Util::PhysFSFile &pf);
    uint8_t charHeight {};
    uint8_t numChars {};
    std::vector<Character> chars;
    std::map<char, size_t> mapping;
    Graphics8Bit::RGBPalette palette;
    std::vector<UInt8> workBuffer;
};

} // namespace OpenGTA
