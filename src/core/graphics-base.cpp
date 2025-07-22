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

uint16_t GraphicsBase::SpriteNumbers::countByType(const SpriteTypes &t) const
{
    switch (t) {
#define CASE_COUNT(_name)    \
    case SpriteTypes::_name: \
        return _name;

        SPRITE_TYPES(CASE_COUNT)
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

bool GraphicsBase::isBlockingSide(uint8_t id) const
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

bool GraphicsBase::isAnimatedBlock(uint8_t area_code, uint8_t id)
{
    return std::ranges::any_of(animations, [&](const auto &anim) {
        return anim.which == area_code && anim.block == id;
    });
}

const char *GraphicsBase::getSpriteName(int t)
{
#define STR_ARRAY_ITEM(name) #name,
    static const std::array types = { SPRITE_TYPES(STR_ARRAY_ITEM) };
    return (t < 0 || t >= types.size()) ? "???" : types[t];
}

CarInfo &GraphicsBase::findCarByModel(uint8_t model)
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

uint8_t GraphicsBase::getFormat() const
{
    if (_topHeaderSize == 52)
        return 0;
    if (_topHeaderSize == 64)
        return 1;
    throw Util::InvalidFormat("graphics-base header size");
}

uint16_t GraphicsBase::SpriteNumbers::reIndex(const uint16_t &id, const SpriteTypes &t) const
{
    uint16_t ret = id;
    switch (t) {
#define CASE_ACCUMULATE(_name, _to_add_name) \
    case SpriteTypes::_name:                 \
        ret += (_to_add_name);               \
        /* fallthrough */

        CASE_ACCUMULATE(ferry, tumtruck)
        CASE_ACCUMULATE(tumtruck, tumcar)
        CASE_ACCUMULATE(tumcar, ex)
        CASE_ACCUMULATE(ex, wcar)
        CASE_ACCUMULATE(wcar, wbus)
        CASE_ACCUMULATE(wbus, tram)
        CASE_ACCUMULATE(tram, bike)
        CASE_ACCUMULATE(bike, trdoors)
        CASE_ACCUMULATE(trdoors, train)
        CASE_ACCUMULATE(train, traffic_lights)
        CASE_ACCUMULATE(traffic_lights, tank)
        CASE_ACCUMULATE(tank, speedo)
        CASE_ACCUMULATE(speedo, ped)
        CASE_ACCUMULATE(ped, object)
        CASE_ACCUMULATE(object, car)
        CASE_ACCUMULATE(car, bus)
        CASE_ACCUMULATE(bus, box)
        CASE_ACCUMULATE(box, boat)
        CASE_ACCUMULATE(boat, digits)
        CASE_ACCUMULATE(digits, arrow)
        case SpriteTypes::arrow:
            break;
    }
    return ret;
}

void GraphicsBase::loadAnim()
{
    uint64_t st = static_cast<uint64_t>(_topHeaderSize) + sideSize + lidSize + auxSize + auxBlockTrailSize;
    styleFile.seek(st);
    uint8_t numAnim;
    styleFile.read(numAnim);
    for (int i = 0; i < numAnim; i++)
        animations.emplace_back(styleFile);
}

void GraphicsBase::loadObjectInfo_shared(uint64_t offset)
{
    styleFile.ensurePosition(offset);
    assert(objectInfoSize % 20 == 0);
    int c = objectInfoSize / 20;

    for (int i = 0; i < c; i++)
        objectInfos.emplace_back(styleFile);
}

void GraphicsBase::loadCarInfo_shared(uint64_t offset)
{
    styleFile.ensurePosition(offset);

    uint32_t bytes_read = 0;
    while (bytes_read < carInfoSize) {
        CarInfo car { styleFile };
        bytes_read += car.bytes_read();
        carInfos.emplace_back(car);
    }
    assert(bytes_read == carInfoSize);
}

void GraphicsBase::loadSpriteNumbers_shared(uint64_t offset)
{

    styleFile.ensurePosition(offset);

#define READ_SPRITE_NUMBER(name) styleFile.read(spriteNumbers.name);

    SPRITE_TYPES(READ_SPRITE_NUMBER)
}

void GraphicsBase::loadTileTextures()
{
    styleFile.ensurePosition(_topHeaderSize);

    uint64_t ts = sideSize + lidSize + auxSize;
    rawTiles.resize(ts);
    styleFile.read(rawTiles.data(), rawTiles.size());
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
    int32_t length_to_go = deltaInfo.size;

    if (mirror) {
        uint32_t doff = 0;
        while (length_to_go > 0) {
            uint16_t *offset = (uint16_t *) delta;
            doff += *offset;
            delta += 2;
            unsigned char this_length = *delta;
            ++delta;
            uint32_t noff = page_offset + doff;
            uint32_t _y = noff / 256 * 256;
            uint32_t _x = doff % 256;
            for (int i = 0; i < this_length; i++)
                *(buffer + _y + spriteInfo.xoffset + spriteInfo.w - _x - i - 1) = *(delta + i);
            length_to_go -= (this_length + 3);
            doff += this_length;
            delta += this_length;
        }
        return;
    }

    while (length_to_go > 0) {
        uint16_t *offset = (uint16_t *) delta;
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

void GraphicsBase::prepareSideTexture(uint32_t idx, std::span<uint8_t> dst)
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

void GraphicsBase::prepareLidTexture(uint32_t idx, std::span<uint8_t> dst)
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

void GraphicsBase::prepareAuxTexture(uint32_t idx, std::span<uint8_t> dst)
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

std::span<const uint8_t> GraphicsBase::getTmpBuffer(bool rgba = false) const
{
    if (rgba)
        return tileTmpRGBA;
    return tileTmpRGB;
}
