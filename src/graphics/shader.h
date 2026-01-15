#pragma once

#ifdef OGTA_USE_MODERN_GL

#include <string>
#include <unordered_map>

#include <glad/gl.h>
#include <glm/glm.hpp>

namespace OpenGL {

class Shader {
public:
    Shader(const std::string &vertexPath, const std::string &fragmentPath);
    ~Shader();

    // Delete copy, allow move
    Shader(const Shader &) = delete;
    Shader &operator=(const Shader &) = delete;
    Shader(Shader &&) noexcept = default;
    Shader &operator=(Shader &&) noexcept = default;

    void use() const;
    [[nodiscard]] GLuint getProgram() const noexcept { return program_; }

    // Uniform setters
    void setMat4(const std::string &name, const glm::mat4 &mat);
    void setVec3(const std::string &name, const glm::vec3 &vec);
    void setFloat(const std::string &name, float value);
    void setInt(const std::string &name, int value);

private:
    GLuint program_;
    std::unordered_map<std::string, GLint> uniformCache_;

    [[nodiscard]] static std::string readFile(const std::string &path);
    [[nodiscard]] static GLuint compileShader(const std::string &source, GLenum type, const std::string &typeName);
    [[nodiscard]] static GLuint linkProgram(GLuint vertexShader, GLuint fragmentShader);

    [[nodiscard]] GLint getUniformLocation(const std::string &name);
};

} // namespace OpenGL

#endif // OGTA_USE_MODERN_GL
