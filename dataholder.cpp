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
#include "dataholder.h"
#include "graphics-24bit.h"
#include "graphics-8bit.h"
#include "log.h"
#include "string_helpers.h"

#include <cassert>

namespace OpenGTA {

template<> ActiveStyle::~ActiveData() {
    unload();
  }

  template<> GraphicsBase & ActiveStyle::get() {
    if (m_data == nullptr)
      throw Util::NotSupported("Load a style-file first!");
    return *m_data;
  }

  template<class T> void ActiveData<T>::unload() {
    delete m_data;
    m_data = nullptr;
  }
  
  template<> void ActiveStyle::load(const std::string & file) {
    unload();
    std::string tempName { Util::string_lower(file) };
    if (tempName.find(".g24") != std::string::npos) {
      m_data = new Graphics24Bit(file);
    }
    else if (tempName.find(".gry") != std::string::npos) {
      m_data = new Graphics8Bit(file);
    }
    else {
      try {
        m_data = new Graphics8Bit(file);
      }
      catch (const Util::Exception & e) {
        WARN("loading 8 bit failed: {}", e.what());
        try {
          m_data = new Graphics24Bit(file);
        }
        catch (const Util::Exception & e) {
          ERROR("loading 24 bit failed: {}", e.what());
          m_data = nullptr;
        }
      }
    }
    assert(m_data);
  }

  template<> ActiveMap::~ActiveData() {
    unload();
  }
  
  template<> Map & ActiveMap::get() {
    if (!m_data)
      throw Util::NotSupported("Load a map-file first!");
    return *m_data;
  }
  
  template<> void ActiveMap::load(const std::string & file) {
    unload();
    try {
      m_data = new Map(file); 
    }
    catch (const Util::Exception & e) {
      ERROR("loading map failed: {}", e.what());
      m_data = nullptr;
    }
    assert(m_data);
  }

  template<> MainMsgLookup::~ActiveData() {
    unload();
  }
  
  template<> MessageDB & MainMsgLookup::get() {
    if (!m_data)
      throw Util::NotSupported("Load a message-file first!");
    return *m_data;
  }
  
  template<> void MainMsgLookup::load(const std::string & file) {
    unload();
    try {
      INFO("Trying to load: {}", file);
      m_data = new MessageDB(file); 
    }
    catch (const Util::Exception & e) {
      ERROR("loading message-db failed: {}", e.what());
      m_data = nullptr;
    }
    assert(m_data);
  }

}
