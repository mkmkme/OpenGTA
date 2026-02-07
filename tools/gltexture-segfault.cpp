
#ifdef _WIN32
#include <windows.h>
#endif

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include <fmt/base.h>

int main()
{
    GLuint texture;
    glGenTextures(1, &texture);
    fmt::println("texture: {}", texture);
    return 0;
}
