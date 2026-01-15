#pragma once

#ifdef OGTA_USE_MODERN_GL

#include <memory>

#include <glad/gl.h>
#include <glm/glm.hpp>

#include "pagedtexture.h"
#include "shader.h"

namespace OpenGL {

class SpriteRendererModern {
public:
    SpriteRendererModern();
    ~SpriteRendererModern();

    SpriteRendererModern(const SpriteRendererModern &) = delete;
    SpriteRendererModern &operator=(const SpriteRendererModern &) = delete;

    void init();

    // Sprite rendering
    void drawSprite(
        const PagedTexture &texture,
        const glm::vec3 &position,
        float rotation,
        float width,
        float height,
        const glm::mat4 &view,
        const glm::mat4 &projection
    );

    // Debug visualization
    void drawBBox(
        const glm::vec3 &position,
        const glm::vec3 &extent,
        float rotation,
        const glm::mat4 &view,
        const glm::mat4 &projection
    );

    void drawTextureBorder(
        const glm::vec3 &position,
        float rotation,
        float width,
        float height,
        const glm::mat4 &view,
        const glm::mat4 &projection
    );

private:
    // Sprite rendering resources
    GLuint spriteVao_ { 0 };
    GLuint spriteVbo_ { 0 };
    GLuint spriteEbo_ { 0 };
    std::unique_ptr<Shader> spriteShader_;

    // Line rendering resources
    GLuint lineVao_ { 0 };
    GLuint lineVbo_ { 0 };
    std::unique_ptr<Shader> lineShader_;

    void setupSpriteBuffers();
    void setupLineBuffers();

    [[nodiscard]] static glm::mat4
    calculateModelMatrix(const glm::vec3 &pos, float rot, const glm::vec3 &scale = glm::vec3(1.0f));
};

} // namespace OpenGL

#endif // OGTA_USE_MODERN_GL
