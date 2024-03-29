#include "gui.h"
#include "m_exceptions.h"
#include "dataholder.h"
#include "gl_spritecache.h"
#include "gl_screen.h"
#include "sprite-info.h"
#include "lua_addon/lua_vm.h"
#include "localplayer.h"

extern float screen_gamma;
namespace GUI {
  Object::Object(const SDL_Rect & r) :
    id(0), rect(), color(), borderColor(), drawBorder(false) {
      copyRect(r);
      color.r = 255; color.g = 255; color.b = 255; color.a = 255;
    }
  Object::Object(const size_t Id, const SDL_Rect & r) :
    id(Id), rect(), color(), borderColor(), drawBorder(false) {
      copyRect(r);
      color.r = 255; color.g = 255; color.b = 255; color.a = 255;
    }
  Object::Object(const size_t Id, const SDL_Rect & r, const SDL_Color & c) :
    id(Id), rect(), color(),  borderColor(), drawBorder(false) {
      copyRect(r);
      copyColor(c);
    }
  void Object::draw([[maybe_unused]] Manager & manager) {
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4ub(color.r, color.g, color.b, color.a);
    glDisable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
    glVertex2i(rect.x, rect.y);
    glVertex2i(rect.x + rect.w, rect.y);
    glVertex2i(rect.x + rect.w, rect.y + rect.h);
    glVertex2i(rect.x, rect.y + rect.h);
    glEnd();
    if (drawBorder)
      draw_border();
    glDisable(GL_BLEND);
  }
  void Object::draw_border() {
    glColor4ub(borderColor.r, borderColor.g, borderColor.b, borderColor.a);
    glBegin(GL_LINE_LOOP);
    glVertex2i(rect.x, rect.y);
    glVertex2i(rect.x + rect.w, rect.y);
    glVertex2i(rect.x + rect.w, rect.y + rect.h);
    glVertex2i(rect.x, rect.y + rect.h);
    glEnd();
  }
  void Object::copyRect(const SDL_Rect & src) {
    rect.x = src.x;
    rect.y = src.y;
    rect.w = src.w;
    rect.h = src.h;
  }
  void Object::copyColor(const SDL_Color & src) {
    color.r = src.r;
    color.g = src.g;
    color.b = src.b;
    color.a = src.a;
  }
  void TexturedObject::draw(Manager & manager) {
    if (texId == 0)
      return;
    const OpenGL::PagedTexture & tex = manager.getCachedImage(texId);
    glColor4ub(color.r, color.g, color.b, color.a);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex.inPage);
    glBegin(GL_QUADS);
    glTexCoord2f(tex.coords[0].u, tex.coords[1].v);
    glVertex2i(rect.x, rect.y);
    glTexCoord2f(tex.coords[1].u, tex.coords[1].v);
    glVertex2i(rect.x + rect.w, rect.y);
    glTexCoord2f(tex.coords[1].u, tex.coords[0].v);
    glVertex2i(rect.x + rect.w, rect.y + rect.h);
    glTexCoord2f(tex.coords[0].u, tex.coords[0].v);
    glVertex2i(rect.x, rect.y + rect.h);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    if (drawBorder)
      draw_border();
  }

  void AnimatedTextureObject::draw(Manager & manager) {
    if (!animation)
      animation = manager.findAnimation(animId);

    assert(animation);
    size_t texId = animation->getCurrentFrame();
    const OpenGL::PagedTexture & tex = manager.getCachedImage(texId);
    glColor4ub(color.r, color.g, color.b, color.a);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex.inPage);
    glBegin(GL_QUADS);
    glTexCoord2f(tex.coords[0].u, tex.coords[1].v);
    glVertex2i(rect.x, rect.y);
    glTexCoord2f(tex.coords[1].u, tex.coords[1].v);
    glVertex2i(rect.x + rect.w, rect.y);
    glTexCoord2f(tex.coords[1].u, tex.coords[0].v);
    glVertex2i(rect.x + rect.w, rect.y + rect.h);
    glTexCoord2f(tex.coords[0].u, tex.coords[0].v);
    glVertex2i(rect.x, rect.y + rect.h);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    if (drawBorder)
      draw_border();
  }

  void Label::draw([[maybe_unused]] Manager & manager) {
    glPushMatrix();
    glColor4ub(color.r, color.g, color.b, color.a);
    glEnable(GL_TEXTURE_2D);
    if (align == 0) {
      glTranslatef(rect.x, rect.y, 0);
      rect.w = uint16_t(font.drawString(text));
    }
    else if (align == 1) {
      glTranslatef(rect.x, rect.y, 0);
      rect.w = uint16_t(font.drawString_r2l(text));
    }
    rect.h = font.getHeight();
    glPopMatrix();
    /*
       glDisable(GL_TEXTURE_2D);
       glBegin(GL_LINE_STRIP);
       glVertex2i(rect.x, rect.y);
       glVertex2i(rect.x + rect.w, rect.y);
       glVertex2i(rect.x + rect.w, rect.y + rect.h);
       glVertex2i(rect.x, rect.y + rect.h);
       glVertex2i(rect.x, rect.y);
       glEnd();
       */
    glDisable(GL_TEXTURE_2D);
    if (drawBorder)
      draw_border();
  }

  void Pager::draw(Manager & manager) {

    const OpenGL::PagedTexture & tex = manager.getCachedImage(texId);
    glColor4ub(color.r, color.g, color.b, color.a);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex.inPage);
    glBegin(GL_QUADS);
    glTexCoord2f(tex.coords[0].u, tex.coords[1].v);
    glVertex2i(rect.x, rect.y);
    glTexCoord2f(tex.coords[1].u, tex.coords[1].v);
    glVertex2i(rect.x + rect.w, rect.y);
    glTexCoord2f(tex.coords[1].u, tex.coords[0].v);
    glVertex2i(rect.x + rect.w, rect.y + rect.h);
    glTexCoord2f(tex.coords[0].u, tex.coords[0].v);
    glVertex2i(rect.x, rect.y + rect.h);
    glEnd();
    glScissor(rect.x, rect.y, rect.w, rect.h);
    glEnable(GL_SCISSOR_TEST);

    glPushMatrix();
    glColor4ub(color.r, color.g, color.b, color.a);
    glTranslatef(rect.x+offset, rect.y+4, 0);
    int slen = (int)font.drawString("test - hello world, this is a really long message. it will not end. never... or maybe sometime, but not yet. The end.");
    if (slen + offset < -10) {
      color.a = 0;
      manager.remove(this);
    }
    INFO("{} {}", slen, offset);
    glPopMatrix();

    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_TEXTURE_2D);
    if (drawBorder)
      draw_border();

  }

  void Pager::update(Uint32 ticks) {
    offset -= 1;

    //if (offset < -50)
    //  color.unused = 0;
  }

  void Manager::draw() {
    for (const auto &[key, layer] : guiLayers) {
      for (const auto &obj : layer) {
        obj->draw(*this);
      }
    }
    glColor4f(1, 1, 1, 1);
    glEnable(GL_TEXTURE_2D);
  }

  Manager::~Manager() {
    clearObjects();
    clearCache();
  }

  void Manager::clearObjects() {
    INFO("clearing gui objects");
    auto layer_it = guiLayers.begin();
    while (layer_it != guiLayers.end()) {
      GuiObjectList & list = layer_it->second;
      for (GuiObjectList::iterator i = list.begin(); i != list.end(); ++i) {
        delete *i;
      }
      list.clear();
      ++layer_it;
    }
    guiLayers.clear();
  }

  void Manager::clearCache() {
    INFO("clearing gui texture cache");
    for (const auto &[key, value] : texCache)
      glDeleteTextures(1, &value.inPage);
    texCache.clear();
    for (auto i = guiAnimations.begin(); i != guiAnimations.end(); ++i) {
      delete i->second;
    }
    guiAnimations.clear();
  }

  const OpenGL::PagedTexture & Manager::getCachedImage(size_t Id) {
    auto i = texCache.find(Id);
    if (i == texCache.end())
      throw Util::UnknownKey("cached texture id " + std::to_string(int(Id)));
    return i->second;
  }

  void Manager::cacheImageRAW(const std::string & file, size_t k) {
    texCache.insert(std::make_pair(k, ImageUtil::loadImageRAW(file)));
  }

  void Manager::cacheImageRAT(const std::string & file, const std::string & palette, size_t k) {
    texCache.insert(std::make_pair(k,
          ImageUtil::loadImageRATWithPalette(file, palette)));
  }

  ImageUtil::WidthHeightPair Manager::cacheStyleArrowSprite(const size_t id, int remap) {
    OpenGTA::GraphicsBase & graphics = OpenGTA::ActiveStyle::Instance().get();
    PHYSFS_uint16 t = graphics.spriteNumbers.reIndex(id, OpenGTA::GraphicsBase::SpriteNumbers::ARROW);
    OpenGTA::SpriteInfo * info = graphics.getSprite(t);
    texCache.insert(std::make_pair(
          id, OpenGL::SpriteCache::Instance().createSprite(size_t(t), remap, 0, info)
          ));
    return ImageUtil::WidthHeightPair(info->w, info->h);
  }

#ifdef WITH_SDL_IMAGE
  void Manager::cacheImageSDL(const std::string & file, size_t k) {
    texCache.insert(std::make_pair<size_t, OpenGL::PagedTexture>(k,
          ImageUtil::loadImageSDL(file)));
  }
#endif

  void Manager::add(Object * obj, uint8_t onLevel) {
    auto l = guiLayers.find(onLevel);
    if (l == guiLayers.end()) {
      GuiObjectList list;
      list.push_back(obj);
      guiLayers.insert(std::make_pair(onLevel, list));
      return;
    }
    GuiObjectList & list = l->second;
    list.push_back(obj);
  }

  void Manager::remove(Object * obj) {
    for (auto l = guiLayers.begin(); l != guiLayers.end(); ++l) {
      GuiObjectList & list = l->second;
      for (GuiObjectList::iterator m = list.begin(); m != list.end(); ++m) {
        Object * o = *m;
        if (o == obj) {
          delete o;
          list.erase(m);
          return;
        }
      }
    }
    throw Util::UnknownKey("not a managed object-ptr");
  }

  Object* Manager::findObject(const size_t id) {
    for (const auto &[key, layer] : guiLayers) {
      for (const auto &obj : layer) {
        if (obj->id == id)
          return obj;
      }
    }
    throw Util::UnknownKey("object by id " + std::to_string(int(id)));
  }

  void Manager::removeById(const size_t id) {
    Object * o = findObject(id);
    if (o)
      remove(o);
    //else
    //  ERROR << "failed to find object " << id << " - cannot remove it" << std::endl;
  }

  bool Manager::isInside(Object & obj, Uint16 x, Uint16 y) const {
    if ((obj.rect.x <= x) && (x <= obj.rect.x + obj.rect.w) &&
        (obj.rect.y <= y) && (y <= obj.rect.y + obj.rect.h))
      return true;
    return false;
  }
  void Manager::receive(SDL_MouseButtonEvent & mb_event, uint32_t height) {
    auto l = guiLayers.rbegin();
    while (l != guiLayers.rend()) {
      GuiObjectList & list = l->second;
      for (GuiObjectList::iterator i = list.begin(); i != list.end(); ++i) {
        Object * obj = *i;
        if (isInside(*obj, mb_event.x, height - mb_event.y)) {
      //    std::cout << "mouse inside obj id: " << obj->id << " at " << mb_event.x << "," << mb_event.y << std::endl;
          obj->receive(mb_event);
          return;
        }
      }
      ++l;
    }
  }

  void Manager::update(uint32_t nowticks) {
    for (AnimationMap::iterator i = guiAnimations.begin(); i != guiAnimations.end(); ++i) {
      i->second->update(nowticks);
    }
    for (const auto &[key, layer] : guiLayers) {
      for (const auto &obj : layer) {
        obj->update(nowticks);
      }
    }
  }

  Animation* Manager::findAnimation(uint16_t id) {
    auto i = guiAnimations.find(id);
    return i->second;
  }
  void Manager::createAnimation(const std::vector<uint16_t> & indices, uint16_t fps, size_t k) {
    Animation * anim = new Animation(indices, fps);
    guiAnimations.insert(std::make_pair(k, anim));
    anim->set(Util::Animation::PLAY_FORWARD, Util::Animation::LOOP);
  }

  uint16_t Animation::getCurrentFrame() {
    return indices[getCurrentFrameNumber()];
  }

  const int WEAPON_DISPLAY_ID = 100;

  const SDL_Rect sdl_rect(size_t a, size_t b, size_t c, size_t d) {
    SDL_Rect rect;
    rect.x = a;
    rect.y = b;
    rect.w = c;
    rect.h = d;
    return rect;
  }

  WeaponDisplay::WeaponDisplay(const SDL_Rect & r) : Object(WEAPON_DISPLAY_ID, r),
    img(sdl_rect(r.x+2, r.y+2, r.w - 4, r.h - 4), 0),
    label(sdl_rect(r.x+2, r.y+r.h, r.w - 4, r.h - 4), "", "F_MTEXT.FON", 1) {
  }

  void WeaponDisplay::draw(Manager & manager) {
    img.draw(manager);
    label.draw(manager);
  }

  size_t WeaponDisplay::getWeaponIdx(const size_t wt) {
    if (wt == 0)
      return 0;
    if (wt < 5)
      return 1000 + wt;

    return 0;
  }

  void WeaponDisplay::setWeapon(const size_t wt, Manager & manager) {
    img.texId = getWeaponIdx(wt);
    try {
      manager.getCachedImage(img.texId);
    }
    catch (const Util::UnknownKey & ek) {
      WARN("GUI image for weapon {} not loaded - retrying", wt);
      manager.cacheStyleArrowSprite(img.texId, -1);
    }
  }

  void ScrollBar::draw(Manager & manager) {
    Object::draw(manager);
    glColor3ub(innerColor.r, innerColor.g, innerColor.b);
    glBegin(GL_QUADS);
    glVertex2i(rect.x+2, rect.y+2);
    glVertex2i(rect.x + int(rect.w * value) -2, rect.y+2);
    glVertex2i(rect.x + int(rect.w * value) -2, rect.y + rect.h-2);
    glVertex2i(rect.x+2, rect.y + rect.h-2);
    glEnd();
  }

  void ScrollBar::receive(SDL_MouseButtonEvent & mb_event) {
    value = (mb_event.x - rect.x) / float(rect.w - 4);
    INFO("{}", value);
    if (changeCB)
      changeCB(value * 2);
    else
      WARN("No callback function set - I ain't seen nothing");
    /*
    SDL_SetGamma(value * 2, value * 2, value * 2);
    Object * o = Manager::Instance().findObject(101);
    if (o) {
      std::ostringstream os;
      os << "Gamma: " << value * 2;
      static_cast<Label*>(o)->text = os.str();
    }*/
  }


}
