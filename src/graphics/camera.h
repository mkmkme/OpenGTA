#pragma once

#include <SDL2/SDL.h>
#include <glm/ext/vector_float3.hpp>

namespace OpenGL {

class Screen;

class Camera {
public:
    Camera();
    ~Camera() = default;

    Camera(const Camera &copy) = delete;
    Camera &operator=(const Camera &copy) = delete;
    Camera(Camera &&move) = delete;
    Camera &operator=(Camera &&move) = delete;

    void setSpeed(float new_speed); // positive is forward
    void setRotating(bool demo);
    void setCamGravity(bool demo);
    void rotateView(float x, float y, float z);
    void rotateAround(const glm::vec3 &lookAt, float x, float y, float z);
    void translateBy(const glm::vec3 &t);
    void translateTo(const glm::vec3 &e);
    void setVectors(const glm::vec3 &e, const glm::vec3 &c, const glm::vec3 &u);
    void moveByMouse(OpenGL::Screen &screen);
    void interpolate(const glm::vec3 &to, const Uint32 &start, const Uint32 &end);
    void setFollowMode(const glm::vec3 &target);
    void releaseFollowMode();

    void update(Uint32 ticks, OpenGL::Screen &screen);
    glm::vec3 &getEye() { return eye; }
    glm::vec3 &getCenter() { return center; }
    glm::vec3 &getUp() { return up; }
    [[nodiscard]] const glm::vec3 &getEye() const { return eye; }
    [[nodiscard]] const glm::vec3 &getCenter() const { return center; }
    [[nodiscard]] const glm::vec3 &getUp() const { return up; }

private:
    void update_game();
    glm::vec3 eye;
    glm::vec3 center;
    glm::vec3 up;
    float speed {};
    bool doRotate;
    bool camGravity;
    bool gameCamMode;
    glm::vec3 const *followTarget;

    glm::vec3 interpolateFrom;
    glm::vec3 interpolateTo;
    Uint32 interpolateStart;
    Uint32 interpolateEnd;
};
} // namespace OpenGL
