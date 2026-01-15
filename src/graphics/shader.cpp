#ifdef OGTA_USE_MODERN_GL

#include "shader.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string_view>

#include <fmt/format.h>
#include <glm/gtc/type_ptr.hpp>

namespace OpenGL {

namespace {

std::string_view glErrorToString(GLenum errorCode)
{
    switch (errorCode) {
        case GL_INVALID_ENUM:
            return "INVALID_ENUM";
        case GL_INVALID_VALUE:
            return "INVALID_VALUE";
        case GL_INVALID_OPERATION:
            return "INVALID_OPERATION";
        case GL_STACK_OVERFLOW:
            return "STACK_OVERFLOW";
        case GL_STACK_UNDERFLOW:
            return "STACK_UNDERFLOW";
        case GL_OUT_OF_MEMORY:
            return "OUT_OF_MEMORY";
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            return "INVALID_FRAMEBUFFER_OPERATION";
        case GL_NO_ERROR:
            return "NO_ERROR";
        default:
            return "UNKNOWN_ERROR";
    }
}

void glVerifyShader(GLuint shader, std::string_view name, std::string_view file, int line)
{
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        std::string infoLog(512, '\0');
        glGetShaderInfoLog(shader, static_cast<GLsizei>(infoLog.size()), nullptr, infoLog.data());
        auto message = fmt::format("ERROR ({}:{}): {} shader compilation failed!\n{}", file, line, name, infoLog);
        fmt::print(stderr, "{}\n", message);
        throw std::runtime_error(message);
    }
}

void glVerifyProgram(GLuint program, std::string_view file, int line)
{
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        std::string infoLog(512, '\0');
        glGetProgramInfoLog(program, static_cast<GLsizei>(infoLog.size()), nullptr, infoLog.data());
        auto message = fmt::format("ERROR ({}:{}): shader program linking failed!\n{}", file, line, infoLog);
        fmt::print(stderr, "{}\n", message);
        throw std::runtime_error(message);
    }
}

} // anonymous namespace

Shader::Shader(const std::string &vertexPath, const std::string &fragmentPath)
{
    // Read shader source files
    std::string vertexSource = readFile(vertexPath);
    std::string fragmentSource = readFile(fragmentPath);

    // Compile shaders
    GLuint vertexShader = compileShader(vertexSource, GL_VERTEX_SHADER, vertexPath);
    GLuint fragmentShader = compileShader(fragmentSource, GL_FRAGMENT_SHADER, fragmentPath);

    // Link program
    program_ = linkProgram(vertexShader, fragmentShader);

    // Clean up shader objects (no longer needed after linking)
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

Shader::~Shader()
{
    if (program_ != 0) {
        glDeleteProgram(program_);
    }
}

void Shader::use() const
{
    glUseProgram(program_);
}

void Shader::setMat4(const std::string &name, const glm::mat4 &mat)
{
    GLint location = getUniformLocation(name);
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(mat));
}

void Shader::setVec3(const std::string &name, const glm::vec3 &vec)
{
    GLint location = getUniformLocation(name);
    glUniform3fv(location, 1, glm::value_ptr(vec));
}

void Shader::setFloat(const std::string &name, float value)
{
    GLint location = getUniformLocation(name);
    glUniform1f(location, value);
}

void Shader::setInt(const std::string &name, int value)
{
    GLint location = getUniformLocation(name);
    glUniform1i(location, value);
}

std::string Shader::readFile(const std::string &path)
{
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error(fmt::format("Failed to open shader file: {}", path));
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

GLuint Shader::compileShader(const std::string &source, GLenum type, const std::string &typeName)
{
    GLuint shader = glCreateShader(type);
    const char *sourceCStr = source.c_str();
    glShaderSource(shader, 1, &sourceCStr, nullptr);
    glCompileShader(shader);

    glVerifyShader(shader, typeName, __FILE__, __LINE__);

    return shader;
}

GLuint Shader::linkProgram(GLuint vertexShader, GLuint fragmentShader)
{
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    glVerifyProgram(program, __FILE__, __LINE__);

    return program;
}

GLint Shader::getUniformLocation(const std::string &name)
{
    // Check cache first
    auto it = uniformCache_.find(name);
    if (it != uniformCache_.end()) {
        return it->second;
    }

    // Query OpenGL and cache result
    GLint location = glGetUniformLocation(program_, name.c_str());
    if (location == -1) {
        fmt::print(stderr, "Warning: uniform '{}' not found in shader program\n", name);
    }
    uniformCache_[name] = location;

    return location;
}

} // namespace OpenGL

#endif // OGTA_USE_MODERN_GL
