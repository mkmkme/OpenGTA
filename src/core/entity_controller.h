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

#include <util/set.h>

namespace OpenGTA {
class EntityController {
public:
    EntityController();

    using Storage_T = uint32_t;

protected:
    EntityController(const EntityController &other);
    Storage_T rawData;
    Util::Set dataSet;
};

class ControllerWithMemory : public EntityController {
public:
    using EntityController::EntityController;
};

class Pedestrian;

enum class Turn : uint8_t {
    Straight = 0,
    Left = 1,
    Right = 2,
};

enum class Move : uint8_t {
    Stop = 0,
    Forward = 1,
    Backward = 2,
};

class PedController : public ControllerWithMemory {
public:
    void setTurnLeft(bool press = true);
    void releaseTurnLeft() { setTurnLeft(false); }
    void setTurnRight(bool press = true);
    void releaseTurnRight() { setTurnRight(false); }
    [[nodiscard]] Turn getTurn() const;
    void setMoveForward(bool press = true);
    void releaseMoveForward() { setMoveForward(false); }
    void setMoveBack(bool press = true);
    void releaseMoveBack() { setMoveBack(false); }
    [[nodiscard]] Move getMove() const;
    void setAction(bool press = true);
    void setJump(bool press = true);
    void setFireWeapon(bool press = true);
    void releaseFireWeapon() { setFireWeapon(false); }
    [[nodiscard]] bool getFireWeapon() const;

    void setActiveWeapon(unsigned char);
    [[nodiscard]] unsigned char getActiveWeapon() const;

    void setRunning(bool yes = true);
    [[nodiscard]] bool getRunning() const;
    void toggleRunning();

    // weapons, equip, shoot
};

// HeliController?
} // namespace OpenGTA
#endif
