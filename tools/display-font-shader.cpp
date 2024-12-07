#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>

#include <SDL_video.h>

// #include <GL/gl.h>
// #include <GL/glext.h>

#ifdef _MSC_VER
#define SDL_MAIN_HANDLED
#endif

#include <SDL2/SDL.h>
#include <core/font.h>
#include <fmt/base.h>
#include <fmt/format.h>
#include <glad/gl.h>

#include <util/file-manager.h>
#include <util/gui.h>
// #include <util/log.h>

#if 0
void main_loop(GUI::Label *label, GUI::Manager &manager, OpenGL::Screen &screen)
{
    SDL_Event event;
    while (true) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    return;
                default:
                    break;
            }
        }
    }
}
#endif

const std::string_view vertex_shader = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
out vec2 TexCoord;

void main()
{
    gl_Position = vec4(aPos, 1.0);
    TexCoord = aTexCoord;
}
)";

const std::string_view fragment_shader = R"(
#version 330 core
out vec4 FragColor;
in vec2 TexCoord;
uniform sampler2D texture1;

void main()
{
    FragColor = texture(texture1, TexCoord);
    // FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
)";

GLenum glCheckError_(const char *file, int line)
{
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR) {
        std::string error;
        switch (errorCode) {
            case GL_INVALID_ENUM:
                error = "INVALID_ENUM";
                break;
            case GL_INVALID_VALUE:
                error = "INVALID_VALUE";
                break;
            case GL_INVALID_OPERATION:
                error = "INVALID_OPERATION";
                break;
            case GL_STACK_OVERFLOW:
                error = "STACK_OVERFLOW";
                break;
            case GL_STACK_UNDERFLOW:
                error = "STACK_UNDERFLOW";
                break;
            case GL_OUT_OF_MEMORY:
                error = "OUT_OF_MEMORY";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                error = "INVALID_FRAMEBUFFER_OPERATION";
                break;
        }
        std::cout << error << " | " << file << " (" << line << ")" << std::endl;
    }
    return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void glVerifyShader_(int shader, std::string_view name, std::string_view file, int line)
{
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        std::string infoLog(512, '\0');
        glGetShaderInfoLog(shader, infoLog.size(), nullptr, infoLog.data());
        auto message = fmt::format("ERROR ({}:{}): {} shader compilation failed!\n{}", name, file, line, infoLog);
        fmt::println(stderr, "{}", message);
        throw std::runtime_error(message);
    }
}
#define glVerifyShader(shader) glVerifyShader_(shader, #shader, __FILE__, __LINE__)

void glVerifyProgram_(int program, std::string_view file, int line)
{
    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        std::string infoLog(512, '\0');
        glGetProgramInfoLog(program, infoLog.size(), nullptr, infoLog.data());
        auto message = fmt::format("ERROR ({}:{}): shader program linking failed!\n{}", file, line, infoLog);
        fmt::println(stderr, "{}", message);
        throw std::runtime_error(message);
    }
}
#define glVerifyProgram(program) glVerifyProgram_(program, __FILE__, __LINE__)

std::vector<UInt8> bitmap;
unsigned int glwidth = 0;
unsigned int glheight = 0;
namespace OpenGTA {
void dumpAs(OpenGTA::Font &font, const char * /*filename*/, size_t id)
{
    unsigned int len = font.chars[id].width;
    len *= font.charHeight;
    glwidth = font.chars[id].width;
    glheight = font.charHeight;
    font.palette.apply(len, font.chars[id].rawData.data(), font.workBuffer.data(), true);
    bitmap.resize(len * 4);
    memcpy(bitmap.data(), font.workBuffer.data(), len * 4);
}
} // namespace OpenGTA

#define countof(x) (sizeof(x) / sizeof(x[0]))

int main(int /*argc*/, char **argv)
{
    const Util::PhysFSContext pfs(argv[0]);

    // auto *fps_label = new GUI::Label({ .x = 5, .y = 50 }, "", "F_MTEXT.FON", 1);
    // fps_label->font.
    OpenGTA::Font font("F_MTEXT.FON");
    const auto id = 0;
    dumpAs(font, "out.bmp", id);
    fmt::print("glwidth: {}, glheight: {}, vector size: {}\n", glwidth, glheight, bitmap.size());
    for (size_t i = 0; i < bitmap.size(); i += 4) {
        fmt::print("bitmap[{}]: {}, {}, {}, {}\n", i / 4, bitmap[i], bitmap[i + 1], bitmap[i + 2], bitmap[i + 3]);
    }

    SDL_Init(SDL_INIT_VIDEO);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    auto *window = SDL_CreateWindow("Shader shenanigans", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    auto *context = SDL_GL_CreateContext(window);

    if (!gladLoadGL((GLADloadfunc) SDL_GL_GetProcAddress)) {
        fmt::println(stderr, "Failed to initialize GLAD");
        return 1;
    }

    glViewport(0, 0, 800, 600);

    auto vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const auto *vertex_shader_raw = vertex_shader.data();
    glShaderSource(vertexShader, 1, &vertex_shader_raw, nullptr);
    glCompileShader(vertexShader);
    glVerifyShader(vertexShader);

    auto fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const auto *fragment_shader_raw = fragment_shader.data();
    glShaderSource(fragmentShader, 1, &fragment_shader_raw, nullptr);
    glCompileShader(fragmentShader);
    glVerifyShader(fragmentShader);

    auto shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glVerifyProgram(shaderProgram);

    float windowWidth = 800.0f;
    float windowHeight = 600.0f;
    float xScale = glwidth / windowWidth;
    float yScale = glheight / windowHeight;

    // Set up vertex data and buffers and configure vertex attributes
    // clang-format off
    float vertices[] = {
        // positions          // texture coords
        xScale,  yScale, 0.0f,   1.0f, 0.0f, // top right
        xScale, -yScale, 0.0f,   1.0f, 1.0f, // bottom right
        -xScale, -yScale, 0.0f,   0.0f, 1.0f, // bottom left
        -xScale,  yScale, 0.0f,   0.0f, 0.0f  // top left
    };
    unsigned int indices[] = {  // note that we start from 0!
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
    };
    // clang-format on

    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    // Bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);
    // Texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) (3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Load and create a texture
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // load image, create texture and generate mipmaps
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, glwidth, glheight, 0, GL_RGBA, GL_UNSIGNED_BYTE, bitmap.data());
    glGenerateMipmap(GL_TEXTURE_2D);

    glCheckError();

    while (true) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    goto end;
                default:
                    break;
            }
        }

        // glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(VAO);
        // glDrawArrays(GL_TRIANGLES, 0, 3);
        glCheckError();
        glDrawElements(GL_TRIANGLES, countof(indices), GL_UNSIGNED_INT, (void *) 0);
        glCheckError();

        SDL_GL_SwapWindow(window);
    }

end:
    // glDeleteVertexArrays(1, &VAO);
    // glDeleteBuffers(1, &VBO);
    // glDeleteBuffers(1, &EBO);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glDeleteProgram(shaderProgram);

    SDL_GL_DeleteContext(context);
    SDL_Quit();

    return 0;
}
