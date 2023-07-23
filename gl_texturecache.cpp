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
#include <limits>
#include "gl_texturecache.h"
#include "log.h"

namespace OpenGL {
  template <typename key_type>
    TextureCache<key_type>::TextureCache(const char* with_name) : m_name(with_name) {
      instance_id = instance_count++;
      clearMagic = 0;
      has_cached_query = false;
      last_query_result = nullptr;
      minClearElements = 50;
    }
  template <typename key_type>
    TextureCache<key_type>::TextureCache() {
      instance_id = instance_count++;
      m_name = "TextureCache_" + std::to_string(instance_count);
      has_cached_query = false;
      clearMagic = 0;
      minClearElements = 50;
    }

  template <typename key_type>
    TextureCache<key_type>::~TextureCache() {
      unsigned int ts = cached.size();
      clearAll();
      INFO("{} exited - {} textures recycled", m_name, ts);
      m_name.clear();
      instance_count--;
    }

  template <typename key_type>
    void TextureCache<key_type>::clearAll() {
      for (auto &c : cached) {
        GLuint tid = c.second->texId;
        glDeleteTextures(1, &tid);
        delete c.second;
      }
      cached.clear();
    }

  template <typename key_type>
    void TextureCache<key_type>::status() {
      INFO("* {} status: {} textures total, position = game_id : usage_count",
           m_name,
           cached.size());
      printStats();
    }

  template <typename key_type>
    void TextureCache<key_type>::sink() {
      constexpr auto MAX_4 = std::numeric_limits<unsigned int>::max() / 4;
      constexpr auto MAX_2 = std::numeric_limits<unsigned int>::max() / 2;
      for (auto &c : cached) {
        if (c.second->refCount <= 1)
          c.second->refCount = 0;
        else if (c.second->refCount < MAX_4)
          c.second->refCount = c.second->refCount >> 1;
        else if (c.second->refCount < MAX_2) {
          INFO("{} texture id {} -- half-count reached", m_name, int(c.first));
          c.second->refCount = c.second->refCount >> 2;
        }
        else {
          WARN("{} texture id {} -- going critical", m_name, int(c.first));
          c.second->refCount = c.second->refCount >> 3;
        }
      }
    }

  template <typename key_type>
    void TextureCache<key_type>::clear() {
      if (clearMagic == 0)
        return;
      if (cached.size() < minClearElements)
        return;

      uint32_t numCleared = 0;
      for (auto i = cached.begin(); i != cached.end(); ) {
        if (i->second->refCount < clearMagic) {
          DEBUG("{} clearing: {} count: {}", m_name, int(i->first), i->second->refCount);
          GLuint tid = (i->second->texId);
          glDeleteTextures(1, &tid);
          delete i->second;
          i = cached.erase(i);
          numCleared++;
        } else {
          i++;
        }
      }
      INFO("{} {} textures recycled", m_name, numCleared);
    }

  template <typename key_type>
    void TextureCache<key_type>::clearStats() {
      for (const auto& [key, value] : cached) {
        value->refCount = 0;
      }
    }

  template <typename key_type>
    void TextureCache<key_type>::printStats() {
      size_t c = 1;
      size_t c_active = 0;
      for (const auto& [key, value] : cached) {
        if (value->refCount > 0) {
          INFO("{} = {} : {}", c, uint32_t(key), value->refCount);
          ++c_active;
        }
        ++c;
      }
      INFO("{} different textures used", c_active);
    }

  template <typename key_type>
    GLuint TextureCache<key_type>::getTextureWithId(key_type id) {
      if (matchingCachedQuery(id)) {
        last_query_result->refCount++;
        return last_query_result->texId;
      }
      auto i = cached.find(id);
      if (i == cached.end()) {
        ERROR("{} failed to find texture {}", m_name, int(id));
        return 0;
      } else {
        cacheQuery(id, i->second);
        i->second->refCount++;
      }
      /*
       * if (i->second->isAnimated) {
       AnimControl->lookup(i->second)
       * }
       */
      return i->second->texId;
    }

  template <typename key_type>
    bool TextureCache<key_type>::hasTexture(key_type id) {
      if (matchingCachedQuery(id))
        return true; // last_query_result;
      auto i = cached.find(id);
      if (i == cached.end())
        return false;
      cacheQuery(id, i->second);
      return true;
    }

  template <typename key_type>
    void TextureCache<key_type>::setToAlpha(key_type id) {
      auto i = cached.find(id);
      if (i == cached.end()) {
        ERROR("{} texture not found when trying to set alpha", m_name);
        return;
      }
      i->second->hasAlpha = true;
    }

  template <typename key_type>
    void TextureCache<key_type>::setToAnimated(key_type id) {
      auto i = cached.find(id);
      if (i == cached.end()) {
        ERROR("{} texture not found when trying to set animation", m_name);
        return;
      }
      i->second->isAnimated = true;
    }

  template <typename key_type>
    void TextureCache<key_type>::addTexture(key_type id, GLuint texId) {
      /*
         std::map<uint8_t, texTuple*>::iterator i = cached.find(id);
         if (i == cached.end())
         return;*/
      auto* tt = new texTuple();
      tt->texId = texId;
      tt->refCount = 1;
      tt->hasAlpha = false;
      tt->isAnimated = false;
      cached[id] = tt;
      DEBUG("{} GL texture {} added for key: {}", m_name, texId, int(id));
    }

  template <typename key_type>
    void TextureCache<key_type>::cacheQuery(key_type id, texTuple *pos) {
      has_cached_query = true;
      last_query_id = id;
      last_query_result = pos;
    }

  template <typename key_type>
    bool TextureCache<key_type>::matchingCachedQuery(key_type id) {
      return ((has_cached_query) && (id == last_query_id));
    }

  template <typename key_type>
    void TextureCache<key_type>::setClearMagic(uint32_t removeLesser) {
      clearMagic = removeLesser;
    }

  template <typename key_type>
    void TextureCache<key_type>::setMinClearElements(uint32_t minElements) {
      minClearElements = minElements;
    }

  template <typename key_type>
    unsigned int TextureCache<key_type>::instance_count = 0;


  template class TextureCache<uint8_t>;
  template class TextureCache<char>;
  template class TextureCache<uint16_t>;
  template class TextureCache<uint32_t>;
}
