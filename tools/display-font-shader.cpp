#include <stdexcept>
#include <string>
#include <string_view>

#include <SDL_video.h>

#include <fmt/base.h>

#ifdef _MSC_VER
#define SDL_MAIN_HANDLED
#endif

#include <SDL2/SDL.h>
#include <core/font.h>
#include <fmt/format.h>
#include <glad/glad.h>

#include <util/file-manager.h>

constexpr const char *vertex_shader = R"(
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

constexpr const char *fragment_shader = R"(
#version 330 core
out vec4 FragColor;
in vec2 TexCoord;
uniform sampler2D texture1;

void main()
{
    FragColor = texture(texture1, TexCoord);
}
)";

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

void glCheckError_(const char *file, int line)
{
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR) {
        fmt::print(stderr, "{} | {} ({})\n", glErrorToString(errorCode), file, line);
    }
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
        fmt::print(stderr, "{}\n", message);
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
        fmt::print(stderr, "{}\n", message);
        throw std::runtime_error(message);
    }
}
#define glVerifyProgram(program) glVerifyProgram_(program, __FILE__, __LINE__)

} // namespace

#define countof(x) (sizeof(x) / sizeof((x)[0]))

// We don't want to catch exceptions here, because we want to see the stack trace
int main(int /*argc*/, char **argv) // NOLINT(bugprone-exception-escape)
{
    const Util::PhysFSContext pfs(argv[0]);

    OpenGTA::Font font("F_MTEXT.FON");
    const auto id = 0;
    unsigned int glwidth = 0;
    unsigned int glheight = 0;
    const auto bitmap = font.getCharacterBitmap(id, &glwidth, &glheight);
    fmt::print("glwidth: {}, glheight: {}, vector size: {}\n", glwidth, glheight, bitmap.size());
    for (size_t i = 0; i < bitmap.size(); i += 4) {
        fmt::print("bitmap[{}]: {}, {}, {}, {}\n", i / 4, bitmap[i], bitmap[i + 1], bitmap[i + 2], bitmap[i + 3]);
    }

    SDL_Init(SDL_INIT_VIDEO);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    auto *window = SDL_CreateWindow("Shader shenanigans", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    auto *context = SDL_GL_CreateContext(window);

    if (!gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress)) {
        fmt::print(stderr, "Failed to initialize GLAD\n");
        return 1;
    }

    glViewport(0, 0, 800, 600);

#define CREATE_SHADER(name, type, source)        \
    auto(name) = glCreateShader(type);           \
    glShaderSource(name, 1, &(source), nullptr); \
    glCompileShader(name);                       \
    glVerifyShader(name)

    CREATE_SHADER(vertexShader, GL_VERTEX_SHADER, vertex_shader);
    CREATE_SHADER(fragmentShader, GL_FRAGMENT_SHADER, fragment_shader);

#undef CREATE_SHADER

    auto shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glVerifyProgram(shaderProgram);

    constexpr float windowWidth = 800.0f;
    constexpr float windowHeight = 600.0f;
    const float xScale = glwidth / windowWidth;
    const float yScale = glheight / windowHeight;

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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) 0); // NOLINT(performance-no-int-to-ptr,*-use-nullptr)
    glEnableVertexAttribArray(0);
    // Texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) (3 * sizeof(float))); // NOLINT(performance-no-int-to-ptr)
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
                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_ESCAPE)
                        goto end;
                default:
                    break;
            }
        }

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glCheckError();
        glDrawElements(GL_TRIANGLES, countof(indices), GL_UNSIGNED_INT, (void *) 0); // NOLINT(performance-no-int-to-ptr,*-use-nullptr)
        glCheckError();

        SDL_GL_SwapWindow(window);
    }

end:
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glDeleteProgram(shaderProgram);

    SDL_GL_DeleteContext(context);
    SDL_Quit();

    return 0;
}
