#ifdef OGTA_USE_MODERN_GL

#include "sprite-renderer-modern.h"

#include <fmt/format.h>
#include <glm/gtc/matrix_transform.hpp>

namespace OpenGL {

SpriteRendererModern::SpriteRendererModern() = default;

SpriteRendererModern::~SpriteRendererModern()
{
    if (spriteVao_ != 0) {
        glDeleteVertexArrays(1, &spriteVao_);
        glDeleteBuffers(1, &spriteVbo_);
        glDeleteBuffers(1, &spriteEbo_);
        spriteVao_ = 0;
        spriteVbo_ = 0;
        spriteEbo_ = 0;
    }

    if (lineVao_ != 0) {
        glDeleteVertexArrays(1, &lineVao_);
        glDeleteBuffers(1, &lineVbo_);
        lineVao_ = 0;
        lineVbo_ = 0;
    }
}

void SpriteRendererModern::init()
{
    // Load shaders
    spriteShader_ = std::make_unique<Shader>("data/shaders/sprite.vert", "data/shaders/sprite.frag");

    lineShader_ = std::make_unique<Shader>("data/shaders/line.vert", "data/shaders/line.frag");

    setupSpriteBuffers();
    setupLineBuffers();
}

void SpriteRendererModern::setupSpriteBuffers()
{
    // Quad vertices: position (x, y, z) + texcoords (u, v)
    // Note: The quad is centered at origin and lies on the XZ plane (y=0)
    float vertices[] = {
        // positions           // texture coords
        0.5f,  0.0f, 0.5f,  1.0f, 0.0f, // top right
        0.5f,  0.0f, -0.5f, 1.0f, 1.0f, // bottom right
        -0.5f, 0.0f, -0.5f, 0.0f, 1.0f, // bottom left
        -0.5f, 0.0f, 0.5f,  0.0f, 0.0f  // top left
    };

    unsigned int indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    glGenVertexArrays(1, &spriteVao_);
    glGenBuffers(1, &spriteVbo_);
    glGenBuffers(1, &spriteEbo_);

    glBindVertexArray(spriteVao_);

    glBindBuffer(GL_ARRAY_BUFFER, spriteVbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, spriteEbo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);

    // Texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) (3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void SpriteRendererModern::setupLineBuffers()
{
    // Line buffer will be updated dynamically, so we just create empty buffers
    glGenVertexArrays(1, &lineVao_);
    glGenBuffers(1, &lineVbo_);

    glBindVertexArray(lineVao_);
    glBindBuffer(GL_ARRAY_BUFFER, lineVbo_);

    // Position attribute only for lines
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void SpriteRendererModern::drawSprite(
    const PagedTexture &texture,
    const glm::vec3 &position,
    float rotation,
    float width,
    float height,
    const glm::mat4 &view,
    const glm::mat4 &projection
)
{
    spriteShader_->use();

    // Calculate model matrix with position, rotation, and scale
    glm::mat4 model = calculateModelMatrix(position, rotation, glm::vec3(width, 1.0f, height));

    // Set uniforms
    spriteShader_->setMat4("model", model);
    spriteShader_->setMat4("view", view);
    spriteShader_->setMat4("projection", projection);
    spriteShader_->setFloat("alphaThreshold", 0.0f); // matches glAlphaFunc(GL_GREATER, 0)
    spriteShader_->setInt("spriteTexture", 0);

    // Update VBO with correct texture coordinates from PagedTexture
    // Original immediate mode mapping:
    //   glTexCoord2f(texture.coords[0].u, texture.coords[1].v); glVertex3f(-w/2, 0, h/2);   // bottom-left
    //   glTexCoord2f(texture.coords[1].u, texture.coords[1].v); glVertex3f(w/2, 0, h/2);    // bottom-right
    //   glTexCoord2f(texture.coords[1].u, texture.coords[0].v); glVertex3f(w/2, 0, -h/2);   // top-right
    //   glTexCoord2f(texture.coords[0].u, texture.coords[0].v); glVertex3f(-w/2, 0, -h/2);  // top-left
    float vertices[] = {
        // positions           // texture coords
        0.5f,  0.0f, 0.5f,  texture.coords[1].u, texture.coords[1].v, // bottom right
        0.5f,  0.0f, -0.5f, texture.coords[1].u, texture.coords[0].v, // top right
        -0.5f, 0.0f, -0.5f, texture.coords[0].u, texture.coords[0].v, // top left
        -0.5f, 0.0f, 0.5f,  texture.coords[0].u, texture.coords[1].v  // bottom left
    };

    glBindVertexArray(spriteVao_);
    glBindBuffer(GL_ARRAY_BUFFER, spriteVbo_);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

    // Bind texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture.inPage);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Draw quad
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void SpriteRendererModern::drawBBox(
    const glm::vec3 &position,
    const glm::vec3 &extent,
    float rotation,
    const glm::mat4 &view,
    const glm::mat4 &projection
)
{
    lineShader_->use();

    // Calculate model matrix
    glm::mat4 model = calculateModelMatrix(position, rotation);

    lineShader_->setMat4("model", model);
    lineShader_->setMat4("view", view);
    lineShader_->setMat4("projection", projection);
    lineShader_->setVec3("lineColor", glm::vec3(1.0f, 1.0f, 0.0f)); // Yellow

    // Define bounding box vertices (two horizontal rectangles at y=0 and y=extent.y)
    // clang-format off
    float vertices[] = {
        // Bottom rectangle (y = 0)
        -extent.x,  0.0f,      extent.z,
         extent.x,  0.0f,      extent.z,
         extent.x,  0.0f,     -extent.z,
        -extent.x,  0.0f,     -extent.z,
        -extent.x,  0.0f,      extent.z,

        // Top rectangle (y = extent.y)
        -extent.x,  extent.y,  extent.z,
         extent.x,  extent.y,  extent.z,
         extent.x,  extent.y, -extent.z,
        -extent.x,  extent.y, -extent.z,
        -extent.x,  extent.y,  extent.z,
    };
    // clang-format on

    glBindVertexArray(lineVao_);
    glBindBuffer(GL_ARRAY_BUFFER, lineVbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    glDrawArrays(GL_LINE_STRIP, 0, 5); // Bottom rectangle
    glDrawArrays(GL_LINE_STRIP, 5, 5); // Top rectangle

    glBindVertexArray(0);
}

void SpriteRendererModern::drawTextureBorder(
    const glm::vec3 &position,
    float rotation,
    float width,
    float height,
    const glm::mat4 &view,
    const glm::mat4 &projection
)
{
    lineShader_->use();

    // Calculate model matrix
    glm::mat4 model = calculateModelMatrix(position, rotation);

    lineShader_->setMat4("model", model);
    lineShader_->setMat4("view", view);
    lineShader_->setMat4("projection", projection);
    lineShader_->setVec3("lineColor", glm::vec3(202.0f / 255.0f, 31.0f / 255.0f, 123.0f / 255.0f)); // Pink

    // Define texture border vertices (rectangle on XZ plane)
    float w = width / 2.0f;
    float h = height / 2.0f;
    // clang-format off
    float vertices[] = {
        -w, 0.0f,  h,
         w, 0.0f,  h,
         w, 0.0f, -h,
        -w, 0.0f, -h,
        -w, 0.0f,  h,
    };
    // clang-format on

    glBindVertexArray(lineVao_);
    glBindBuffer(GL_ARRAY_BUFFER, lineVbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    glDrawArrays(GL_LINE_STRIP, 0, 5);

    glBindVertexArray(0);
}

glm::mat4 SpriteRendererModern::calculateModelMatrix(const glm::vec3 &pos, float rot, const glm::vec3 &scale)
{
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, pos);
    model = glm::rotate(model, glm::radians(rot), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, scale);
    return model;
}

} // namespace OpenGL

#endif // OGTA_USE_MODERN_GL
