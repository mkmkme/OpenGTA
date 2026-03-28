#include "graphics/font-renderer-modern.h"

#ifdef OGTA_USE_MODERN_GL

#include <vector>

#include <glm/gtc/matrix_transform.hpp>

namespace OpenGL {

namespace {
constexpr float kAdvanceSpacingMultiplier = 1.1f;

size_t charIndex(char c)
{
    return static_cast<size_t>(static_cast<unsigned char>(c));
}
} // namespace

FontRendererModern::FontRendererModern(const std::string &filename, uint16_t scale)
    : fontSource_(std::make_unique<OpenGTA::Font>(filename))
    , shader_(std::make_unique<Shader>("data/shaders/font.vert", "data/shaders/font.frag"))
    , scale_(scale)
{
    // Configure VAO/VBO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

FontRendererModern::~FontRendererModern()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    for (const auto &ch : Characters) {
        if (ch.TextureID != 0) {
            glDeleteTextures(1, &ch.TextureID);
        }
    }
}

void FontRendererModern::loadCharacter(char c)
{
    auto &entry = Characters[charIndex(c)];
    if (entry.TextureID != 0)
        return;

    unsigned int w, h;
    const auto src = fontSource_->getCharacterBitmap(fontSource_->getIdByChar(c), &w, &h);

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, src.data());
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    entry.TextureID = texture;
    entry.Size = glm::ivec2(w, h);
    entry.Bearing = glm::ivec2(0, 0);
    entry.UV = glm::vec2(1.0f, 1.0f);
    entry.Advance = float(fontSource_->getMoveWidth(c)) * kAdvanceSpacingMultiplier;
}

void FontRendererModern::renderText(
    const std::string &text,
    float x,
    float y,
    float scale,
    const glm::vec3 &color,
    const glm::mat4 &projection
)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST); // Ensure text draws on top
    glDisable(GL_CULL_FACE);  // Disable culling for 2D text (projection flip might invert winding)

    shader_->use();
    shader_->setInt("text", 0);
    shader_->setVec3("textColor", color);
    shader_->setMat4("projection", projection);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    float appliedScale = scale * scale_; // Combine global scale with font internal scale

    for (char c : text) {
        loadCharacter(c);
        const auto &ch = Characters[charIndex(c)];
        if (ch.TextureID == 0) {
            continue;
        }

        if (c != ' ') {
            float xpos = x + (ch.Bearing.x * appliedScale);
            float ypos = y + (ch.Bearing.y * appliedScale);

            float w = ch.Size.x * appliedScale;
            float h = ch.Size.y * appliedScale;

            float uw = ch.UV.x;
            float vh = ch.UV.y;

            // Vertices for a quad [x, y, u, v]
            // Assuming top-left origin for screen and top-left for texture
            float vertices[6][4] = { { xpos, ypos + h, 0.0f, vh },  { xpos, ypos, 0.0f, 0.0f },
                                     { xpos + w, ypos, uw, 0.0f },

                                     { xpos, ypos + h, 0.0f, vh },  { xpos + w, ypos, uw, 0.0f },
                                     { xpos + w, ypos + h, uw, vh } };

            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glBindTexture(GL_TEXTURE_2D, ch.TextureID);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        x += (ch.Advance * appliedScale);
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
}

} // namespace OpenGL

#endif // OGTA_USE_MODERN_GL
