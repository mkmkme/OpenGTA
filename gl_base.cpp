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
#include "gl_base.h"
#include <SDL2/SDL_opengl.h>


namespace OpenGL {

template<class T>
void ImmediateRenderer<T>::begin()
{
  }
  template<class T> void ImmediateRenderer<T>::end() {
  }

  template<> void ImmediateRenderer<Quad2Int>::draw(const Quad2Int & quad) {
    //assertCorrectPrimitive(quad.primitiveType);
    for (auto vertex : quad.vertices)
      glVertex2i(vertex[0], vertex[1]);
  }
  template<> void ImmediateRenderer<ColoredQuad2Int>::draw(const ColoredQuad2Int & quad) {
    //assertCorrectPrimitive(quad.primitiveType);
    for (int i = 0; i < 4; i++) {
      glColor3f(quad.colors[i][0], quad.colors[i][1], quad.colors[i][2]);
      glVertex2i(quad.vertices[i][0], quad.vertices[i][1]);
    }
  }
  template<> void ImmediateRenderer<FontQuad>::draw(const FontQuad & quad) {
    glBindTexture(GL_TEXTURE_2D, quad.texId);
    glBegin(OpenGL::FontQuad::primitiveType);
    for (int i = 0; i < 4; i++) {
      glTexCoord2f(quad.texCoords[i][0], quad.texCoords[i][1]);
      glVertex2i(quad.vertices[i][0], quad.vertices[i][1]);
    }
    glEnd();
  }

  /*
  template<> void ImmediateRenderer<OpenGTA::Map::BlockInfo>::draw(const OpenGTA::Map::BlockInfo & info) {
    for (int i = 0; i < 4; ++i) {
    }
  }
  */
}
