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
#pragma once

#include "game_objects.h"
#include "map_helper.h"
#include "train_system.h"

#include <list>
#include <map>

namespace OpenGTA {

class SpriteManager {
private:
    SpriteManager();
    ~SpriteManager();

public:
    SpriteManager(const SpriteManager &copy) = delete;
    SpriteManager &operator=(const SpriteManager &copy) = delete;

    static SpriteManager &Instance()
    {
        static SpriteManager instance;
        return instance;
    }

    void drawInRect(SDL_Rect &r);
    void clear();
    void removeDeadPeds();

    Pedestrian &add(const Pedestrian &ped);
    Pedestrian &getPed(uint32_t id);
    inline void removePed(uint32_t id) { _peds.erase(id); }
    inline std::map<uint32_t, Pedestrian> &getPeds() { return _peds; }

    Car &add(const Car &car);
    Car &getCar(uint32_t id);
    inline void removeCar(uint32_t id) { _cars.erase(id); }
    inline std::map<uint32_t, Car> &getCars() { return _cars; }

    SpriteObject &add(const SpriteObject &obj);
    SpriteObject &getObject(uint32_t id);
    inline void removeObject(uint32_t id) { _objects.erase(id); }
    inline std::map<uint32_t, SpriteObject> &getObjects() { return _objects; }

    inline bool getDrawTexture() { return (_drawMode & 1); }
    inline bool getDrawTexBorder() { return (_drawMode & 2); }
    inline bool getDrawBBox() { return (_drawMode & 4); }
    void setDrawTexture(bool v);
    void setDrawTexBorder(bool v);
    void setDrawBBox(bool v);
    void drawBBoxOutline(const OBox &);
    void drawTextureOutline(const float &, const float &);

    void draw(Car &);
    void draw(Pedestrian &);
    void draw(SpriteObject &);
    void draw(Projectile &);

    void drawExplosion(SpriteObject &);

    void update(Uint32 ticks, OpenGTA::LocalPlayer &player);

    SpriteObject::Animation &getAnimationById(const Uint32 &id);
    void registerAnimation(const Uint32 &id, const SpriteObject::Animation &anim);

    void createExplosion(Vector3D center);
    void createProjectile(uint8_t typeId, float, Vector3D p, Vector3D d, Uint32 &ticks, Uint32 &owner);

public:
    // TrainSystem   trainSystem;
    Util::SpriteCreationArea creationArea;

protected:
    std::map<Uint32, SpriteObject::Animation> animations;
    std::list<Projectile> activeProjectiles;

private:
    uint32_t _drawMode;
    uint32_t _lastCreateTick;

    std::map<uint32_t, Pedestrian> _peds;
    std::map<uint32_t, Car> _cars;
    std::map<uint32_t, SpriteObject> _objects;
};
} // namespace OpenGTA
