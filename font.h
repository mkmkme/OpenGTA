#pragma once

#include "graphics-8bit.h"

#include <cstddef>
#include <cstdint>
#include <map>
#include <physfs.h>
#include <string>
#include <vector>

namespace OpenGTA {

class Font {
public:
    class Character {
    public:
        Character(PHYSFS_file *, uint8_t);
        ~Character();
        uint8_t width{};
        uint8_t *rawData;
    };
    explicit Font(const std::string &file);
    ~Font();
    [[nodiscard]] uint8_t getCharHeight() const noexcept { return charHeight; }
    size_t getIdByChar(char c);
    uint8_t getMoveWidth(char c);

    void addMapping(char c, size_t num);

    friend void dumpAs(Font &font, const char *filename, size_t id);
    unsigned char *getCharacterBitmap(size_t num,
                                      unsigned int *width,
                                      unsigned int *height);

private:
    void loadMapping(const std::string &name);
    void readHeader(PHYSFS_file *);
    uint8_t charHeight{};
    uint8_t numChars{};
    std::vector<Character *> chars;
    std::map<char, size_t> mapping;
    Graphics8Bit::RGBPalette palette;
    unsigned char *workBuffer;
};

} // namespace OpenGTA
