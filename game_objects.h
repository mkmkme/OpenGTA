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

#include "math3d.h"
#include "obox.h"
#include "animation.h"
#include "graphics-base.h"
#include "cell_iterator.h"
#include "entity_controller.h"
#include "OpenSteer/Proximity.h"
#include "util/set.h"

#include <list>

namespace OpenGTA {

  class LocalPlayer;
  struct CarInfo;
  struct GameObject_common;
  struct GameObject_common {
    Vector3D pos;
    float    rot;
    // uint8_t  activeState;
    GameObject_common() :
      pos(0, 0, 0), rot(0) {}
    explicit GameObject_common(const Vector3D & p) : pos(p), rot(0) {}
    GameObject_common(const Vector3D & p, float r) : pos(p), rot(r) {}
    float heightOverTerrain(const Vector3D &);
  };

  class Sprite {
    public:
      struct Animation : public Util::Animation {
        Animation();
        Animation(const Animation & other);
        Animation(uint16_t foff, uint8_t num);
        Animation(uint16_t foff, uint8_t num, float speed);
        uint16_t firstFrameOffset;
        // uint8_t  numFrames;
        float  moveSpeed;
      };
      Sprite();
      Sprite(uint16_t sprN, int16_t rem, GraphicsBase::SpriteNumbers::SpriteTypes sprT);
      Sprite(const Sprite & o);
      uint16_t sprNum;
      int16_t remap;
      Animation anim;
      uint32_t animId;
      GraphicsBase::SpriteNumbers::SpriteTypes sprType;
      void switchToAnim(uint32_t newId);
  };

  class Pedestrian : public GameObject_common, public Sprite, public OBox {
    public:
      Pedestrian(const Vector3D&, const Vector3D &, uint32_t id, int16_t remapId = -1);
      Pedestrian(const Pedestrian & o);
      uint32_t pedId;
      [[nodiscard]] inline uint32_t id() const { return pedId; }
      PedController m_control;
      void update(uint32_t ticks);
      uint32_t lastUpdateAt;
      uint32_t lastWeaponTick{};
      Vector3D speedForces;
      bool inGroundContact;
      void tryMove(Vector3D nPos);
      uint8_t isDead;
      void getShot(uint32_t shooterId, uint32_t dmg, bool front = true);
      void die();
      typedef std::map<uint8_t, uint32_t> InventoryMap;
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
          bool    opening;
      };
      CarSprite();
      CarSprite(uint16_t sprN, int16_t rem, GraphicsBase::SpriteNumbers::SpriteTypes sprT);
      CarSprite(const CarSprite & o);
      uint16_t sprNum;
      int16_t remap;
      GraphicsBase::SpriteNumbers::SpriteTypes sprType;
      uint32_t delta;
      Util::Set deltaSet;
      Util::Set animState;
      void setDamage(uint8_t k);
      void openDoor(uint8_t k);
      void closeDoor(uint8_t k);
      void setSirenAnim(bool on);
      bool assertDeltaById(uint8_t k) const;
      virtual void update(uint32_t ticks);
    private:
      typedef std::list<DoorDeltaAnimation> DoorAnimList;
      DoorAnimList doorAnims;
      uint32_t lt_siren{};
  };

  class Car : public GameObject_common, public CarSprite, public OBox {
    public:
      Car(const Car & o);
      Car(OpenGTA::Map::ObjectPosition&, uint32_t id);
      Car(Vector3D & _pos, float _rot, uint32_t id, uint8_t _type, int16_t _remap = -1);
      uint32_t carId;
      [[nodiscard]] inline uint32_t id() const { return carId; }
      CarInfo & carInfo;
      uint8_t type;
      void update(uint32_t ticks) override;
      void damageAt(const Vector3D & hit, uint32_t dmg);
      void explode();
    private:
      void fixSpriteType();
      int32_t hitPoints;
  };

  class SpriteObject : public GameObject_common, public Sprite, public OBox {
    public:
      SpriteObject(OpenGTA::Map::ObjectPosition&, uint32_t id);
      SpriteObject(const Vector3D& pos, uint16_t spriteNum, GraphicsBase::SpriteNumbers::SpriteTypes st);
      SpriteObject(const SpriteObject & o);
      uint32_t objId{};
      [[nodiscard]] inline uint32_t id() const { return objId; }
      void update(uint32_t ticks);

      bool isActive;

  };

  /*
  class TrainSegment : public GameObject_common, public OBox {
    public:
      TrainSegment(uint32_t id, Util::CellIterator & cell);
      TrainSegment(const TrainSegment & o);
      uint32_t trainId;
      inline uint32_t id() const { return trainId; }
  };*/

  class Projectile : public GameObject_common {
    public:
      Projectile(uint8_t, float, const Vector3D&, const Vector3D&, uint32_t, uint32_t);
      Projectile(const Projectile & other);
      uint8_t typeId;
      Vector3D delta;
      uint32_t endsAtTick;
      uint32_t owner;
      void update(uint32_t ticks, LocalPlayer & player);
      uint32_t lastUpdateAt;
      bool testCollideBlock(Util::CellIterator &, Vector3D & newp);
      bool testCollideBlock_flat(Util::CellIterator &, Vector3D & newp);
      static uint32_t damageByType(const uint8_t & k);
  };

}

#endif
