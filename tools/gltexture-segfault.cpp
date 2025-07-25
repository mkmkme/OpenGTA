
#include <GL/gl.h>
#include <GL/glu.h>
#include <fmt/base.h>

int main(int argc, char *argv[])
{
    GLuint texture;
    glGenTextures(1, &texture);
    fmt::println("texture: {}", texture);
    return 0;
}
