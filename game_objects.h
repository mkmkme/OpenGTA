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
#ifndef GAME_OBJECTS_H
#define GAME_OBJECTS_H

#include "OpenSteer/Proximity.h"
#include "animation.h"
#include "cell_iterator.h"
#include "entity_controller.h"
#include "graphics-base.h"
#include "math3d.h"
#include "obox.h"
#include "spritemanager.h"
#include "util/set.h"

#include <list>

namespace OpenGTA {

struct GameObject_common {
    Vector3D pos { 0, 0, 0 };
    float rot { 0 };
};

class Sprite {
public:
    struct Animation : public Util::Animation {
        Animation();
        Animation(const Animation &other);
        Animation(uint16_t foff, uint8_t num);
        Animation(uint16_t foff, uint8_t num, float speed);
        uint16_t firstFrameOffset { 0 };
        float moveSpeed { 0.0f };
    };
    // TODO: remove if compiler does not complain
    //    Sprite();
    Sprite(uint16_t sprN, int16_t rem, GraphicsBase::SpriteNumbers::SpriteTypes sprT, SpriteManager &spriteManager);
    uint16_t sprNum { 0 };
    int16_t remap { -1 };
    Animation anim {};
    uint32_t animId { 0 };
    GraphicsBase::SpriteNumbers::SpriteTypes sprType { GraphicsBase::SpriteNumbers::ARROW };
    void switchToAnim(uint32_t newId);

protected:
    SpriteManager &spriteManager;
};

class Pedestrian : public GameObject_common, public Sprite, public OBox {
public:
    Pedestrian(const Vector3D &, const Vector3D &, uint32_t id, SpriteManager &spriteManager, int16_t remapId = -1);
    Pedestrian(const Pedestrian &o);
    uint32_t pedId;
    [[nodiscard]] inline uint32_t id() const noexcept { return pedId; }
    PedController m_control;
    void update(uint32_t ticks);
    uint32_t lastUpdateAt;
    uint32_t lastWeaponTick {};
    Vector3D speedForces;
    bool inGroundContact { false };
    void tryMove(Vector3D nPos);
    uint8_t isDead { 0 };
    void getShot(uint32_t shooterId, uint32_t dmg, bool front = true);
    void die();
    using InventoryMap = std::map<uint8_t, uint32_t>;
    InventoryMap inventory;
    uint8_t activeWeapon;
    uint32_t aiMode = 0;
    struct AiData {
        AiData() = default;
        Vector3D pos1;
    };
    AiData aiData;
    Vector3D moveDelta;
};

class CarSprite {
public:
    class DoorDeltaAnimation : public Util::Animation {
    public:
        DoorDeltaAnimation(uint8_t dId, bool dOpen);
        uint8_t doorId;
        bool opening;
    };
    CarSprite();
    CarSprite(uint16_t sprN, int16_t rem, GraphicsBase::SpriteNumbers::SpriteTypes sprT);
    CarSprite(const CarSprite &o);
    uint16_t sprNum { 0 };
    int16_t remap { -1 };
    GraphicsBase::SpriteNumbers::SpriteTypes sprType { GraphicsBase::SpriteNumbers::CAR };
    uint32_t delta { 0 };
    Util::Set deltaSet { sizeof(delta) * 8, (unsigned char *) &delta };
    Util::Set animState {};
    void setDamage(uint8_t k);
    void openDoor(uint8_t k);
    void closeDoor(uint8_t k);
    void setSirenAnim(bool on);
    [[nodiscard]] bool assertDeltaById(uint8_t k) const;
    virtual void update(uint32_t ticks);

private:
    using DoorAnimList = std::list<DoorDeltaAnimation>;
    DoorAnimList doorAnims;
    uint32_t lt_siren {};
};

struct CarInfo;
class Car : public GameObject_common, public CarSprite, public OBox {
public:
    Car(const Car &o);
    Car(OpenGTA::Map::ObjectPosition &, uint32_t id);
    Car(Vector3D &_pos, float _rot, uint32_t id, uint8_t _type, int16_t _remap = -1);
    [[nodiscard]] inline uint32_t id() const { return carId; }
    CarInfo &carInfo;
    uint8_t type;
    void update(uint32_t ticks) override;
    void damageAt(const Vector3D &hit, uint32_t dmg);
    void explode();

private:
    void fixSpriteType();
    uint32_t carId;
    int32_t hitPoints;
};

class SpriteObject : public GameObject_common, public Sprite, public OBox {
public:
    SpriteObject(OpenGTA::Map::ObjectPosition &op, uint32_t id, SpriteManager &spriteManager);
    SpriteObject(
        const Vector3D &pos, uint16_t spriteNum, GraphicsBase::SpriteNumbers::SpriteTypes st,
        SpriteManager &spriteManager
    );
    SpriteObject(const SpriteObject &o);
    [[nodiscard]] inline uint32_t id() const noexcept { return objId; }

    void update(uint32_t ticks);

    bool isActive;

private:
    uint32_t objId {};
};

class LocalPlayer;
class Projectile : public GameObject_common {
public:
    Projectile(uint8_t, float, const Vector3D &, const Vector3D &, uint32_t, uint32_t);
    Projectile(const Projectile &other);
    uint8_t typeId;
    Vector3D delta;
    uint32_t endsAtTick;
    uint32_t owner;
    void update(uint32_t ticks, LocalPlayer &player);
    uint32_t lastUpdateAt;
    bool testCollideBlock(Util::CellIterator &, Vector3D &newp);
    bool testCollideBlock_flat(Util::CellIterator &, Vector3D &newp);
    static uint32_t damageByType(const uint8_t &k);
};

} // namespace OpenGTA

#endif
