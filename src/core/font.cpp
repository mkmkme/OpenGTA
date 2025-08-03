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
#include <algorithm>
#include <set>

#include <core/font.h>

#include <util/file-manager.h>
#include <util/file_helper.h>
#include <util/log.h>
#include <util/string_helpers.h>

namespace OpenGTA {
Font::Font(const std::string &file)
{
    Util::PhysFSFile pf { file };
    const auto numChars = readHeader(pf);
    chars.reserve(numChars);
    int ww = 0;
    int lw = 0;
    for (uint8_t i = 0; i < numChars; i++) {
        const auto &ch = chars.emplace_back(pf, charHeight);
        ww += ch.width;
        lw = std::max<int>(ch.width, lw);
    }
    INFO("total width {} largest width {}", ww, lw);
    palette.loadFromFile(pf);
    // size_t ih = charHeight;
    // while (ww > 1024) {
    // ih *= 2;
    // ww /= 2;
    // }
    loadMapping(file);
}

uint8_t Font::readHeader(Util::PhysFSFile &pf)
{
    uint8_t numChars;
    pf.read(numChars);
    pf.read(charHeight);
    INFO("Font contains {} characters of height {}", numChars, charHeight);
    return numChars;
}
void Font::addMapping(unsigned char c, size_t num)
{
    mapping[c] = num;
}
size_t Font::getIdByChar(const char c) const noexcept
{
    return mapping[c];
}
uint8_t Font::getMoveWidth(const char c)
{
    return chars[getIdByChar(c)].width;
}

std::vector<uint8_t> Font::getCharacterBitmap(size_t num, unsigned int *width, unsigned int *height)
{
    std::vector<uint8_t> buffer;
    const unsigned int len = chars[num].width * charHeight;
    buffer.resize(len * 4);
    palette.apply(len, chars[num].rawData.data(), buffer.data(), true);
    if (width != nullptr)
        *width = chars[num].width;
    if (height != nullptr)
        *height = charHeight;
    return buffer;
}

Font::Character::Character(Util::PhysFSFile &pf, uint8_t height) noexcept
{
    pf.read(width);
    size_t c = size_t(width) * size_t(height);
    // std::cout <<"width " << int(width) << " going to read " << c << " bytes" << std::endl;
    rawData = std::vector<uint8_t>(c);
    pf.read(rawData.data(), c);
}

void Font::loadMapping(const std::string &name)
{
    std::string name2 { Util::string_lower(name) };
    if (name2 == "big1.fon") {
        INFO("found mapping: big1.fon - {}", name);
        addMapping('!', 0);
        addMapping('-', 12);
        for (unsigned char j = 65; j < 91; j++) {
            addMapping(j, j - 33);
        }
        for (unsigned char j = 192; j < 195; j++) {
            addMapping(j, j - 97);
        }
        addMapping(196, 98);
        addMapping(198, 99);
        addMapping(199, 100);
        for (unsigned char j = 200; j < 208; j++)
            addMapping(j, j - 99);
        for (unsigned char j = 210; j < 213; j++)
            addMapping(j, j - 101);
        addMapping(214, 112);
        for (unsigned char j = 217; j < 221; j++)
            addMapping(j, j - 104);
        addMapping(223, 117);
    } else if (std::set<std::string> { "pager1.fon", "pager2.fon" }.contains(name2)) {
        addMapping('!', 0);
        addMapping('"', 1);
        addMapping('$', 3);
        addMapping('\'', 6);
        addMapping('(', 7);
        addMapping(')', 8);
        addMapping(',', 11);
        addMapping('.', 13);
        for (unsigned char j = 48; j < 58; j++) {
            addMapping(j, j - 33);
        }
        addMapping(':', 25);
        addMapping(';', 26);
        addMapping('<', 27);
        addMapping('>', 29);
        addMapping('?', 30);
        addMapping('_', 62);
        for (unsigned char j = 65; j < 91; j++) {
            addMapping(j, j - 33);
        }
        for (unsigned char j = 192; j < 195; j++) {
            addMapping(j, j - 97);
        }
        addMapping(196, 98);
        addMapping(198, 99);
        addMapping(199, 100);
        for (unsigned char j = 200; j < 208; j++)
            addMapping(j, j - 99);
        for (unsigned char j = 210; j < 213; j++)
            addMapping(j, j - 101);
        addMapping(214, 112);
        for (unsigned char j = 217; j < 221; j++)
            addMapping(j, j - 104);
        addMapping(223, 117);
    } else if (name2 == "street1.fon") {
        INFO("found mapping: streen1.fon - {}", name);
        for (unsigned char j = 65; j < 91; j++) {
            addMapping(j, j - 33);
        }
        for (unsigned char j = 48; j < 58; j++) {
            addMapping(j, j - 33);
        }
        for (unsigned char j = 97; j < 123; j++) {
            addMapping(j, j - 33);
        }
        WARN("incomplete mapping");
    } else if (name2 == "m_mmiss.fon") {
        addMapping('!', 0);
        addMapping('"', 1);
        addMapping('#', 2);
        addMapping('$', 3);
        addMapping('\'', 6);
        addMapping('(', 7);
        addMapping(')', 8);
        addMapping('+', 10);
        addMapping(',', 11);
        addMapping('.', 13);
        addMapping('/', 14);
        addMapping(':', 25);
        addMapping(';', 26);
        addMapping('<', 27);
        addMapping('=', 28);
        addMapping('>', 29);
        addMapping('?', 30);
        addMapping('\\', 59);
        addMapping('[', 58);
        addMapping(']', 60);
        addMapping('|', 91);
        addMapping('~', 93);
        for (unsigned char j = 65; j < 91; j++) {
            addMapping(j, j - 33);
        }
        for (unsigned char j = 97; j < 123; j++) {
            addMapping(j, j - 33);
        }
        for (unsigned char j = 48; j < 58; j++) {
            addMapping(j, j - 33);
        }
        for (unsigned char j = 192; j < 195; j++) {
            addMapping(j, j - 97);
        }
        // incomplete
    } else if (name2 == "f_mtext.fon") {
        addMapping('!', 0);
        addMapping('"', 1);
        addMapping('#', 2);
        addMapping('$', 3);
        addMapping('%', 4);
        addMapping('\'', 6);
        addMapping('(', 7);
        addMapping(')', 8);
        addMapping(169, 9); // copyright
        addMapping(',', 11);
        addMapping('-', 12);
        addMapping('.', 13);
        addMapping('/', 14);
        addMapping(':', 25);
        addMapping(';', 26);
        addMapping('<', 27);
        addMapping('=', 28);
        addMapping('>', 29);
        addMapping('?', 30);
        for (unsigned char j = 48; j < 58; j++) {
            addMapping(j, j - 33);
        }
        addMapping('\\', 59);
        for (unsigned char j = 65; j < 91; j++) {
            addMapping(j, j - 33);
        }
        for (unsigned char j = 97; j < 123; j++) {
            addMapping(j, j - 33);
        }
        // incomplete

    } else if (name2 == "f_mhead.fon") {
        addMapping('!', 0);
        addMapping('"', 1);
        addMapping('#', 2);
        addMapping('$', 3);
        addMapping('\'', 6);
        addMapping('(', 7);
        addMapping(')', 8);
        addMapping(',', 11);
        addMapping('.', 13);
        addMapping('/', 14);
        addMapping(':', 25);
        addMapping(';', 26);
        addMapping('<', 27);
        addMapping('>', 29);
        addMapping('?', 30);
        addMapping('\\', 59);
        for (unsigned char j = 65; j < 91; j++) {
            addMapping(j, j - 33);
        }
        for (unsigned char j = 97; j < 123; j++) {
            addMapping(j, j - 33);
        }
        for (unsigned char j = 48; j < 58; j++) {
            addMapping(j, j - 33);
        }
        for (unsigned char j = 192; j < 195; j++) {
            addMapping(j, j - 97);
        }
        addMapping(196, 98);
        addMapping(198, 99);
        addMapping(199, 100);
        for (unsigned char j = 200; j < 208; j++)
            addMapping(j, j - 99);
        for (unsigned char j = 210; j < 213; j++)
            addMapping(j, j - 101);
        addMapping(214, 112);
        for (unsigned char j = 217; j < 221; j++)
            addMapping(j, j - 104);
        addMapping(223, 117);
        for (unsigned char j = 224; j < 227; j++)
            addMapping(j, j - 106);
        addMapping(228, 121);
        for (unsigned char j = 230; j < 240; j++)
            addMapping(j, j - 108);
        for (unsigned char j = 242; j < 245; j++)
            addMapping(j, j - 110);
        addMapping(246, 135);
        for (unsigned char j = 249; j < 253; j++)
            addMapping(j, j - 113);

    } else if (std::set<std::string> { "sub1.fon", "sub2.fon" }.contains(name2)) {
        addMapping('!', 0);
        addMapping('"', 1);
        addMapping('$', 3);
        addMapping('\'', 6); // ´
        addMapping('(', 7);
        addMapping(')', 8);
        addMapping(',', 11);
        addMapping('-', 12); // not in street1/2
        addMapping('.', 13);
        addMapping('/', 14);
        addMapping(':', 25);
        addMapping(';', 26);
        addMapping('<', 27);
        addMapping('>', 29);
        addMapping('?', 30);

        for (unsigned char j = 65; j < 91; j++) {
            addMapping(j, j - 33);
        }
        for (unsigned char j = 97; j < 123; j++) {
            addMapping(j, j - 33);
        }
        for (unsigned char j = 48; j < 58; j++) {
            addMapping(j, j - 33);
        }
        for (unsigned char j = 192; j < 195; j++) {
            addMapping(j, j - 97);
        }
        addMapping(196, 98);
        addMapping(198, 99);
        addMapping(199, 100);
        for (unsigned char j = 200; j < 208; j++)
            addMapping(j, j - 99);
        for (unsigned char j = 210; j < 213; j++)
            addMapping(j, j - 101);
        addMapping(214, 112);
        for (unsigned char j = 217; j < 221; j++)
            addMapping(j, j - 104);
        addMapping(223, 117);
        for (unsigned char j = 224; j < 227; j++)
            addMapping(j, j - 106);
        addMapping(228, 121);
        for (unsigned char j = 230; j < 240; j++)
            addMapping(j, j - 108);
        for (unsigned char j = 242; j < 245; j++)
            addMapping(j, j - 110);
        addMapping(246, 135);
        for (unsigned char j = 249; j < 253; j++)
            addMapping(j, j - 113);
    } else if (std::set<std::string> { "score1.fon", "score2.fon", "score8.fon" }.contains(name2)) {
        for (unsigned char j = 48; j < 58; j++) {
            addMapping(j, j - 48);
        }
    } else {
        ERROR("mapping for font {} is not known", name);
    }
}
} // namespace OpenGTA
