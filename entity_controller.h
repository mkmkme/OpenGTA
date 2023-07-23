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
#ifndef OBJECT_CONTROLLER_H
#define OBJECT_CONTROLLER_H

#include <cstdint>
#include "set.h"

namespace OpenGTA {
  class EntityController {
    public:
      EntityController();

        typedef uint32_t Storage_T;

  protected:
      EntityController(const EntityController & other);
      Storage_T rawData;
      Util::Set    dataSet;
  };

  class ControllerWithMemory : public EntityController {
    public:
      ControllerWithMemory() : EntityController() {}
  };

  class Pedestrian;
  class PedController : public ControllerWithMemory {
    public:
      void setTurnLeft(bool press = true);
      inline void releaseTurnLeft() { setTurnLeft(false); }
      void setTurnRight(bool press = true);
      inline void releaseTurnRight() { setTurnRight(false); }
      signed char getTurn();
      void setMoveForward(bool press = true);
      inline void releaseMoveForward() { setMoveForward(false); }
      void setMoveBack(bool press = true);
      inline void releaseMoveBack() { setMoveBack(false); }
      signed char getMove();
      void setAction(bool press = true);
      void setJump(bool press = true);
      void setFireWeapon(bool press = true);
      inline void releaseFireWeapon() { setFireWeapon(false); }
      bool getFireWeapon();

      void setActiveWeapon(unsigned char);
      unsigned char getActiveWeapon();

      void setRunning(bool yes = true);
      bool getRunning();

      // weapons, equip, shoot
  };

  // HeliController?
}
#endif
