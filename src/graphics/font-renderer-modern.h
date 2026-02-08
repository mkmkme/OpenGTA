#pragma once

#ifdef OGTA_USE_MODERN_GL

#include <map>
#include <memory>
#include <string>

#include <glad/gl.h>
#include <glm/glm.hpp>

#include "core/font.h"

namespace OpenGL {

class FontRendererModern {
public:
    explicit FontRendererModern(const std::string &filename, uint16_t scale = 1);
    ~FontRendererModern();

    void renderText(
        const std::string &text,
        float x,
        float y,
        float scale,
        const glm::vec3 &color,
        const glm::mat4 &projection
    );

private:
    struct Character {
        GLuint TextureID;   // ID handle of the glyph texture
        glm::ivec2 Size;    // Size of glyph
        glm::ivec2 Bearing; // Offset from baseline to left/top of glyph
        float Advance;      // Offset to advance to next glyph
    };

    std::unique_ptr<OpenGTA::Font> fontSource_;
    std::map<char, Character> Characters;
    GLuint shaderProgram_;
    GLuint VAO, VBO;
    uint16_t scale_;

    void loadCharacter(char c);
};

} // namespace OpenGL

#endif // OGTA_USE_MODERN_GL
