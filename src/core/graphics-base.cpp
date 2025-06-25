#include <algorithm>
#include <cassert>
#include <cstddef>
#include <random>

#include <core/graphics-base.h>
#include <core/loaded-anim.h>
#include <core/sprite-info.h>

#include <util/errors.h>
#include <util/log.h>

using namespace OpenGTA;
using namespace Util;

UInt16 GraphicsBase::SpriteNumbers::countByType(const SpriteTypes &t) const
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
    throw std::runtime_error("UPS");
}

GraphicsBase::GraphicsBase(const std::string &style)
    : sideTexBlockMove(256)
    , styleFile { style }
{
    delta_is_a_set = false;
    for (int i = 0; i < 256; ++i)
        sideTexBlockMove.set_item(i, true);
}

bool GraphicsBase::isBlockingSide(UInt8 id) const
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

bool GraphicsBase::getDeltaHandling() const
{
    return delta_is_a_set;
}

void GraphicsBase::setDeltaHandling(bool delta_as_set)
{
    delta_is_a_set = delta_as_set;
}

GraphicsBase::~GraphicsBase() = default;

bool GraphicsBase::isAnimatedBlock(UInt8 area_code, UInt8 id)
{
    return std::ranges::any_of(animations, [&](const auto &anim) {
        return anim.which == area_code && anim.block == id;
    });
}

CarInfo &GraphicsBase::findCarByModel(UInt8 model)
{
    for (auto &car : carInfos) {
        if (car.model == model)
            return car;
    }
    throw Util::UnknownKey("Searching for car model {} failed", int(model));
}

unsigned int GraphicsBase::getRandomPedRemapNumber() const
{
    static thread_local std::mt19937 generator(std::random_device {}());
    std::uniform_int_distribution<int> distribution(firstValidPedRemap, lastValidPedRemap - 1);
    return distribution(generator);
}

unsigned int GraphicsBase::getPedRemapNumberType(unsigned int _type)
{
    ERROR("not implemented");
    return _type;
}

UInt8 GraphicsBase::getFormat() const
{
    if (_topHeaderSize == 52)
        return 0;
    if (_topHeaderSize == 64)
        return 1;
    throw Util::InvalidFormat("graphics-base header size");
}

UInt16 GraphicsBase::SpriteNumbers::reIndex(const UInt16 &id, const SpriteTypes &t) const
{
    UInt16 ret = id;
    switch (t) {
#define CASE_ACCUMULATE(_enum, _val) \
    case _enum:                      \
        ret += (_val);               \
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
    UInt64 st = static_cast<UInt64>(_topHeaderSize) + sideSize + lidSize + auxSize + auxBlockTrailSize;
    styleFile.seek(st);
    UInt8 numAnim;
    styleFile.read(numAnim);
    for (int i = 0; i < numAnim; i++)
        animations.emplace_back(styleFile);
}

void GraphicsBase::loadObjectInfo_shared(UInt64 offset)
{
    styleFile.ensurePosition(offset);
    assert(objectInfoSize % 20 == 0);
    int c = objectInfoSize / 20;

    for (int i = 0; i < c; i++)
        objectInfos.emplace_back(styleFile);
}

void GraphicsBase::loadCarInfo_shared(UInt64 offset)
{
    styleFile.ensurePosition(offset);

    UInt32 bytes_read = 0;
    while (bytes_read < carInfoSize) {
        CarInfo car { styleFile };
        bytes_read += car.bytes_read();
        carInfos.emplace_back(car);
    }
    assert(bytes_read == carInfoSize);
}

void GraphicsBase::loadSpriteNumbers_shared(UInt64 offset)
{

    styleFile.ensurePosition(offset);

    styleFile.read(spriteNumbers.GTA_SPRITE_ARROW);
    styleFile.read(spriteNumbers.GTA_SPRITE_DIGITS);
    styleFile.read(spriteNumbers.GTA_SPRITE_BOAT);
    styleFile.read(spriteNumbers.GTA_SPRITE_BOX);
    styleFile.read(spriteNumbers.GTA_SPRITE_BUS);
    styleFile.read(spriteNumbers.GTA_SPRITE_CAR);
    styleFile.read(spriteNumbers.GTA_SPRITE_OBJECT);
    styleFile.read(spriteNumbers.GTA_SPRITE_PED);
    styleFile.read(spriteNumbers.GTA_SPRITE_SPEEDO);
    styleFile.read(spriteNumbers.GTA_SPRITE_TANK);
    styleFile.read(spriteNumbers.GTA_SPRITE_TRAFFIC_LIGHTS);
    styleFile.read(spriteNumbers.GTA_SPRITE_TRAIN);
    styleFile.read(spriteNumbers.GTA_SPRITE_TRDOORS);
    styleFile.read(spriteNumbers.GTA_SPRITE_BIKE);
    styleFile.read(spriteNumbers.GTA_SPRITE_TRAM);
    styleFile.read(spriteNumbers.GTA_SPRITE_WBUS);
    styleFile.read(spriteNumbers.GTA_SPRITE_WCAR);
    styleFile.read(spriteNumbers.GTA_SPRITE_EX);
    styleFile.read(spriteNumbers.GTA_SPRITE_TUMCAR);
    styleFile.read(spriteNumbers.GTA_SPRITE_TUMTRUCK);
    styleFile.read(spriteNumbers.GTA_SPRITE_FERRY);
}

void GraphicsBase::loadTileTextures()
{
    styleFile.ensurePosition(_topHeaderSize);

    UInt64 ts = sideSize + lidSize + auxSize;
    rawTiles.resize(ts);
    styleFile.read(rawTiles.data(), rawTiles.size());
}

void GraphicsBase::handleDeltas(const SpriteInfo &info, unsigned char *buffer, UInt32 delta)
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
    UInt32 page_offset,
    const DeltaInfo &deltaInfo,
    bool mirror
)
{
    unsigned char *b = buffer + page_offset;
    unsigned char *delta = deltaInfo.ptr;
    Int32 length_to_go = deltaInfo.size;

    if (mirror) {
        UInt32 doff = 0;
        while (length_to_go > 0) {
            UInt16 *offset = (UInt16 *) delta;
            doff += *offset;
            delta += 2;
            unsigned char this_length = *delta;
            ++delta;
            UInt32 noff = page_offset + doff;
            UInt32 _y = noff / 256 * 256;
            UInt32 _x = doff % 256;
            for (int i = 0; i < this_length; i++)
                *(buffer + _y + spriteInfo.xoffset + spriteInfo.w - _x - i - 1) = *(delta + i);
            length_to_go -= (this_length + 3);
            doff += this_length;
            delta += this_length;
        }
        return;
    }

    while (length_to_go > 0) {
        UInt16 *offset = (UInt16 *) delta;
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

void GraphicsBase::prepareSideTexture(UInt32 idx, std::span<UInt8> dst)
{
    ++idx;
    auto dstIterator = dst.begin();
    auto tilesIterator = rawTiles.begin() + ((idx / 4) * 4096 * 4) + ((idx % 4) * 64);
    for (int i = 0; i < 64; ++i) {
        std::copy_n(tilesIterator, 64, dstIterator);
        dstIterator += 64;
        tilesIterator += 64 * 4;
    }
}

void GraphicsBase::prepareLidTexture(UInt32 idx, std::span<UInt8> dst)
{
    auto tilesIterator = rawTiles.begin();
    auto dstIterator = dst.begin();
    idx += sideSize / 4096 + 1; // FIXME: assumes partition == block end
    tilesIterator += ((idx / 4) * 4096 * 4) + ((idx % 4) * 64);
    for (int i = 0; i < 64; i++) {
        std::copy_n(tilesIterator, 64, dstIterator);
        dstIterator += 64;
        tilesIterator += 64 * 4;
    }
}

void GraphicsBase::prepareAuxTexture(UInt32 idx, std::span<UInt8> dst)
{
    auto tilesIterator = rawTiles.begin();
    auto dstIterator = dst.begin();
    idx += (sideSize + lidSize) / 4096 + 1; // FIXME: assumes partition == block end
    tilesIterator += ((idx / 4) * 4096 * 4) + ((idx % 4) * 64);
    for (int i = 0; i < 64; i++) {
        std::copy_n(tilesIterator, 64, dstIterator);
        dstIterator += 64;
        tilesIterator += 64 * 4;
    }
}

std::span<const UInt8> GraphicsBase::getTmpBuffer(bool rgba = false) const
{
    if (rgba)
        return tileTmpRGBA;
    return tileTmpRGB;
}
