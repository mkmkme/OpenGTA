// #include <stdlib.h>

#include <cstring>
#include <iostream>

#include <physfs.h>

#include <SDL2/SDL_surface.h>
#include <core/font.h>
#include <fmt/core.h>

#include <util/file-manager.h>
#include <util/log.h>

namespace OpenGTA {
void dumpAs(OpenGTA::Font &font, const char *filename, size_t id)
{
    INFO("Dumping font character {} to {}", id, filename);
    unsigned int width = 0;
    unsigned int height = 0;
    const auto bitmap = font.getCharacterBitmap(id, &width, &height);
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
#define rmask 0xff000000
#define gmask 0x00ff0000
#define bmask 0x0000ff00
#define amask 0x000000ff
#else
#define rmask 0x000000ff
#define gmask 0x0000ff00
#define bmask 0x00ff0000
#define amask 0xff000000
#endif
    SDL_Surface *s = SDL_CreateRGBSurface(0, width, height, 32, rmask, gmask, bmask, amask);
    SDL_LockSurface(s);
    memcpy(s->pixels, bitmap.data(), bitmap.size());
    SDL_UnlockSurface(s);
    SDL_SaveBMP(s, filename);
    SDL_FreeSurface(s);
}
} // namespace OpenGTA

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fmt::print(stderr, "USAGE: {} FONT_FILE CHAR_ID\n", argv[0]);
        return 1;
    }
    const Util::PhysFSContext pfs(argv[0]);
    std::cout << "Has: " << argv[1] << " : " << pfs.exists(argv[1]) << std::endl;
    OpenGTA::Font a(argv[1]);
    dumpAs(a, "out.bmp", strtol(argv[2], nullptr, 10));
    return 0;
}
