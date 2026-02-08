#include "graphics/font-renderer-modern.h"

#ifdef OGTA_USE_MODERN_GL

// #include <algorithm>
#include <cstring>
// #include <iostream>
#include <vector>

#include <fmt/format.h>
#include <glm/gtc/matrix_transform.hpp>

// #include "util/image_loader.h"
#include "util/log.h"

namespace OpenGL {

const char *textVertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
out vec2 TexCoords;

uniform mat4 projection;

void main()
{
    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
    TexCoords = vertex.zw;
}
)";

const char *textFragmentShaderSource = R"(
#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D text;
uniform vec3 textColor;

void main()
{
    vec4 sampled = texture(text, TexCoords);
    // Simple alpha test
    if (sampled.a < 0.1)
        discard;
    color = vec4(textColor, 1.0) * sampled;
}
)";

FontRendererModern::FontRendererModern(const std::string &filename, uint16_t scale)
    : fontSource_(std::make_unique<OpenGTA::Font>(filename))
    , scale_(scale)
{
    // Compile Vertex Shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &textVertexShaderSource, NULL);
    glCompileShader(vertexShader);

    // Check Vertex Shader
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        ERROR("Vertex Shader Compilation Failed: {}", infoLog);
    }

    // Compile Fragment Shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &textFragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // Check Fragment Shader
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        ERROR("Fragment Shader Compilation Failed: {}", infoLog);
    }

    // Link Program
    shaderProgram_ = glCreateProgram();
    glAttachShader(shaderProgram_, vertexShader);
    glAttachShader(shaderProgram_, fragmentShader);
    glLinkProgram(shaderProgram_);

    // Check Linking
    glGetProgramiv(shaderProgram_, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram_, 512, nullptr, infoLog);
        ERROR("Shader Linking Failed: {}", infoLog);
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Configure VAO/VBO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

FontRendererModern::~FontRendererModern()
{
    glDeleteProgram(shaderProgram_);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    for (auto const &[key, val] : Characters) {
        glDeleteTextures(1, &val.TextureID);
    }
}

void FontRendererModern::loadCharacter(char c)
{
    if (Characters.contains(c))
        return;

    unsigned int w, h;
    const auto src = fontSource_->getCharacterBitmap(fontSource_->getIdByChar(c), &w, &h);

    unsigned int glwidth = 1;
    unsigned int glheight = 1;
    while (glwidth < w)
        glwidth <<= 1;
    while (glheight < h)
        glheight <<= 1;

    std::vector<uint8_t> dst(glwidth * glheight * 4, 0);
    const unsigned char *r = src.data();
    unsigned char *t = dst.data();

    for (unsigned int i = 0; i < h; i++) {
        memcpy(t, r, w * 4);
        t += glwidth * 4;
        r += w * 4;
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, glwidth, glheight, 0, GL_RGBA, GL_UNSIGNED_BYTE, dst.data());
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    Character character = {
        .TextureID = texture,
        .Size = glm::ivec2(w, h),
        .Bearing = glm::ivec2(0, 0),
        .Advance = float(fontSource_->getMoveWidth(c)) * 1.1f // Use logic from old renderer
    };
    Characters.insert(std::pair<char, Character>(c, character));
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

    glUseProgram(shaderProgram_);
    glUniform1i(glGetUniformLocation(shaderProgram_, "text"), 0);
    glUniform3f(glGetUniformLocation(shaderProgram_, "textColor"), color.x, color.y, color.z);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram_, "projection"), 1, GL_FALSE, &projection[0][0]);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    float appliedScale = scale * scale_; // Combine global scale with font internal scale

    for (char c : text) {
        loadCharacter(c);
        if (!Characters.contains(c)) {
            continue;
        }
        Character ch = Characters[c];

        if (c != ' ' && ch.TextureID != 0) {
            float xpos = x + (ch.Bearing.x * appliedScale);
            float ypos = y + (ch.Bearing.y * appliedScale);

            float w = ch.Size.x * appliedScale;
            float h = ch.Size.y * appliedScale;

            // Texture coords (uv) need to match the padded texture size
            unsigned int glwidth = 1;
            while (glwidth < ch.Size.x)
                glwidth <<= 1;
            unsigned int glheight = 1;
            while (glheight < ch.Size.y)
                glheight <<= 1;

            float uw = (float) ch.Size.x / glwidth;
            float vh = (float) ch.Size.y / glheight;

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
