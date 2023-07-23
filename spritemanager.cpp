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
#include "spritemanager.h"

#include "dataholder.h"
#include "gl_spritecache.h"
#include "id_sys.h"
#include "log.h"
#include "sprite-info.h"
#include "timer.h"
#include "localplayer.h"

#include <SDL2/SDL_opengl.h>

namespace OpenGTA {

SpriteManager::SpriteManager()
{
    _drawMode = (1);

    registerAnimation(0, SpriteObject::Animation(0, 0)); // dummy

    registerAnimation(1, SpriteObject::Animation(98, 0));         // standing still
    registerAnimation(2, SpriteObject::Animation(0, 7, 0.001f));  // walking
    registerAnimation(3, SpriteObject::Animation(8, 7, 0.0015f)); // running

    //    registerAnimation(3, SpriteObject::Animation(16,  0)); // sitting in
    //    car registerAnimation(4, SpriteObject::Animation(17, 7)); // car-exit
    //    registerAnimation(5, SpriteObject::Animation(25, 7)); // car-enter
    // registerAnimation(3, SpriteObject::Animation(107, 7, 0.002f));
    // registerAnimation(4, SpriteObject::Animation(99, 7, 0.001f));
    // registerAnimation(5, SpriteObject::Animation(28, 7));
    //    registerAnimation(6, SpriteObject::Animation(38, 2)); // falling
    // registerAnimation(7, SpriteObject::Animation(41, 0)); // sliding under
    // registerAnimation(9, SpriteObject::Animation(44, 0)); // death-back pose
    // registerAnimation(11, SpriteObject::Animation(47, 1)); // swimming
    // registerAnimation(12, SpriteObject::Animation(98, 0)); // standing still

    registerAnimation(4, SpriteObject::Animation(89, 0));          // standing, gun
    registerAnimation(5, SpriteObject::Animation(99, 7, 0.001f));  // walking, gun
    registerAnimation(6, SpriteObject::Animation(107, 7, 0.002f)); // running, gun

    registerAnimation(7, SpriteObject::Animation(134,
                                                 0)); // standing, flamethrower
    registerAnimation(8, SpriteObject::Animation(118, 7,
                                                 0.001f)); // walking, flamethrower
    registerAnimation(9, SpriteObject::Animation(126, 7,
                                                 0.002f)); // running, flamethrower

    registerAnimation(10, SpriteObject::Animation(152, 0));         // standing, uzi
    registerAnimation(11, SpriteObject::Animation(136, 7, 0.001f)); // walking, uzi
    registerAnimation(12, SpriteObject::Animation(144, 7, 0.002f)); // running, uzi

    registerAnimation(13, SpriteObject::Animation(170,
                                                  0));              // standing, rocket-launcher
    registerAnimation(14, SpriteObject::Animation(154, 7, 0.001f)); // walking, rocket-launcher
    registerAnimation(15, SpriteObject::Animation(162, 7, 0.002f)); // running, rocket-launcher

    registerAnimation(42, SpriteObject::Animation(42,
                                                  1));     // death pose; maybe just 1?
    registerAnimation(45, SpriteObject::Animation(45, 1)); // shot-in-front

    registerAnimation(46, SpriteObject::Animation(354, 12));

    /*
       registerAnimation(12, SpriteObject::Animation(
       registerAnimation(13, SpriteObject::Animation(
       registerAnimation(14, SpriteObject::Animation(
       registerAnimation(15, SpriteObject::Animation(
       registerAnimation(16, SpriteObject::Animation(
       registerAnimation(17, SpriteObject::Animation(
       registerAnimation(18, SpriteObject::Animation(
       registerAnimation(19, SpriteObject::Animation(
       */
    registerAnimation(99, SpriteObject::Animation(0, 12));
    registerAnimation(100, SpriteObject::Animation(12, 12));
}
SpriteManager::~SpriteManager()
{
    clear();
    animations.clear();
}

Pedestrian &SpriteManager::add(const Pedestrian &ped)
{
    auto [it, inserted] = _peds.insert({ ped.id(), ped });
    assert(inserted);
    return it->second;
}

Pedestrian &SpriteManager::getPed(uint32_t id)
{
    if (auto it = _peds.find(id); it != _peds.end()) {
        return it->second;
    }
    throw Util::UnknownKey(std::to_string(id));
}

Car &SpriteManager::add(const Car &car)
{
    auto [it, inserted] = _cars.insert({ car.id(), car });
    assert(inserted);
    return it->second;
}

Car &SpriteManager::getCar(uint32_t id)
{
    if (auto it = _cars.find(id); it != _cars.end()) {
        return it->second;
    }
    throw Util::UnknownKey(std::to_string(id));
}

SpriteObject &SpriteManager::add(const SpriteObject &obj)
{
    auto [it, inserted] = _objects.insert({ obj.id(), obj });
    assert(inserted);
    return it->second;
}

SpriteObject &SpriteManager::getObject(uint32_t id)
{
    if (auto it = _objects.find(id); it != _objects.end()) {
        return it->second;
    }
    throw Util::UnknownKey(std::to_string(id));
}

void SpriteManager::update(Uint32 ticks, LocalPlayer &player)
{
    size_t num_peds, num_cars, num_obj;
    num_peds = 0;
    for (auto &[id, ped] : _peds) {
        ped.update(ticks);
        ++num_peds;
    }
    num_cars = 0;
    for (auto &[id, car] : _cars) {
        car.update(ticks);
        ++num_cars;
    }
    num_obj = 0;
    for (auto it = _objects.begin(); it != _objects.end();) {
        SpriteObject &obj = it->second;
        if (!obj.isActive) {
            _objects.erase(it++);
            continue;
        }
        obj.update(ticks);
        ++num_obj;
        ++it;
    }
    for (auto it = activeProjectiles.begin(); it != activeProjectiles.end();) {
        Projectile &pr = (*it);
        pr.update(ticks, player);
        if (pr.lastUpdateAt >= pr.endsAtTick) {
            activeProjectiles.erase(it++);
        } else {
            ++it;
        }
    }
    removeDeadPeds();
    if (num_peds < 50 && num_peds > 2 && ticks - _lastCreateTick > 100) {
        // MapHelper::createPeds(5);
        Map &map = OpenGTA::ActiveMap::Instance().get();
        _lastCreateTick = ticks;
        while (true) {
            Util::TupleOfUint8 tu8 = creationArea.getValidCoord();
            INFO("testing: {}, {}", tu8.first, tu8.second);
            int k = -1;
            for (int i = 0; i < map.getNumBlocksAtNew(tu8.first, tu8.second); ++i) {
                Map::BlockInfo *bi = map.getBlockAtNew(tu8.first, tu8.second, i);
                if (bi->blockType() == 3) {
                    k = i;
                    break;
                }
            }
            if (k == -1)
                continue;

            INFO("{} {} {}", tu8.first, tu8.second, k);

            Vector3D pos(tu8.first + 0.5f, k + 1, tu8.second + 0.5f);
            int id = OpenGTA::TypeIdBlackBox::Instance().requestId();
            Sint16 remap = OpenGTA::ActiveStyle::Instance().get().getRandomPedRemapNumber();
            OpenGTA::Pedestrian p(Vector3D(0.3f, 0.5f, 0.3f), pos, id, remap);
            p.rot = 360 * (rand() / (RAND_MAX + 1.0));
            Instance().add(p);
            break;
        }
    }
}

namespace {

inline bool isPositionInsideRect(const Vector3D &pos, const SDL_Rect &r)
{
    return (pos.x >= r.x) && (pos.x <= r.x + r.w) && (pos.z >= r.y) && (pos.z <= r.y + r.h);
}

} // namespace

void SpriteManager::drawInRect(SDL_Rect &r)
{
    for (auto it = _peds.begin(); it != _peds.end();) {
        Pedestrian &ped = it->second;
        if (isPositionInsideRect(ped.pos, r)) {
            draw(ped);
            ++it;
        } else {
            _peds.erase(it++);
        }
    }
    for (auto &[id, obj] : _objects) {
        if (isPositionInsideRect(obj.pos, r)) {
            draw(obj);
        }
    }
    for (auto &[id, car] : _cars) {
        if (isPositionInsideRect(car.pos, r)) {
            draw(car);
        }
    }

    glColor3f(0.2f, 0.2f, 0.2f);
    for (auto &prj : activeProjectiles) {
        if (isPositionInsideRect(prj.pos, r)) {
            draw(prj);
        }
    }
    glColor3f(1, 1, 1);
}

void SpriteManager::clear()
{
    _peds.clear();
    _cars.clear();
    _objects.clear();
    activeProjectiles.clear();
}

#define GL_OBJ_COMMON(o)                     \
    GL_CHECKERROR;                           \
    glPushMatrix();                          \
    glTranslatef(o.pos.x, o.pos.y, o.pos.z); \
    glRotatef(o.rot, 0, 1, 0)

#define DRAW_TEX_QUADS_OBJ(t, w, h)             \
    glBindTexture(GL_TEXTURE_2D, t.inPage);     \
    glBegin(GL_QUADS);                          \
    glTexCoord2f(t.coords[0].u, t.coords[1].v); \
    glVertex3f(-w / 2, 0.0f, h / 2);            \
    glTexCoord2f(t.coords[1].u, t.coords[1].v); \
    glVertex3f(w / 2, 0.0f, h / 2);             \
    glTexCoord2f(t.coords[1].u, t.coords[0].v); \
    glVertex3f(w / 2, 0.0f, -h / 2);            \
    glTexCoord2f(t.coords[0].u, t.coords[0].v); \
    glVertex3f(-w / 2, 0.0f, -h / 2);           \
    glEnd()

void SpriteManager::draw(Car &car)
{
    GL_OBJ_COMMON(car);
    GraphicsBase &style = ActiveStyle::Instance().get();
    OpenGL::PagedTexture t;
    PHYSFS_uint16 sprNum = style.spriteNumbers.reIndex(car.sprNum, car.sprType);
    //+ car.anim.firstFrameOffset + car.anim.currentFrame, car.sprType);

    SpriteInfo *info = style.getSprite(sprNum);
    assert(info);
    float w = float(info->w) / 64.0f;
    float h = float(info->h) / 64.0f;
    OpenGL::SpriteIdentifier si(sprNum, car.remap, car.delta);
    if (OpenGL::SpriteCache::Instance().has(si))
        t = OpenGL::SpriteCache::Instance().get(si);
    else {
        t = OpenGL::SpriteCache::Instance().create(car.sprNum, // +
                                                               // car.anim.firstFrameOffset + car.anim.currentFrame,
                                                   car.sprType, car.remap, car.delta);
    }

    DRAW_TEX_QUADS_OBJ(t, w, h);

    glDisable(GL_TEXTURE_2D);
    glBegin(GL_POINTS);
    glVertex3f(car.carInfo.door[0].rpx / 64.0f, 0.1f, car.carInfo.door[0].rpy / 64.0f);
    glEnd();
    glEnable(GL_TEXTURE_2D);

    if (getDrawBBox() || getDrawTexBorder())
        glDisable(GL_TEXTURE_2D);

    if (getDrawBBox())
        drawBBoxOutline(car);
    if (getDrawTexBorder())
        drawTextureOutline(w, h);

    if (getDrawBBox() || getDrawTexBorder())
        glEnable(GL_TEXTURE_2D);

    glPopMatrix();
    GL_CHECKERROR;
}

void SpriteManager::drawBBoxOutline(const OBox &box)
{
    glBegin(GL_LINE_STRIP);
    glVertex3f(-box.m_Extent.x, 0.0f, box.m_Extent.z);
    glVertex3f(box.m_Extent.x, 0.0f, box.m_Extent.z);
    glVertex3f(box.m_Extent.x, 0.0f, -box.m_Extent.z);
    glVertex3f(-box.m_Extent.x, 0.0f, -box.m_Extent.z);
    glVertex3f(-box.m_Extent.x, 0.0f, box.m_Extent.z);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glVertex3f(-box.m_Extent.x, box.m_Extent.y, box.m_Extent.z);
    glVertex3f(box.m_Extent.x, box.m_Extent.y, box.m_Extent.z);
    glVertex3f(box.m_Extent.x, box.m_Extent.y, -box.m_Extent.z);
    glVertex3f(-box.m_Extent.x, box.m_Extent.y, -box.m_Extent.z);
    glVertex3f(-box.m_Extent.x, box.m_Extent.y, box.m_Extent.z);
    glEnd();
}

void SpriteManager::drawTextureOutline(const float &w, const float &h)
{
    glBegin(GL_LINE_STRIP);
    glColor3f(float(202) / 255.0f, float(31) / 255.0f, float(123) / 255.0f);
    glVertex3f(-w / 2, 0.0f, h / 2);
    glVertex3f(w / 2, 0.0f, h / 2);
    glVertex3f(w / 2, 0.0f, -h / 2);
    glVertex3f(-w / 2, 0.0f, -h / 2);
    glVertex3f(-w / 2, 0.0f, h / 2);
    glEnd();
    glColor3f(1.0f, 1.0f, 1.0f);
}

void SpriteManager::draw(SpriteObject &obj)
{
    if (obj.sprType == GraphicsBase::SpriteNumbers::EX) {
        return drawExplosion(obj);
    }
    GL_OBJ_COMMON(obj);
    GraphicsBase &style = ActiveStyle::Instance().get();
    OpenGL::PagedTexture t;
    PHYSFS_uint16 sprNum =
        style.spriteNumbers.reIndex(obj.sprNum + obj.anim.firstFrameOffset + obj.anim.currentFrame, obj.sprType);

    SpriteInfo *info = style.getSprite(sprNum);
    assert(info);
    float w = float(info->w) / 64.0f;
    float h = float(info->h) / 64.0f;
    if (OpenGL::SpriteCache::Instance().has(sprNum, obj.remap))
        t = OpenGL::SpriteCache::Instance().get(sprNum, obj.remap);
    else {
        t = OpenGL::SpriteCache::Instance().create(obj.sprNum + obj.anim.firstFrameOffset + obj.anim.currentFrame,
                                                   obj.sprType, obj.remap);
    }

    DRAW_TEX_QUADS_OBJ(t, w, h);

    if (getDrawBBox() || getDrawTexBorder())
        glDisable(GL_TEXTURE_2D);

    if (getDrawBBox())
        drawBBoxOutline(obj);
    if (getDrawTexBorder())
        drawTextureOutline(w, h);

    if (getDrawBBox() || getDrawTexBorder())
        glEnable(GL_TEXTURE_2D);

    glPopMatrix();
    GL_CHECKERROR;
}

void SpriteManager::draw(Pedestrian &ped)
{
    GL_OBJ_COMMON(ped);

    GraphicsBase &style = ActiveStyle::Instance().get();
    OpenGL::PagedTexture t;
    PHYSFS_uint16 sprNum =
        style.spriteNumbers.reIndex(ped.sprNum + ped.anim.firstFrameOffset + ped.anim.currentFrame, ped.sprType);

    SpriteInfo *info = style.getSprite(sprNum);
    assert(info);
    float w = float(info->w) / 64.0f;
    float h = float(info->h) / 64.0f;

    if (OpenGL::SpriteCache::Instance().has(sprNum, ped.remap))
        t = OpenGL::SpriteCache::Instance().get(sprNum, ped.remap);
    else {
        t = OpenGL::SpriteCache::Instance().create(ped.sprNum + ped.anim.firstFrameOffset + ped.anim.currentFrame,
                                                   ped.sprType, ped.remap);
    }

    DRAW_TEX_QUADS_OBJ(t, w, h);

    if (getDrawBBox() || getDrawTexBorder())
        glDisable(GL_TEXTURE_2D);

    if (getDrawBBox())
        drawBBoxOutline(ped);
    if (getDrawTexBorder())
        drawTextureOutline(w, h);

    if (getDrawBBox() || getDrawTexBorder())
        glEnable(GL_TEXTURE_2D);

    glPopMatrix();
    GL_CHECKERROR;
}

void SpriteManager::drawExplosion(SpriteObject &obj)
{

    if (obj.anim.get() == Util::Animation::STOPPED) {
        obj.isActive = false;
        return;
    }
    glPushMatrix();
    glTranslatef(obj.pos.x, obj.pos.y, obj.pos.z);
    // glRotatef(obj.rot, 0, 1, 0);
    glGetFloatv(GL_MODELVIEW_MATRIX, *obj.m_M.m);

    GraphicsBase &style = ActiveStyle::Instance().get();

    PHYSFS_uint16 sprNum =
        style.spriteNumbers.reIndex(obj.sprNum + obj.anim.firstFrameOffset + obj.anim.currentFrame, obj.sprType);

    SpriteInfo *info = style.getSprite(sprNum);
    assert(info);
    float w = float(info->w) / 64.0f;
    float h = float(info->h) / 64.0f;
    OpenGL::PagedTexture t;
    if (OpenGL::SpriteCache::Instance().has(sprNum))
        t = OpenGL::SpriteCache::Instance().get(sprNum);
    else
        t = OpenGL::SpriteCache::Instance().create(obj.sprNum + obj.anim.firstFrameOffset + obj.anim.currentFrame,
                                                   obj.sprType, -1);

    glBindTexture(GL_TEXTURE_2D, t.inPage);

    glBegin(GL_QUADS);
    glTexCoord2f(t.coords[0].u, t.coords[1].v);
    glVertex3f(-w, 0.0f, 0.0f);
    glTexCoord2f(t.coords[1].u, t.coords[1].v);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glTexCoord2f(t.coords[1].u, t.coords[0].v);
    glVertex3f(0.0f, 0.0f, -h);
    glTexCoord2f(t.coords[0].u, t.coords[0].v);
    glVertex3f(-w, 0.0f, -h);

    glEnd();

    sprNum =
        style.spriteNumbers.reIndex(obj.sprNum + obj.anim.firstFrameOffset + obj.anim.currentFrame + 12, obj.sprType);
    info = style.getSprite(sprNum);
    assert(info);
    w = float(info->w) / 64.0f;
    h = float(info->h) / 64.0f;
    if (OpenGL::SpriteCache::Instance().has(sprNum))
        t = OpenGL::SpriteCache::Instance().get(sprNum);
    else
        t = OpenGL::SpriteCache::Instance().create(obj.sprNum + obj.anim.firstFrameOffset + obj.anim.currentFrame + 12,
                                                   obj.sprType, -1);

    glBindTexture(GL_TEXTURE_2D, t.inPage);

    glBegin(GL_QUADS);
    glTexCoord2f(t.coords[0].u, t.coords[1].v);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glTexCoord2f(t.coords[1].u, t.coords[1].v);
    glVertex3f(w, 0.0f, 0.0f);
    glTexCoord2f(t.coords[1].u, t.coords[0].v);
    glVertex3f(w, 0.0f, -h);
    glTexCoord2f(t.coords[0].u, t.coords[0].v);
    glVertex3f(0.0f, 0.0f, -h);

    glEnd();

    sprNum =
        style.spriteNumbers.reIndex(obj.sprNum + obj.anim.firstFrameOffset + obj.anim.currentFrame + 24, obj.sprType);
    info = style.getSprite(sprNum);
    assert(info);
    w = float(info->w) / 64.0f;
    h = float(info->h) / 64.0f;
    if (OpenGL::SpriteCache::Instance().has(sprNum))
        t = OpenGL::SpriteCache::Instance().get(sprNum);
    else
        t = OpenGL::SpriteCache::Instance().create(obj.sprNum + obj.anim.firstFrameOffset + obj.anim.currentFrame + 24,
                                                   obj.sprType, -1);

    glBindTexture(GL_TEXTURE_2D, t.inPage);

    glBegin(GL_QUADS);
    glTexCoord2f(t.coords[0].u, t.coords[1].v);
    glVertex3f(-w, 0.0f, 0.0f + h);
    glTexCoord2f(t.coords[1].u, t.coords[1].v);
    glVertex3f(0.0f, 0.0f, 0.0f + h);
    glTexCoord2f(t.coords[1].u, t.coords[0].v);
    glVertex3f(0.0f, 0.0f, -h + h);
    glTexCoord2f(t.coords[0].u, t.coords[0].v);
    glVertex3f(-w, 0.0f, -h + h);

    glEnd();

    sprNum =
        style.spriteNumbers.reIndex(obj.sprNum + obj.anim.firstFrameOffset + obj.anim.currentFrame + 36, obj.sprType);
    info = style.getSprite(sprNum);
    assert(info);
    w = float(info->w) / 64.0f;
    h = float(info->h) / 64.0f;
    if (OpenGL::SpriteCache::Instance().has(sprNum))
        t = OpenGL::SpriteCache::Instance().get(sprNum);
    else
        t = OpenGL::SpriteCache::Instance().create(obj.sprNum + obj.anim.firstFrameOffset + obj.anim.currentFrame + 36,
                                                   obj.sprType, -1);

    glBindTexture(GL_TEXTURE_2D, t.inPage);

    glBegin(GL_QUADS);
    glTexCoord2f(t.coords[0].u, t.coords[1].v);
    glVertex3f(0.0f, 0.0f, 0.0f + h);
    glTexCoord2f(t.coords[1].u, t.coords[1].v);
    glVertex3f(w, 0.0f, 0.0f + h);
    glTexCoord2f(t.coords[1].u, t.coords[0].v);
    glVertex3f(w, 0.0f, -h + h);
    glTexCoord2f(t.coords[0].u, t.coords[0].v);
    glVertex3f(0.0f, 0.0f, -h + h);

    glEnd();

    glPopMatrix();
    GL_CHECKERROR;
}

/*
   void SpriteManager::draw(TrainSegment & train) {
   GL_OBJ_COMMON(train);
   GraphicsBase & style = ActiveStyle::Instance().get();

   OpenGL::PagedTexture t;
   PHYSFS_uint16 sprNum = style.spriteNumbers.reIndex(train.sprNum,
   train.sprType);

   GraphicsBase::SpriteInfo * info = style.getSprite(sprNum);
   assert(info);
   float w = float(info->w) / 64.0f;
   float h = float(info->h) / 64.0f;


   if (OpenGL::SpriteCache::Instance().has(sprNum, train.remap))
   t = OpenGL::SpriteCache::Instance().get(sprNum, train.remap);
   else {
   t = OpenGL::SpriteCache::Instance().create(train.sprNum, train.sprType, -1);
   }
   glBindTexture(GL_TEXTURE_2D, t.inPage);

   glBegin(GL_QUADS);
   glTexCoord2f(t.coords[0].u, t.coords[1].v);
   glVertex3f(-w/2, 0.0f, h/2);
   glTexCoord2f(t.coords[1].u, t.coords[1].v);
   glVertex3f(w/2,  0.0f, h/2);
   glTexCoord2f(t.coords[1].u, t.coords[0].v);
   glVertex3f(w/2,  0.0f, -h/2);
   glTexCoord2f(t.coords[0].u, t.coords[0].v);
   glVertex3f(-w/2, 0.0f, -h/2);

   glEnd();


   glPopMatrix();
   GL_CHECKERROR;
   }
   */

void SpriteManager::draw(Projectile &proj)
{
    // GL_OBJ_COMMON(proj); // can't use; not derived from OBox
    const float w = 0.05f;
    const float h = 0.05f;

    glPushMatrix();
    glTranslatef(proj.pos.x, proj.pos.y, proj.pos.z);
    glRotatef(proj.rot, 0, 1, 0);

    glDisable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);

    glVertex3f(-w / 2, 0.0f, h / 2);
    glVertex3f(w / 2, 0.0f, h / 2);
    glVertex3f(w / 2, 0.0f, -h / 2);
    glVertex3f(-w / 2, 0.0f, -h / 2);

    glEnd();
    glEnable(GL_TEXTURE_2D);

    glPopMatrix();
    GL_CHECKERROR;
}

void SpriteManager::removeDeadPeds()
{
    for (auto it = _peds.cbegin(); it != _peds.cend();) {
        const Pedestrian &ped = it->second;
        // TODO: replace this hilarious "isDead == 3" with something sensible
        if (ped.isDead == 3 && !creationArea.isOnScreen(ped.pos)) {
            _peds.erase(it++);
        } else {
            ++it;
        }
    }
}

SpriteObject::Animation &SpriteManager::getAnimationById(const Uint32 &id)
{
    if (auto it = animations.find(id); it != animations.end()) {
        return it->second;
    }
    ERROR("Failed to find anim id: {}", id);
    return animations.begin()->second;
}

void SpriteManager::registerAnimation(const Uint32 &id, const SpriteObject::Animation &anim)
{
    animations.insert(std::make_pair(id, anim));
}

void SpriteManager::setDrawBBox(bool v)
{
    if (v)
        _drawMode = (_drawMode | 4);
    else
        _drawMode = (_drawMode & 4 ? _drawMode - 4 : _drawMode);
}

void SpriteManager::setDrawTexBorder(bool v)
{
    if (v)
        _drawMode = (_drawMode | 2);
    else
        _drawMode = (_drawMode & 2 ? _drawMode - 2 : _drawMode);
}

void SpriteManager::setDrawTexture(bool v) {}

void SpriteManager::createProjectile(uint8_t typeId, float r, Vector3D p, Vector3D d, Uint32 &ticks, Uint32 &owner)
{
    activeProjectiles.push_back(Projectile(typeId, r, p, d, ticks, owner));
}

void SpriteManager::createExplosion(Vector3D center)
{
    SpriteObject expl(center, 0, GraphicsBase::SpriteNumbers::EX);
    expl.anim = SpriteObject::Animation(getAnimationById(99));
    expl.anim.set(Util::Animation::PLAY_FORWARD, Util::Animation::STOP);
    add(expl);
}

} // namespace OpenGTA
