// #include <stdlib.h>

#include <iostream>

#include <physfs.h>

#include <SDL2/SDL_surface.h>
#include <core/font.h>

#include <util/file-manager.h>

void OpenGTA::dumpAs(OpenGTA::Font &font, const char *filename, size_t id)
{
    unsigned int len = font.chars[id]->width;
    len *= font.charHeight;
    font.palette.apply(len, font.chars[id]->rawData, font.workBuffer, true);
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
    SDL_Surface *s = SDL_CreateRGBSurface(0, font.chars[id]->width, font.charHeight, 32, rmask, gmask, bmask, amask);
    SDL_LockSurface(s);
    unsigned char *dst = static_cast<unsigned char *>(s->pixels);
    unsigned char *rp = font.workBuffer;
    for (unsigned int i = 0; i < len; i++) {
        *dst = *rp;
        ++dst;
        ++rp;
        *dst = *rp;
        ++dst;
        ++rp;
        *dst = *rp;
        ++dst;
        ++rp;
        //*dst = 0xff; ++dst;
        *dst = *rp;
        ++dst;
        ++rp;
    }
    SDL_UnlockSurface(s);
    SDL_SaveBMP(s, filename);
    SDL_FreeSurface(s);
}

int main(int argc, char *argv[])
{
    const Util::PhysFSContext pfs(argv[0]);
    std::cout << "Has: " << argv[1] << " : " << PHYSFS_exists(argv[1]) << std::endl;
    OpenGTA::Font a(argv[1]);
    dumpAs(a, "out.bmp", atoi(argv[2]));
    return 0;
}
