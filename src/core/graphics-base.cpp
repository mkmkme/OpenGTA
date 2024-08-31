#include <algorithm>
#include <cassert>

#include <core/graphics-base.h>
#include <core/loaded-anim.h>
#include <core/sprite-info.h>

#include <util/errors.h>
#include <util/log.h>

using namespace OpenGTA;
using namespace Util;

PHYSFS_uint16 GraphicsBase::SpriteNumbers::countByType(const SpriteTypes &t) const
{
    switch (t) {
#define CASE_COUNT(_enum, _value) \
    case _enum:                   \
        return _value
#define CASE_COUNT_TRIVIAL(_enum) CASE_COUNT(_enum, GTA_SPRITE_##_enum);

        CASE_COUNT_TRIVIAL(ARROW);
        CASE_COUNT(DIGIT, GTA_SPRITE_DIGITS);
        CASE_COUNT_TRIVIAL(BOAT);
        CASE_COUNT_TRIVIAL(BOX);
        CASE_COUNT_TRIVIAL(BUS);
        CASE_COUNT_TRIVIAL(CAR);
        CASE_COUNT_TRIVIAL(OBJECT);
        CASE_COUNT_TRIVIAL(PED);
        CASE_COUNT_TRIVIAL(SPEEDO);
        CASE_COUNT_TRIVIAL(TANK);
        CASE_COUNT(TRAFFIC_LIGHT, GTA_SPRITE_TRAFFIC_LIGHTS);
        CASE_COUNT_TRIVIAL(TRAIN);
        CASE_COUNT(TRDOOR, GTA_SPRITE_TRDOORS);
        CASE_COUNT_TRIVIAL(BIKE);
        CASE_COUNT_TRIVIAL(TRAM);
        CASE_COUNT_TRIVIAL(WBUS);
        CASE_COUNT_TRIVIAL(WCAR);
        CASE_COUNT_TRIVIAL(EX);
        CASE_COUNT_TRIVIAL(TUMCAR);
        CASE_COUNT_TRIVIAL(TUMTRUCK);
        CASE_COUNT_TRIVIAL(FERRY);

#undef CASE_COUNT
#undef CASE_COUNT_TRIVIAL
    }
    ERROR("UPS: {}", static_cast<int>(t));
    assert(0);
}

GraphicsBase::GraphicsBase()
    : sideTexBlockMove(256)
{
    rawTiles = nullptr;
    rawSprites = nullptr;
    delta_is_a_set = false;
    for (int i = 0; i < 256; ++i)
        sideTexBlockMove.set_item(i, true);
}

bool GraphicsBase::isBlockingSide(uint8_t id)
{
    return sideTexBlockMove.get_item(id);
}

void GraphicsBase::setupBlocking()
{
    // style001
    sideTexBlockMove.set_item(10, false);
    sideTexBlockMove.set_item(20, false);
    sideTexBlockMove.set_item(97, false);
    sideTexBlockMove.set_item(109, false);
    sideTexBlockMove.set_item(110, false);
    sideTexBlockMove.set_item(115, false);
    sideTexBlockMove.set_item(116, false);
    sideTexBlockMove.set_item(155, false);
    sideTexBlockMove.set_item(156, false);
    sideTexBlockMove.set_item(157, false);
    sideTexBlockMove.set_item(158, false);
}

bool GraphicsBase::getDeltaHandling()
{
    return delta_is_a_set;
}

void GraphicsBase::setDeltaHandling(bool delta_as_set)
{
    delta_is_a_set = delta_as_set;
}

GraphicsBase::~GraphicsBase()
{
    if (fd)
        PHYSFS_close(fd);
    for (auto &spriteInfo : spriteInfos)
        delete spriteInfo;
    spriteInfos.clear();
    delete[] rawTiles;
    delete[] rawSprites;
}

bool GraphicsBase::isAnimatedBlock(uint8_t area_code, uint8_t id)
{
    return std::ranges::any_of(animations, [&](const auto &anim) {
        return anim.which == area_code && anim.block == id;
    });
}

CarInfo &GraphicsBase::findCarByModel(PHYSFS_uint8 model)
{
    for (auto &car : carInfos) {
        if (car.model == model)
            return car;
    }
    throw Util::UnknownKey("Searching for car model " + std::to_string(int(model)) + " failed");
}

unsigned int GraphicsBase::getRandomPedRemapNumber() const
{
    return int(rand() * (1.0f / (1.0f + RAND_MAX)) * (lastValidPedRemap - firstValidPedRemap) + firstValidPedRemap);
}

unsigned int GraphicsBase::getPedRemapNumberType(unsigned int _type)
{
    ERROR("not implemented");
    return _type;
}

uint8_t GraphicsBase::getFormat()
{
    if (_topHeaderSize == 52)
        return 0;
    else if (_topHeaderSize == 64)
        return 1;
    throw Util::InvalidFormat("graphics-base header size");
}

PHYSFS_uint16 GraphicsBase::SpriteNumbers::reIndex(const PHYSFS_uint16 &id, const SpriteTypes &t) const
{
    PHYSFS_uint16 ret = id;
    switch (t) {
#define CASE_ACCUMULATE(_enum, _val) \
    case _enum:                      \
        ret += _val;                 \
        [[fallthrough]]

        CASE_ACCUMULATE(FERRY, GTA_SPRITE_TUMTRUCK);
        CASE_ACCUMULATE(TUMTRUCK, GTA_SPRITE_TUMCAR);
        CASE_ACCUMULATE(TUMCAR, GTA_SPRITE_EX);
        CASE_ACCUMULATE(EX, GTA_SPRITE_WCAR);
        CASE_ACCUMULATE(WCAR, GTA_SPRITE_WBUS);
        CASE_ACCUMULATE(WBUS, GTA_SPRITE_TRAM);
        CASE_ACCUMULATE(TRAM, GTA_SPRITE_BIKE);
        CASE_ACCUMULATE(BIKE, GTA_SPRITE_TRDOORS);
        CASE_ACCUMULATE(TRDOOR, GTA_SPRITE_TRAIN);
        CASE_ACCUMULATE(TRAIN, GTA_SPRITE_TRAFFIC_LIGHTS);
        CASE_ACCUMULATE(TRAFFIC_LIGHT, GTA_SPRITE_TANK);
        CASE_ACCUMULATE(TANK, GTA_SPRITE_SPEEDO);
        CASE_ACCUMULATE(SPEEDO, GTA_SPRITE_PED);
        CASE_ACCUMULATE(PED, GTA_SPRITE_OBJECT);
        CASE_ACCUMULATE(OBJECT, GTA_SPRITE_CAR);
        CASE_ACCUMULATE(CAR, GTA_SPRITE_BUS);
        CASE_ACCUMULATE(BUS, GTA_SPRITE_BOX);
        CASE_ACCUMULATE(BOX, GTA_SPRITE_BOAT);
        CASE_ACCUMULATE(BOAT, GTA_SPRITE_DIGITS);
        CASE_ACCUMULATE(DIGIT, GTA_SPRITE_ARROW);
        case ARROW:
            break;
#undef CASE_ACCUMULATE
    }
    return ret;
}

void GraphicsBase::loadAnim()
{
    PHYSFS_uint64 st = static_cast<PHYSFS_uint64>(_topHeaderSize) + sideSize + lidSize + auxSize + auxBlockTrailSize;
    PHYSFS_seek(fd, st);
    PHYSFS_uint8 numAnim;
    PHYSFS_readBytes(fd, static_cast<void *>(&numAnim), 1);
    for (int i = 0; i < numAnim; i++)
        animations.emplace_back(fd);
}

void GraphicsBase::loadObjectInfo_shared(PHYSFS_uint64 offset)
{
    PHYSFS_seek(fd, offset);
    assert(objectInfoSize % 20 == 0);
    int c = objectInfoSize / 20;

    for (int i = 0; i < c; i++)
        objectInfos.emplace_back(fd);
}

void GraphicsBase::loadCarInfo_shared(PHYSFS_uint64 offset)
{
    PHYSFS_seek(fd, offset);

    PHYSFS_uint32 bytes_read = 0;
    while (bytes_read < carInfoSize) {
        CarInfo car { fd };
        bytes_read += car.bytes_read();
        carInfos.emplace_back(std::move(car));
    }
    assert(bytes_read == carInfoSize);
}

void GraphicsBase::loadSpriteNumbers_shared(PHYSFS_uint64 offset)
{

    PHYSFS_seek(fd, offset);

    PHYSFS_readULE16(fd, &spriteNumbers.GTA_SPRITE_ARROW);
    PHYSFS_readULE16(fd, &spriteNumbers.GTA_SPRITE_DIGITS);
    PHYSFS_readULE16(fd, &spriteNumbers.GTA_SPRITE_BOAT);
    PHYSFS_readULE16(fd, &spriteNumbers.GTA_SPRITE_BOX);
    PHYSFS_readULE16(fd, &spriteNumbers.GTA_SPRITE_BUS);
    PHYSFS_readULE16(fd, &spriteNumbers.GTA_SPRITE_CAR);
    PHYSFS_readULE16(fd, &spriteNumbers.GTA_SPRITE_OBJECT);
    PHYSFS_readULE16(fd, &spriteNumbers.GTA_SPRITE_PED);
    PHYSFS_readULE16(fd, &spriteNumbers.GTA_SPRITE_SPEEDO);
    PHYSFS_readULE16(fd, &spriteNumbers.GTA_SPRITE_TANK);
    PHYSFS_readULE16(fd, &spriteNumbers.GTA_SPRITE_TRAFFIC_LIGHTS);
    PHYSFS_readULE16(fd, &spriteNumbers.GTA_SPRITE_TRAIN);
    PHYSFS_readULE16(fd, &spriteNumbers.GTA_SPRITE_TRDOORS);
    PHYSFS_readULE16(fd, &spriteNumbers.GTA_SPRITE_BIKE);
    PHYSFS_readULE16(fd, &spriteNumbers.GTA_SPRITE_TRAM);
    PHYSFS_readULE16(fd, &spriteNumbers.GTA_SPRITE_WBUS);
    PHYSFS_readULE16(fd, &spriteNumbers.GTA_SPRITE_WCAR);
    PHYSFS_readULE16(fd, &spriteNumbers.GTA_SPRITE_EX);
    PHYSFS_readULE16(fd, &spriteNumbers.GTA_SPRITE_TUMCAR);
    PHYSFS_readULE16(fd, &spriteNumbers.GTA_SPRITE_TUMTRUCK);
    PHYSFS_readULE16(fd, &spriteNumbers.GTA_SPRITE_FERRY);
}

void GraphicsBase::loadTileTextures()
{
    PHYSFS_seek(fd, static_cast<PHYSFS_uint64>(_topHeaderSize));

    PHYSFS_uint64 ts = sideSize + lidSize + auxSize;
    rawTiles = new unsigned char[ts];
    int r = PHYSFS_readBytes(fd, static_cast<void *>(rawTiles), ts);
    if (PHYSFS_uint64(r) == ts)
        return;
    else if (r == -1) {
        ERROR("Could not read texture raw data");
        return;
    } else
        ERROR("This message should never be displayed!");
}

void GraphicsBase::handleDeltas(const SpriteInfo &info, unsigned char *buffer, uint32_t delta)
{
    const unsigned int b_offset = 256 * info.yoffset + info.xoffset;
    if (delta_is_a_set) {
        Util::Set delta_set(32, (unsigned char *) &delta);
        for (int i = 0; i < 20; ++i) {
            if (delta_set.get_item(i)) {
                assert(i < info.deltaCount);
                const DeltaInfo &di = info.delta[i];
                applyDelta(info, buffer, b_offset, di);
            }
        }
        for (int i = 20; i < 24; i++) {
            if (delta_set.get_item(i)) {
                const DeltaInfo &di = info.delta[i - 20 + 6];
                applyDelta(info, buffer, b_offset, di, true);
            }
        }
        for (int i = 24; i < 28; i++) {
            if (delta_set.get_item(i)) {
                const DeltaInfo &di = info.delta[i - 24 + 11];
                applyDelta(info, buffer, b_offset, di, true);
            }
        }
        // assert(0);
    } else {
        // delta is only an index; one to big
        assert(delta <= info.deltaCount);
        const DeltaInfo &di = info.delta[delta - 1];
        applyDelta(info, buffer, b_offset, di);
    }
}

void GraphicsBase::applyDelta(
    const SpriteInfo &spriteInfo,
    unsigned char *buffer,
    uint32_t page_offset,
    const DeltaInfo &deltaInfo,
    bool mirror
)
{
    unsigned char *b = buffer + page_offset;
    unsigned char *delta = deltaInfo.ptr;
    PHYSFS_sint32 length_to_go = deltaInfo.size;

    if (mirror) {
        PHYSFS_uint32 doff = 0;
        while (length_to_go > 0) {
            PHYSFS_uint16 *offset = (PHYSFS_uint16 *) delta;
            doff += *offset;
            delta += 2;
            unsigned char this_length = *delta;
            ++delta;
            PHYSFS_uint32 noff = page_offset + doff;
            PHYSFS_uint32 _y = noff / 256 * 256;
            PHYSFS_uint32 _x = doff % 256;
            for (int i = 0; i < this_length; i++)
                *(buffer + _y + spriteInfo.xoffset + spriteInfo.w - _x - i - 1) = *(delta + i);
            length_to_go -= (this_length + 3);
            doff += this_length;
            delta += this_length;
        }
        return;
    }

    while (length_to_go > 0) {
        PHYSFS_uint16 *offset = (PHYSFS_uint16 *) delta;
        b += *offset;
        delta += 2;
        unsigned char this_length = *delta;
        ++delta;
        memcpy(b, delta, this_length);
        b += this_length;
        delta += this_length;
        length_to_go -= (this_length + 3);
    }
}

void GraphicsBase::prepareSideTexture(unsigned int idx, unsigned char *dst)
{
    assert(dst != NULL);
    ++idx;
    assert(rawTiles);
    unsigned char *rt = rawTiles + (idx / 4) * 4096 * 4 + (idx % 4) * 64;
    for (int i = 0; i < 64; i++) {
        memcpy(dst, rt, 64);
        dst += 64;
        rt += 64 * 4;
    }
}

void GraphicsBase::prepareLidTexture(unsigned int idx, unsigned char *dst)
{
    assert(dst != NULL);
    unsigned char *rt = rawTiles;
    assert(rawTiles);
    idx += sideSize / 4096 + 1; // FIXME: assumes partition == block end
    rt += (idx / 4) * 4096 * 4 + (idx % 4) * 64;
    for (int i = 0; i < 64; i++) {
        memcpy(dst, rt, 64);
        dst += 64;
        rt += 64 * 4;
    }
}

void GraphicsBase::prepareAuxTexture(unsigned int idx, unsigned char *dst)
{
    assert(dst != NULL);
    unsigned char *rt = rawTiles;
    assert(rawTiles);
    idx += (sideSize + lidSize) / 4096 + 1; // FIXME: assumes partition == block end
    rt += (idx / 4) * 4096 * 4 + (idx % 4) * 64;
    for (int i = 0; i < 64; i++) {
        memcpy(dst, rt, 64);
        dst += 64;
        rt += 64 * 4;
    }
}

unsigned char *GraphicsBase::getTmpBuffer(bool rgba = false)
{
    if (rgba)
        return tileTmpRGBA;
    return tileTmpRGB;
}
