#include "graphics-base.h"

#include "loaded-anim.h"
#include "log.h"
#include "m_exceptions.h"
#include "sprite-info.h"

#include <cassert>

using namespace OpenGTA;
using namespace Util;

PHYSFS_uint16 GraphicsBase::SpriteNumbers::countByType(const SpriteTypes &t) const
{
    switch (t) {
        case ARROW:
            return GTA_SPRITE_ARROW;
        case DIGIT:
            return GTA_SPRITE_DIGITS;
        case BOAT:
            return GTA_SPRITE_BOAT;
        case BOX:
            return GTA_SPRITE_BOX;
        case BUS:
            return GTA_SPRITE_BUS;
        case CAR:
            return GTA_SPRITE_CAR;
        case OBJECT:
            return GTA_SPRITE_OBJECT;
        case PED:
            return GTA_SPRITE_PED;
        case SPEEDO:
            return GTA_SPRITE_SPEEDO;
        case TANK:
            return GTA_SPRITE_TANK;
        case TRAFFIC_LIGHT:
            return GTA_SPRITE_TRAFFIC_LIGHTS;
        case TRAIN:
            return GTA_SPRITE_TRAIN;
        case TRDOOR:
            return GTA_SPRITE_TRDOORS;
        case BIKE:
            return GTA_SPRITE_BIKE;
        case TRAM:
            return GTA_SPRITE_TRAM;
        case WBUS:
            return GTA_SPRITE_WBUS;
        case WCAR:
            return GTA_SPRITE_WCAR;
        case EX:
            return GTA_SPRITE_EX;
        case TUMCAR:
            return GTA_SPRITE_TUMCAR;
        case TUMTRUCK:
            return GTA_SPRITE_TUMTRUCK;
        case FERRY:
            return GTA_SPRITE_FERRY;
        default:
            break;
    }
    ERROR("UPS: {}", t);
    assert(0);
}

GraphicsBase::GraphicsBase()
    : sideTexBlockMove(256)
{
    rawTiles = NULL;
    rawSprites = NULL;
    delta_is_a_set = false;
    for (int i = 0; i < 256; ++i)
        sideTexBlockMove.set_item(i, true);
}

bool GraphicsBase::isBlockingSide(uint8_t id)
{
    return sideTexBlockMove.get_item(id);
}

void GraphicsBase::setupBlocking(const std::string &filename)
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
    std::vector<SpriteInfo *>::iterator i2 = spriteInfos.begin();
    while (i2 != spriteInfos.end()) {
        delete *i2;
        i2++;
    }
    spriteInfos.clear();
    if (rawTiles)
        delete[] rawTiles;
    if (rawSprites)
        delete[] rawSprites;
}

bool GraphicsBase::isAnimatedBlock(uint8_t area_code, uint8_t id)
{
    for (const auto &anim : animations) {
        if (anim.which == area_code && anim.block == id)
            return true;
    }
    return false;
}

CarInfo &GraphicsBase::findCarByModel(PHYSFS_uint8 model)
{
    for (auto &car : carInfos) {
        if (car.model == model)
            return car;
    }
    // throw std::string("Failed to find car by model");
    throw E_UNKNOWNKEY("Searching for car model " + std::to_string(int(model)) + " failed");
}

unsigned int GraphicsBase::getRandomPedRemapNumber()
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
    throw E_INVALIDFORMAT("graphics-base header size");
    return 255;
}

PHYSFS_uint16 GraphicsBase::SpriteNumbers::reIndex(const PHYSFS_uint16 &id, const SpriteTypes &t) const
{
    switch (t) {
        case ARROW:
            return id;
        case DIGIT:
            return GTA_SPRITE_ARROW + id;
        case BOAT:
            return GTA_SPRITE_ARROW + GTA_SPRITE_DIGITS + id;
        case BOX:
            return GTA_SPRITE_ARROW + GTA_SPRITE_DIGITS + GTA_SPRITE_BOAT + id;
        case BUS:
            return GTA_SPRITE_ARROW + GTA_SPRITE_DIGITS + GTA_SPRITE_BOAT + GTA_SPRITE_BOX + id;
        case CAR:
            return GTA_SPRITE_ARROW + GTA_SPRITE_DIGITS + GTA_SPRITE_BOAT + GTA_SPRITE_BOX + GTA_SPRITE_BUS + id;
        case OBJECT:
            return GTA_SPRITE_ARROW + GTA_SPRITE_DIGITS + GTA_SPRITE_BOAT + GTA_SPRITE_BOX + GTA_SPRITE_BUS +
                GTA_SPRITE_CAR + id;
        case PED:
            return GTA_SPRITE_ARROW + GTA_SPRITE_DIGITS + GTA_SPRITE_BOAT + GTA_SPRITE_BOX + GTA_SPRITE_BUS +
                GTA_SPRITE_CAR + GTA_SPRITE_OBJECT + id;
        case SPEEDO:
            return GTA_SPRITE_ARROW + GTA_SPRITE_DIGITS + GTA_SPRITE_BOAT + GTA_SPRITE_BOX + GTA_SPRITE_BUS +
                GTA_SPRITE_CAR + GTA_SPRITE_OBJECT + GTA_SPRITE_PED + id;
        case TANK:
            return GTA_SPRITE_ARROW + GTA_SPRITE_DIGITS + GTA_SPRITE_BOAT + GTA_SPRITE_BOX + GTA_SPRITE_BUS +
                GTA_SPRITE_CAR + GTA_SPRITE_OBJECT + GTA_SPRITE_PED + GTA_SPRITE_SPEEDO + id;
        case TRAFFIC_LIGHT:
            return GTA_SPRITE_ARROW + GTA_SPRITE_DIGITS + GTA_SPRITE_BOAT + GTA_SPRITE_BOX + GTA_SPRITE_BUS +
                GTA_SPRITE_CAR + GTA_SPRITE_OBJECT + GTA_SPRITE_PED + GTA_SPRITE_SPEEDO + GTA_SPRITE_TANK + id;
        case TRAIN:
            return GTA_SPRITE_ARROW + GTA_SPRITE_DIGITS + GTA_SPRITE_BOAT + GTA_SPRITE_BOX + GTA_SPRITE_BUS +
                GTA_SPRITE_CAR + GTA_SPRITE_OBJECT + GTA_SPRITE_PED + GTA_SPRITE_SPEEDO + GTA_SPRITE_TANK +
                +GTA_SPRITE_TRAFFIC_LIGHTS + id;
        case TRDOOR:
            return GTA_SPRITE_ARROW + GTA_SPRITE_DIGITS + GTA_SPRITE_BOAT + GTA_SPRITE_BOX + GTA_SPRITE_BUS +
                GTA_SPRITE_CAR + GTA_SPRITE_OBJECT + GTA_SPRITE_PED + GTA_SPRITE_SPEEDO + GTA_SPRITE_TANK +
                +GTA_SPRITE_TRAFFIC_LIGHTS + GTA_SPRITE_TRAIN + id;
        case BIKE:
            return GTA_SPRITE_ARROW + GTA_SPRITE_DIGITS + GTA_SPRITE_BOAT + GTA_SPRITE_BOX + GTA_SPRITE_BUS +
                GTA_SPRITE_CAR + GTA_SPRITE_OBJECT + GTA_SPRITE_PED + GTA_SPRITE_SPEEDO + GTA_SPRITE_TANK +
                +GTA_SPRITE_TRAFFIC_LIGHTS + GTA_SPRITE_TRAIN + GTA_SPRITE_TRDOORS + id;
        case TRAM:
            return GTA_SPRITE_ARROW + GTA_SPRITE_DIGITS + GTA_SPRITE_BOAT + GTA_SPRITE_BOX + GTA_SPRITE_BUS +
                GTA_SPRITE_CAR + GTA_SPRITE_OBJECT + GTA_SPRITE_PED + GTA_SPRITE_SPEEDO + GTA_SPRITE_TANK +
                +GTA_SPRITE_TRAFFIC_LIGHTS + GTA_SPRITE_TRAIN + GTA_SPRITE_TRDOORS + GTA_SPRITE_BIKE + id;
        case WBUS:
            return GTA_SPRITE_ARROW + GTA_SPRITE_DIGITS + GTA_SPRITE_BOAT + GTA_SPRITE_BOX + GTA_SPRITE_BUS +
                GTA_SPRITE_CAR + GTA_SPRITE_OBJECT + GTA_SPRITE_PED + GTA_SPRITE_SPEEDO + GTA_SPRITE_TANK +
                +GTA_SPRITE_TRAFFIC_LIGHTS + GTA_SPRITE_TRAIN + GTA_SPRITE_TRDOORS + GTA_SPRITE_BIKE + GTA_SPRITE_TRAM +
                id;
        case WCAR:
            return GTA_SPRITE_ARROW + GTA_SPRITE_DIGITS + GTA_SPRITE_BOAT + GTA_SPRITE_BOX + GTA_SPRITE_BUS +
                GTA_SPRITE_CAR + GTA_SPRITE_OBJECT + GTA_SPRITE_PED + GTA_SPRITE_SPEEDO + GTA_SPRITE_TANK +
                +GTA_SPRITE_TRAFFIC_LIGHTS + GTA_SPRITE_TRAIN + GTA_SPRITE_TRDOORS + GTA_SPRITE_BIKE + GTA_SPRITE_TRAM +
                GTA_SPRITE_WBUS + id;
        case EX:
            return GTA_SPRITE_ARROW + GTA_SPRITE_DIGITS + GTA_SPRITE_BOAT + GTA_SPRITE_BOX + GTA_SPRITE_BUS +
                GTA_SPRITE_CAR + GTA_SPRITE_OBJECT + GTA_SPRITE_PED + GTA_SPRITE_SPEEDO + GTA_SPRITE_TANK +
                +GTA_SPRITE_TRAFFIC_LIGHTS + GTA_SPRITE_TRAIN + GTA_SPRITE_TRDOORS + GTA_SPRITE_BIKE + GTA_SPRITE_TRAM +
                GTA_SPRITE_WBUS + GTA_SPRITE_WCAR + id;
        case TUMCAR:
            return GTA_SPRITE_ARROW + GTA_SPRITE_DIGITS + GTA_SPRITE_BOAT + GTA_SPRITE_BOX + GTA_SPRITE_BUS +
                GTA_SPRITE_CAR + GTA_SPRITE_OBJECT + GTA_SPRITE_PED + GTA_SPRITE_SPEEDO + GTA_SPRITE_TANK +
                +GTA_SPRITE_TRAFFIC_LIGHTS + GTA_SPRITE_TRAIN + GTA_SPRITE_TRDOORS + GTA_SPRITE_BIKE + GTA_SPRITE_TRAM +
                GTA_SPRITE_WBUS + GTA_SPRITE_WCAR + GTA_SPRITE_TUMCAR + id;
        case TUMTRUCK:
            return GTA_SPRITE_ARROW + GTA_SPRITE_DIGITS + GTA_SPRITE_BOAT + GTA_SPRITE_BOX + GTA_SPRITE_BUS +
                GTA_SPRITE_CAR + GTA_SPRITE_OBJECT + GTA_SPRITE_PED + GTA_SPRITE_SPEEDO + GTA_SPRITE_TANK +
                +GTA_SPRITE_TRAFFIC_LIGHTS + GTA_SPRITE_TRAIN + GTA_SPRITE_TRDOORS + GTA_SPRITE_BIKE + GTA_SPRITE_TRAM +
                GTA_SPRITE_WBUS + GTA_SPRITE_WCAR + GTA_SPRITE_TUMCAR + GTA_SPRITE_TUMCAR + id;
        case FERRY:
            return GTA_SPRITE_ARROW + GTA_SPRITE_DIGITS + GTA_SPRITE_BOAT + GTA_SPRITE_BOX + GTA_SPRITE_BUS +
                GTA_SPRITE_CAR + GTA_SPRITE_OBJECT + GTA_SPRITE_PED + GTA_SPRITE_SPEEDO + GTA_SPRITE_TANK +
                +GTA_SPRITE_TRAFFIC_LIGHTS + GTA_SPRITE_TRAIN + GTA_SPRITE_TRDOORS + GTA_SPRITE_BIKE + GTA_SPRITE_TRAM +
                GTA_SPRITE_WBUS + GTA_SPRITE_WCAR + GTA_SPRITE_TUMCAR + GTA_SPRITE_TUMCAR + GTA_SPRITE_TUMTRUCK + id;
    }
    assert(0); // should never be reached
    return 0;
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
    const SpriteInfo &spriteInfo, unsigned char *buffer, uint32_t page_offset, const DeltaInfo &deltaInfo, bool mirror
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