/************************************************************************
 * Copyright (c) 2005-2007 tok@openlinux.org.uk                          *
 *                                                                       *
 * This software is provided as-is, without any express or implied       *
 * warranty. In no event will the authors be held liable for any         *
 * damages arising from the use of this software.                        *
 *                                                                       *
 * Permission is granted to anyone to use this software for any purpose, *
 * including commercial applications, and to alter it and redistribute   *
 * it freely, subject to the following restrictions:                     *
 *                                                                       *
 * 1. The origin of this software must not be misrepresented; you must   *
 * not claim that you wrote the original software. If you use this       *
 * software in a product, an acknowledgment in the product documentation *
 * would be appreciated but is not required.                             *
 *                                                                       *
 * 2. Altered source versions must be plainly marked as such, and must   *
 * not be misrepresented as being the original software.                 *
 *                                                                       *
 * 3. This notice may not be removed or altered from any source          *
 * distribution.                                                         *
 ************************************************************************/
#include <numbers>

#include <core/active-map.h>
#include <core/active-style.h>
#include <core/ai.h>
#include <core/game_objects.h>
#include <core/localplayer.h>
#include <core/sprite-info.h>
#include <core/spritemanager.h>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <math/plane.h>

#include <util/cell_iterator.h>
#include <util/log.h>
#include <util/timer.h>

#define INT2FLOAT_WRLD(c) (float(c >> 6) + (float(c % 64) / 64.0f))

float slope_height_offset(unsigned char slope_type, float dx, float dz);

namespace {
glm::mat4 rotationMatrix(const glm::vec3 &p, float rot)
{
    const auto tmp = glm::translate(glm::mat4(1.0f), p);
    return glm::rotate(tmp, glm::radians(rot), glm::vec3(0, 0, 1));
}
} // namespace

namespace OpenGTA {
float GameObject_common::heightOverTerrain(const glm::vec3 &v)
{
    float x, y, z;
    x = floor(v.x);
    y = floor(v.y);
    z = floor(v.z);
    uint8_t x_b, z_b;
    x_b = (uint8_t) x;
    z_b = (uint8_t) z;
    if (y < 0.0f) {
        // ERROR << "Below level! at coords: " << v.x << ", " << v.y << ", " << v.z << std::endl;
        return 1.0f;
    }
    if (x < 0 || x > 255 || z < 0 || z > 255) {
        throw Util::OutOfRange("invalid x/z pos, x = {} ({}), z = {} ({})", x, v.x, z, v.z);
    }
    if (y > 20) {
        WARN("{} seems a bit high; going to 20", y);
        INFO("{} {}", x, z);
        y = 20;
    }
    OpenGTA::Map &map = OpenGTA::ActiveMap::Instance().get();
    while (y >= map.getNumBlocksAtNew(x_b, z_b) && y > 0.0f)
        y -= 1.0f;
    while (y < map.getNumBlocksAtNew(x_b, z_b) && y > 0.0f) {
        OpenGTA::Map::BlockInfo *block = map.getBlockAtNew(x_b, z_b, (uint8_t) y);
        assert(block);
        if (block->blockType() > 0) {
            float bz = slope_height_offset(block->slopeType(), v.x - x, v.z - z);
            if (block->slopeType() == 0 && (block->blockType() != 5 && block->blockType() != 6))
                bz -= 1.0f;
            // INFO << "hit " << int(block->blockType()) << " at " << int(y) << std::endl;
            return v.y - (y + bz);
        }
        y -= 1.0f;
    }
    y = floor(v.y) + 1.0f;
    while (y < map.getNumBlocksAtNew(x_b, z_b) && y > 0.0f) {
        OpenGTA::Map::BlockInfo *block = map.getBlockAtNew(x_b, z_b, (uint8_t) y);
        assert(block);
        if (block->blockType() > 0) {
            float bz = slope_height_offset(block->slopeType(), v.x - x, v.z - z);
            if (block->slopeType() == 0 && (block->blockType() != 5 && block->blockType() != 6))
                bz -= 1.0f;
            // INFO << "hit " << int(block->blockType()) << " at " << int(y) << std::endl;
            return v.y - (y + bz);
        }
        y += 1.0f;
    }
    WARN("should this be reached?");
    return 1.0f;
}

Sprite::Animation::Animation() noexcept
    : Util::Animation(7, 7)
    , firstFrameOffset(0)
    , moveSpeed(0.0f)
{
}

Sprite::Animation::Animation(const Animation &other) noexcept
    : // Util::Animation(other.numFrames, 1000 / other.delay),
    Util::Animation(other)
    , firstFrameOffset(other.firstFrameOffset)
    // , numFrames(other.numFrames)
    , moveSpeed(other.moveSpeed)
{
    set(other.get(), other.getDone());
}

Sprite::Animation::Animation(uint16_t foff, uint8_t num) noexcept
    : Util::Animation(num, 7)
    , firstFrameOffset(foff)
    , moveSpeed(0.0f)
{
}

Sprite::Animation::Animation(uint16_t foff, uint8_t num, float speed) noexcept
    : Util::Animation(num, 7)
    , firstFrameOffset(foff)
    , moveSpeed(speed)
{
}

Sprite::Sprite() noexcept
    : sprNum(0)
    , remap(-1)
    //, anim(SpriteManager::Instance().getAnimationById(0)),
    , animId()
    , sprType(SpriteTypes::arrow)
{
}

Sprite::Sprite(uint16_t sprN, int16_t rem, SpriteTypes sprT) noexcept
    : sprNum(sprN)
    , remap(rem)
    , animId()
    , sprType(sprT)
{
}

Sprite::Sprite(const Sprite &other) noexcept = default;

void Sprite::switchToAnim(uint32_t newId)
{
    DEBUG("switching to anim {}", newId);
    anim = SpriteManager::Instance().getAnimationById(newId);
    anim.set(Util::Animation::Status::PlayForward, Util::Animation::OnDone::Loop);
    animId = newId;
}

Pedestrian::Pedestrian(const glm::vec3 &e, const glm::vec3 &p, uint32_t id, int16_t remapId) noexcept
    : GameObject_common(p)
    , Sprite(0, remapId, SpriteTypes::ped)
    , OBox(glm::translate(glm::mat4(1.0f), p), e * 0.5f)
    , m_control()
    , speedForces(0, 0, 0)
    , activeWeapon(0)
{
    transform_ = rotationMatrix(p, -rot);

    pedId = id;
    animId = 0;
    isDead = 0;
    lastUpdateAt = Timer::Instance().getRealTime();
    inGroundContact = false;
}

Pedestrian::Pedestrian(Pedestrian &&other) noexcept
    : GameObject_common(other)
    , Sprite(other)
    , OBox(other)
    , pedId(other.pedId)
    , m_control()
    , speedForces(other.speedForces)
    , inventory(other.inventory)
    , activeWeapon(other.activeWeapon)
    , aiData(other.aiData)
{
    lastUpdateAt = other.lastUpdateAt;
    inGroundContact = other.inGroundContact;
    animId = other.animId;
    isDead = other.isDead;
    transform_ = rotationMatrix(other.pos, -other.rot);
}

extern void ai_step_fake(Pedestrian *);
void Pedestrian::update(uint32_t ticks)
{
    if (isDead) {
        anim.update(ticks);
        lastUpdateAt = ticks;
        return;
    }
    // TODO reverendhomer: right now it calls the function from another module
    // which appears to be empty. Need to check why.
    // if (pedId < 0xffffffff)
    //   ai_step_fake(this);
    // AI::Pedestrian::walk_pavement(this);
    if (aiMode) {
        AI::Pedestrian::moveto_shortrange(this);
    }
    uint8_t chooseWeapon = m_control.getActiveWeapon();
    if (chooseWeapon != activeWeapon) {
        if (chooseWeapon == 0) {
            activeWeapon = 0;
        } else {
            auto i = inventory.find(chooseWeapon);
            if (i != inventory.end()) {
                activeWeapon = chooseWeapon;
            }
        }
    }
    activeWeapon = chooseWeapon;
    switch (m_control.getMove()) {
        case Move::Forward:
            if (m_control.getRunning()) {
                if (animId != 3u + activeWeapon * 3)
                    switchToAnim(3 + (activeWeapon * 3));
            } else {
                if (animId != 2u + activeWeapon * 3)
                    switchToAnim(2 + (activeWeapon * 3));
            }
            break;
        case Move::Stop:
            if (animId != 1u + activeWeapon * 3)
                switchToAnim(1 + (activeWeapon * 3));
            break;
        case Move::Backward:
            if (animId != 2u + activeWeapon * 3) {
                switchToAnim(2 + (activeWeapon * 3));
                anim.set(Util::Animation::Status::PlayBackward, Util::Animation::OnDone::Loop);
            }
    }
    anim.update(ticks);
    auto delta = ticks - lastUpdateAt;
    // INFO << "delta = " << delta  << " t: " << ticks << " lt: " << lastUpdateAt << std::endl;
    moveDelta = glm::vec3();
    switch (m_control.getTurn()) {
        case Turn::Right:
            rot -= 0.2f * delta;
            // INFO << "rot: "<< rot << std::endl;
            break;
        case Turn::Left:
            rot += 0.2f * delta;
            // INFO << "rot: "<< rot << std::endl;
            break;
        case Turn::Straight:
            break;
    }
    if (rot >= 360.0f)
        rot -= 360.0f;
    if (rot < 0.0f)
        rot += 360.0f;
    using std::numbers::pi;
    switch (m_control.getMove()) {
        case Move::Backward:
            moveDelta.x -= sin(rot * pi / 180.0f) * anim.moveSpeed * delta;
            moveDelta.z -= cos(rot * pi / 180.0f) * anim.moveSpeed * delta;
            break;
        case Move::Forward:
            moveDelta.x += sin(rot * pi / 180.0f) * anim.moveSpeed * delta;
            moveDelta.z += cos(rot * pi / 180.0f) * anim.moveSpeed * delta;
            break;
        case Move::Stop:
            break;
    }
    if (pedId == 0xffffffff) {
    }
    tryMove(pos + moveDelta);
    if (!inGroundContact) {
        speedForces.y += 0.0005f * delta;
        pos.y -= speedForces.y;
        if (speedForces.y < 0.2f)
            DEBUG("bridge step? height: {} speed: {}", pos.y, speedForces.y);
        else
            DEBUG("FALLING {} speed {}", pos.y, speedForces.y);
    } else {
        if (speedForces.y > 0.1)
            DEBUG("impacting with speed: {}", speedForces.y);
        speedForces.y = 0.0f;
    }
    transform_ = rotationMatrix(pos, rot);
    if (m_control.getFireWeapon() && ticks - lastWeaponTick > 400) {
        auto d1 = glm::normalize(glm::vec3(sin(rot * pi / 180.0f), 0, cos(rot * pi / 180.0f))) * 0.01f;
        SpriteManager::Instance().createProjectile(0, rot, pos, d1, ticks, pedId);
        lastWeaponTick = ticks;
    }
    /*
    if (m_control.statusChanged()) {
      INFO << "Ped-event id: " << pedId << " control: " << m_control.getRaw() <<
        " time: " << ticks << std::endl;
    }*/

    // INFO << pos.x << " " << pos.y << " " << pos.z << std::endl;
    lastUpdateAt = ticks;
}

void Pedestrian::tryMove(glm::vec3 nPos)
{
    float x, y, z;
    x = floor(nPos.x);
    y = floor(nPos.y);
    z = floor(nPos.z);
    OpenGTA::Map &map = OpenGTA::ActiveMap::Instance().get();
    OpenGTA::GraphicsBase &graphics = OpenGTA::ActiveStyle::Instance().get();
    // INFO << heightOverTerrain(nPos) << std::endl;
    float hot = heightOverTerrain(nPos);
    if (hot > 0.3f)
        inGroundContact = false;
    else if (hot < 0.0) {
        WARN("gone below: {} at {}, {}, {}", hot, nPos.x, nPos.y, nPos.z);
        nPos.y -= (hot - 0.3f);
        // nPos.y += 1;
        // INFO << nPos.y << std::endl;
        inGroundContact = true;
    } else {
        inGroundContact = true;
        if (isDead)
            nPos.y -= hot - 0.05f;
        else
            nPos.y -= hot - 0.1f;
    }
    if (y < map.getNumBlocksAtNew(uint8_t(x), uint8_t(z)) && y > 0.0f) {
        OpenGTA::Map::BlockInfo *block = map.getBlockAtNew(uint8_t(x), uint8_t(z), uint8_t(y));
        assert(block);
        if (block->left && graphics.isBlockingSide(block->left)) {
            if (block->isFlat()) {
                if (x - pos.x < 0 && x - pos.x > -0.2f) {
                    nPos.x = (nPos.x < pos.x) ? pos.x : nPos.x;
                } else if (x - pos.x > 0 && x - pos.x < 0.2f)
                    nPos.x = pos.x;
            } else {
#ifdef DEBUG_OLD_PED_BLOCK
                DEBUG("xblock left: {} tex: {}", x - pos.x, int(block->left));
#endif
                if (x - pos.x > 0 && x - pos.x < 0.2f)
                    nPos.x = pos.x;
                else if (x - pos.x < 0 && x - pos.x > -0.2f)
                    nPos.x = (nPos.x < pos.x) ? pos.x : nPos.x;
            }
        }
        if (block->right && !block->isFlat()) {
#ifdef DEBUG_OLD_PED_BLOCK
            DEBUG("xblock right: {} tex: {}", pos.x - x - 1, int(block->right));
#endif
            if (pos.x - x - 1 > 0 && pos.x - x - 1 < 0.2f) {
                nPos.x = pos.x;
            } else if (pos.x - x - 1 < 0 && pos.x - x - 1 > -0.2f)
                nPos.x = (nPos.x > pos.x) ? pos.x : nPos.x;
        }
        if (block->top && graphics.isBlockingSide(block->top)) {
            if (z - pos.z > 0 && z - pos.z < 0.2f)
                nPos.z = pos.z;
            else if (z - pos.z < 0 && z - pos.z > -0.2f)
                nPos.z = (nPos.z < pos.z) ? pos.z : nPos.z;
        }
        if (block->bottom && !block->isFlat()) {
#ifdef DEBUG_OLD_PED_BLOCK
            DEBUG("zblock bottom: {} tex: {}", pos.z - z - 1, int(block->bottom));
#endif
            if (pos.z - z - 1 > 0 && pos.z - z - 1 < 0.2f) {
                nPos.z = pos.z;
            } else if (pos.z - z - 1 < 0 && pos.z - z - 1 > -0.2f)
                nPos.z = (nPos.z > pos.z) ? pos.z : nPos.z;
        }
        if (x >= 1 && y < map.getNumBlocksAtNew(uint8_t(x - 1), uint8_t(z))) {
            block = map.getBlockAtNew(uint8_t(x - 1), uint8_t(z), uint8_t(y));
            if (block->right && !block->isFlat()) {
#ifdef DEBUG_OLD_PED_BLOCK
                DEBUG("xblock right: {} tex: {}", pos.x - x, int(block->right));
#endif
                if (pos.x - x < 0.2f) {
                    nPos.x = (nPos.x < pos.x ? pos.x : nPos.x);
                }
            }
        }
        if (x < 255 && y < map.getNumBlocksAtNew(uint8_t(x + 1), uint8_t(z))) {
            block = map.getBlockAtNew(uint8_t(x + 1), uint8_t(z), uint8_t(y));
            if (block->left && graphics.isBlockingSide(block->left)) {
#ifdef DEBUG_OLD_PED_BLOCK
                DEBUG("xblock left: {} tex: {}", x + 1 - pos.x, int(block->left));
#endif
                if (block->isFlat()) {
                    if (x + 1 - pos.x > 0 && x + 1 - pos.x < 0.2f)
                        nPos.x = (nPos.x < pos.x ? nPos.x : pos.x);
                } else {
                    if (x + 1 - pos.x < 0.2f)
                        nPos.x = (nPos.x < pos.x ? nPos.x : pos.x);
                }
            }
        }
        if (z >= 1 && y < map.getNumBlocksAtNew(uint8_t(x), uint8_t(z - 1))) {
            block = map.getBlockAtNew(uint8_t(x), uint8_t(z - 1), uint8_t(y));
            if (block->bottom && !block->isFlat()) {
#ifdef DEBUG_OLD_PED_BLOCK
                DEBUG("zblock bottom: {} tex: {}", pos.z - z, int(block->bottom));
#endif
                if (pos.z - z < 0.2f) {
                    nPos.z = (nPos.z < pos.z ? pos.z : nPos.z);
                }
            }
        }
        if (z < 255 && y < map.getNumBlocksAtNew(uint8_t(x), uint8_t(z + 1))) {
            block = map.getBlockAtNew(uint8_t(x), uint8_t(z + 1), uint8_t(y));
            if (block->top && graphics.isBlockingSide(block->top)) {
#ifdef DEBUG_OLD_PED_BLOCK
                DEBUG("zblock top: {} tex: {}", z + 1 - pos.z, int(block->top));
#endif
                if (block->isFlat()) {
                    if (z + 1 - pos.z > 0 && z + 1 - pos.z < 0.2f)
                        nPos.z = (nPos.z < pos.z ? nPos.z : pos.z);
                } else {
                    if (z + 1 - pos.z < 0.2f)
                        nPos.z = (nPos.z < pos.z ? nPos.z : pos.z);
                }
            }
        }
        // if (inGroundContact)
        //   pos = nPos;
    }
    bool obj_blocked = false;
    for (auto &[id, car] : SpriteManager::Instance().getCars()) {
        if (isBoxInBox(car) && glm::distance(pos, car.pos) > glm::distance(nPos, car.pos)) {
            obj_blocked = true;
            break;
        }
    }
    if (inGroundContact && !obj_blocked)
        pos = nPos;
    // else
    //   inGroundContact = 0;
}

void Pedestrian::die()
{
    DEBUG("DIE!!!");
    switchToAnim(42);
    if (isDead == 3) {
        anim.set(Util::Animation::Status::Stopped, Util::Animation::OnDone::Stop);
        return;
    }
    anim.set(Util::Animation::Status::PlayForward, Util::Animation::OnDone::FCallback);
    anim.setCallback([this]() { die(); });
    isDead++;
}

// TODO: should shooterId and other parameters be used?
void Pedestrian::getShot(
    [[maybe_unused]] uint32_t shooterId,
    [[maybe_unused]] uint32_t dmg,
    [[maybe_unused]] bool front
)
{
    isDead = 1;
    switchToAnim(45);
    anim.set(Util::Animation::Status::PlayForward, Util::Animation::OnDone::FCallback);
    anim.setCallback([this]() { die(); });
}

CarSprite::CarSprite()
    : sprNum(0)
    , remap(-1)
    , sprType(GraphicsBase::SpriteNumbers::SpriteTypes::car)
    , delta(0)
    , deltaSet(sizeof(delta) * 8, (unsigned char *) &delta)
{
}

CarSprite::CarSprite(const CarSprite &o)
    : sprNum(o.sprNum)
    , remap(o.remap)
    , sprType(o.sprType)
    , delta(o.delta)
    , deltaSet(sizeof(delta) * 8, (unsigned char *) &delta)
    , animState(o.animState)
{
}

CarSprite::CarSprite(uint16_t sprN, int16_t rem, GraphicsBase::SpriteNumbers::SpriteTypes sprT)
    : sprNum(sprN)
    , remap(rem)
    , sprType(sprT)
    , delta(0)
    , deltaSet(sizeof(delta) * 8, (unsigned char *) &delta)
{
}

void CarSprite::setDamage(uint8_t k)
{
    deltaSet.set_item(k, true);
}

bool CarSprite::assertDeltaById(uint8_t k) const
{
    GraphicsBase &style = ActiveStyle::Instance().get();
    const auto absNum = style.spriteNumbers.reIndex(sprNum, sprType);
    const SpriteInfo &info = style.getSprite(absNum);
    return k < info.deltaCount;
}

void CarSprite::openDoor(uint8_t k)
{
    DoorDeltaAnimation dda(k, true);
    doorAnims.push_back(dda);
}

void CarSprite::closeDoor(uint8_t k)
{
    doorAnims.emplace_back(k, false);
}

void CarSprite::setSirenAnim(bool on)
{
    if (!(assertDeltaById(15) && assertDeltaById(16))) {
        ERROR("Trying to set siren anim on car-sprite that has no such delta!");
        return;
    }
    animState.set_item(10, on);
}

void CarSprite::update(uint32_t ticks)
{

    // drive-anim
    if (animState.get_item(0)) {
    }
    /*
    if (ticks - lt_door > 500) {
      // 1-4 door-opening
      if (animState.get_item(1)) {
        deltaSet.set_item(6, true);
      }
      // 5-8 door-closing
      lt_door = ticks;
    }*/
    auto i = doorAnims.begin();
    while (i != doorAnims.end()) {
        i->update(ticks);
        if (i->opening) {
            if (i->doorId == 0) {
                for (int k = 6; k < 10; k++)
                    deltaSet.set_item(k, false);
                deltaSet.set_item(i->getCurrentFrameNumber() + 6, true);
            } else if (i->doorId == 1) {
                for (int k = 11; k < 15; k++)
                    deltaSet.set_item(k, false);
                deltaSet.set_item(i->getCurrentFrameNumber() + 11, true);
            } else if (i->doorId == 2) {
                for (int k = 20; k < 24; k++)
                    deltaSet.set_item(k, false);
                deltaSet.set_item(i->getCurrentFrameNumber() + 20, true);
            } else if (i->doorId == 3) {
                for (int k = 24; k < 28; k++)
                    deltaSet.set_item(k, false);
                deltaSet.set_item(i->getCurrentFrameNumber() + 24, true);
            }
        } else {
            if (i->doorId == 0) {
                for (int k = 6; k < 10; k++)
                    deltaSet.set_item(k, false);
                if (i->getCurrentFrameNumber() > 0)
                    deltaSet.set_item(i->getCurrentFrameNumber() + 5, true);
            } else if (i->doorId == 1) {
                for (int k = 11; k < 15; k++)
                    deltaSet.set_item(k, false);
                if (i->getCurrentFrameNumber() > 0)
                    deltaSet.set_item(i->getCurrentFrameNumber() + 10, true);
            } else if (i->doorId == 2) {
                for (int k = 20; k < 24; k++)
                    deltaSet.set_item(k, false);
                if (i->getCurrentFrameNumber() > 0)
                    deltaSet.set_item(i->getCurrentFrameNumber() + 19, true);
            } else if (i->doorId == 3) {
                for (int k = 24; k < 28; k++)
                    deltaSet.set_item(k, false);
                if (i->getCurrentFrameNumber() > 0)
                    deltaSet.set_item(i->getCurrentFrameNumber() + 23, true);
            }
        }
        if (i->get() == Util::Animation::Status::Stopped) {
            auto j = i;
            i++;
            animState.set_item(j->doorId + 1, j->opening);
            doorAnims.erase(j);
        } else
            i++;
    }
    if (animState.get_item(10)) {
        if (ticks - lt_siren > 500) {
            // Siren animation indices, former DSI_1 and DSI_2
            if (deltaSet.get_item(15)) {
                deltaSet.set_item(15, false);
                deltaSet.set_item(16, true);
            } else {
                deltaSet.set_item(15, true);
                deltaSet.set_item(16, false);
            }
            lt_siren = ticks;
        }
    }
}

CarSprite::DoorDeltaAnimation::DoorDeltaAnimation(uint8_t dId, bool dOpen)
    : Util::Animation(4 + (dOpen ? 0 : 1), 5)
    , doorId(dId)
    , opening(dOpen)
{
    if (!opening) {
        set(Util::Animation::Status::PlayBackward, Util::Animation::OnDone::Stop);
        jumpToFrame(4, Util::Animation::Status::PlayBackward);
    } else {
        set(Util::Animation::Status::PlayForward, Util::Animation::OnDone::Stop);
    }
}

Car::Car(const glm::vec3 &_pos, float _rot, uint32_t id, uint8_t _type, int16_t _remap)
    : GameObject_common(_pos, _rot)
    , CarSprite(0, -1, GraphicsBase::SpriteNumbers::SpriteTypes::car)
    , carInfo(ActiveStyle::Instance().get().findCarByModel(_type))
{
    type = _type;
    carId = id;
    sprNum = carInfo.sprNum;
    if ((_remap > -1) && (ActiveStyle::Instance().get().getFormat() == 0))
        remap = carInfo.remap8[_remap];
    fixSpriteType();
    extent_ = glm::vec3(carInfo.width, carInfo.depth, carInfo.height) / 128.0f;
    transform_ = rotationMatrix(pos, -rot);
    hitPoints = carInfo.damagable;
}

void Car::fixSpriteType()
{
    if (carInfo.vtype == 3)
        sprType = GraphicsBase::SpriteNumbers::SpriteTypes::bike;
    else if (carInfo.vtype == 0)
        sprType = GraphicsBase::SpriteNumbers::SpriteTypes::bus;
    else if (carInfo.vtype == 8)
        sprType = GraphicsBase::SpriteNumbers::SpriteTypes::train;
}

Car::Car(OpenGTA::Map::ObjectPosition &op, uint32_t id)
    : GameObject_common(glm::vec3(INT2FLOAT_WRLD(op.x), 6.05f - INT2FLOAT_WRLD(op.z), INT2FLOAT_WRLD(op.y)))
    , CarSprite(0, -1, GraphicsBase::SpriteNumbers::SpriteTypes::car)
    , carInfo(ActiveStyle::Instance().get().findCarByModel(op.type))
{
    carId = id;
    type = op.type;
    if (op.remap - 128 > 0) {
        if (ActiveStyle::Instance().get().getFormat() == 0)
            remap = carInfo.remap8[op.remap - 129];
        else
            WARN("remap {} requested but not implemented for G24", int(op.remap - 129));
    }
    sprNum = carInfo.sprNum;
    fixSpriteType();
    extent_ = glm::vec3(carInfo.width, carInfo.depth, carInfo.height) / 128.0f;

    rot = op.rotation * 360.f / 1024.f;
    transform_ = rotationMatrix(pos, -rot);
    hitPoints = carInfo.damagable;
}

Car::Car(const Car &other)
    : GameObject_common(other)
    , CarSprite(other)
    , OBox(other)
    , carInfo(ActiveStyle::Instance().get().findCarByModel(other.type))
{
    type = other.type;
    transform_ = rotationMatrix(pos, -rot);
    hitPoints = other.hitPoints;

    carId = other.carId;
}

void Car::update(uint32_t ticks)
{
    // m_M = TranslateMatrix3D(pos);
    // m_M.RotZ(rot+90);
    CarSprite::update(ticks);
}

void Car::damageAt(const glm::vec3 &hit, uint32_t dmg)
{
    float angle = Util::xz_angle(glm::vec3(), hit);
    INFO("hit angle: {}", angle);

    /*
     * front   rear
     *       270
     * 0 _|o---o| _ 180
     *    |o---o|
     * 45    90
     */
#define A1 60
#define A2 (180 - A1)
#define A3 (180 + A1)
#define A4 (360 - A1)

    uint8_t k; // TODO: lambda
    if (angle <= A1) {
        // front left
        k = 3;
    } else if (angle <= A2) {
        // left side
        k = 2;
    } else if (angle < 180.0f) {
        // rear left
        k = 1;
    } else if (angle <= A3) {
        // rear right
        k = 4;
    } else if (angle <= A4) {
        // right side
        k = 5;
    } else {
        // right front
        k = 0;
    }
    setDamage(k);
    hitPoints -= dmg;
    if (hitPoints <= 0)
        explode();
}

void Car::explode()
{
    // SpriteManager::Instance().removeCar(carId);
    // return;
    glm::vec3 exp_pos(pos);
    exp_pos.y += 0.1f;
    SpriteManager::Instance().createExplosion(exp_pos);
    sprNum = 0;
    remap = -1;
    sprType = GraphicsBase::SpriteNumbers::SpriteTypes::wcar;
    delta = 0;
}

SpriteObject::SpriteObject(OpenGTA::Map::ObjectPosition &op, uint32_t id)
    : GameObject_common(glm::vec3(INT2FLOAT_WRLD(op.x), 6.05f - INT2FLOAT_WRLD(op.z), INT2FLOAT_WRLD(op.y)))
    , Sprite(0, -1, GraphicsBase::SpriteNumbers::SpriteTypes::object)
{
    objId = id;
    GraphicsBase &style = ActiveStyle::Instance().get();
    const auto &info = style.objectInfos[op.type];
    sprNum = info.sprNum;
    extent_ = glm::vec3(info.width, info.depth, info.height) / 128.0f;
    transform_ = rotationMatrix(pos, -rot);
    rot = op.rotation * 360.f / 1024.f;
    isActive = true;
}

SpriteObject::SpriteObject(
    const glm::vec3 &pos,
    uint16_t spriteNum,
    OpenGTA::GraphicsBase::SpriteNumbers::SpriteTypes st
)
    : GameObject_common(pos)
    , Sprite(spriteNum, -1, st)
{
    isActive = true;
    transform_ = rotationMatrix(pos, -rot);
}

SpriteObject::SpriteObject(SpriteObject &&other) noexcept
    : GameObject_common(other)
    , Sprite(other)
    , OBox(other)
    , objId(other.objId)
{
    transform_ = rotationMatrix(pos, -rot);

    isActive = other.isActive;
}

void SpriteObject::update(uint32_t ticks)
{
    anim.update(ticks);
}

Projectile::Projectile(unsigned char t, float r, const glm::vec3 &p, const glm::vec3 &d, uint32_t ticks, uint32_t o)
    : GameObject_common(p, r)
    , typeId(t)
    , delta(d)
    , endsAtTick(ticks)
    , owner(o)
    , lastUpdateAt(ticks)
{
    endsAtTick = lastUpdateAt + 1000;
}

Projectile::Projectile(const Projectile &other) = default;

bool Projectile::testCollideBlock_flat(Util::CellIterator &ci, glm::vec3 &newp)
{
    Map::BlockInfo &bi = ci.getBlock();
    if (bi.top) {
        Math::Plane plane(glm::vec3(ci.x, ci.y, ci.z), glm::vec3(0, 0, -1));
        glm::vec3 hit_pos;
        if (plane.segmentIntersect(pos, newp, hit_pos)) {
            INFO("intersect flat-t: {} {} {}", hit_pos.x, hit_pos.y, hit_pos.z);
            if (hit_pos.x >= ci.x && hit_pos.x <= ci.x + 1) {
                newp = hit_pos;
                return true;
            }
        }
    }
    if (bi.left) {
        Math::Plane plane(glm::vec3(ci.x, ci.y, ci.z), glm::vec3(-1, 0, 0));
        glm::vec3 hit_pos;
        if (plane.segmentIntersect(pos, newp, hit_pos)) {
            INFO("intersect flat-l: {} {} {}", hit_pos.x, hit_pos.y, hit_pos.z);
            if (hit_pos.z >= ci.y && hit_pos.z <= ci.y + 1) {
                newp = hit_pos;
                return true;
            }
        }
    }
    return false;
}

bool Projectile::testCollideBlock(Util::CellIterator &ci, glm::vec3 &newp)
{
    Map::BlockInfo &bi = ci.getBlock();
    // INFO << "pos: " << ci.x << " " << ci.y << " " << ci.z << std::endl;
    // if (bi.isFlat())
    //   return false;
    if (bi.left) {
        Math::Plane plane(glm::vec3(ci.x, ci.y, ci.z), glm::vec3(-1, 0, 0));
        glm::vec3 hit_pos;
        if (plane.segmentIntersect(pos, newp, hit_pos)) {
            INFO("intersect left: {} {} {}", hit_pos.x, hit_pos.y, hit_pos.z);
            if (hit_pos.z >= ci.y && hit_pos.z <= ci.y + 1) {
                newp = hit_pos;
                return true;
            }
        }
    }
    if (bi.right && !bi.isFlat()) {
        Math::Plane plane(glm::vec3(ci.x + 1, ci.y, ci.z), glm::vec3(1, 0, 0));
        glm::vec3 hit_pos;
        if (plane.segmentIntersect(pos, newp, hit_pos)) {
            INFO("intersect right: {} {} {}", hit_pos.x, hit_pos.y, hit_pos.z);
            if (hit_pos.z >= ci.y && hit_pos.z <= ci.y + 1) {
                newp = hit_pos;
                return true;
            }
        }
    }
    if (bi.top) {
        Math::Plane plane(glm::vec3(ci.x, ci.y, ci.z), glm::vec3(0, 0, -1));
        glm::vec3 hit_pos;
        if (plane.segmentIntersect(pos, newp, hit_pos)) {
            INFO("intersect top: {} {} {}", hit_pos.x, hit_pos.y, hit_pos.z);
            if (hit_pos.x >= ci.x && hit_pos.x <= ci.x + 1) {
                newp = hit_pos;
                return true;
            }
        }
    }
    if (bi.bottom && !bi.isFlat()) {
        Math::Plane plane(glm::vec3(ci.x, ci.z, ci.y + 1), glm::vec3(0, 0, 1));
        glm::vec3 hit_pos;
        if (plane.segmentIntersect(pos, newp, hit_pos)) {
            INFO("intersect bottom: {} {} {}", hit_pos.x, hit_pos.y, hit_pos.z);
            if (hit_pos.x >= ci.x && hit_pos.x <= ci.x + 1) {
                newp = hit_pos;
                return true;
            }
        }
    }
    return false;
}

void Projectile::update(uint32_t ticks, LocalPlayer &player)
{
    auto dt = ticks - lastUpdateAt;
    glm::vec3 new_pos(pos + delta * float(dt));
    /*INFO << "p-m " << pos.x << " " << pos.y << " " << pos.z <<
      " to " << new_pos.x << " " << new_pos.y << " " << new_pos.z << std::endl;
      */
    for (auto &[id, ped] : SpriteManager::Instance().getPeds()) {
        if (ped.id() == owner || ped.isDead)
            continue;

        if (ped.isLineInBox(pos, new_pos)) {
            glm::vec3 p;
            ped.lineCrossBox(pos, new_pos, p);
            float angle = Util::xz_angle(glm::vec3(), p);
            INFO("{}", angle);
            if (angle <= 90.0f || angle > 270.0f)
                INFO("FRONT");
            else
                INFO("BACK");
            ped.getShot(owner, Projectile::damageByType(typeId), true);
            if (owner == player.getId()) {
                player.addCash(10);
                player.addWanted(1);
            }
            endsAtTick = 0;
        }
    }
    for (auto &[id, car] : SpriteManager::Instance().getCars()) {
        if (car.isLineInBox(pos, new_pos)) {
            INFO("CAR HIT");
            glm::vec3 p;
            car.lineCrossBox(pos, new_pos, p);
            car.damageAt(p, 5);
            delta = glm::vec3();
            p = car.transformCoords(p);
            new_pos.x = p.x;
            new_pos.z = p.z;
            new_pos.y += 0.1f;
        }
    }

    Util::CellIterator oi(pos);
    int collided = 0;
    if (oi.isValid()) {
        Map::BlockInfo &bi = oi.getBlock();
        if (bi.isFlat()) {
            collided += testCollideBlock_flat(oi, new_pos);
        }
        Util::CellIterator ni(oi.right());
        if (ni.isValid())
            collided += testCollideBlock(ni, new_pos);
        ni = oi.left();
        if (ni.isValid())
            collided += testCollideBlock(ni, new_pos);
        ni = oi.top();
        if (ni.isValid())
            collided += testCollideBlock(ni, new_pos);
        ni = oi.bottom();
        if (ni.isValid())
            collided += testCollideBlock(ni, new_pos);
    }
    if (collided)
        delta = glm::vec3(0, 0, 0);
    pos = new_pos;

    lastUpdateAt = ticks;
}

uint32_t Projectile::damageByType(const uint8_t &k)
{
    return k == 2 ? 150 : 7;
}
} // namespace OpenGTA
