
#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>
#include <fmt/base.h>

int main()
{
    GLuint texture;
    glGenTextures(1, &texture);
    fmt::println("texture: {}", texture);
    return 0;
}
