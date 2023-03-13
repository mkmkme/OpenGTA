#include "graphics-24bit.h"
#include "sprite-info.h"

#include <SDL2/SDL.h>
#include <cassert>
#include <iostream>
#include <physfs.h>

SDL_Surface *get_image(unsigned char *rp, unsigned int w, unsigned int h)
{
    assert(rp);
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
    SDL_Surface *s = SDL_CreateRGBSurface(0, w, h, 32, rmask, gmask, bmask, amask);
    SDL_LockSurface(s);
    auto *dst = static_cast<unsigned char *>(s->pixels);
    for (auto i = 0u; i < w * h; ++i) {
        for (int j = 0; j < 4; ++j) {
            *dst = *rp;
            ++dst;
            ++rp;
        }
    }
    SDL_UnlockSurface(s);
    return s;
}

void OpenGTA::dumpClut(OpenGTA::Graphics24Bit &g24, const char* fname) {
    assert(g24.pagedClutSize % 1024 == 0);
    //PHYSFS_uint32 num_clut = pagedClutSize / 1024;
    const PHYSFS_uint32 num_pal = g24.paletteIndexSize / 2;
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
    SDL_Surface* s = SDL_CreateRGBSurface(0, num_pal, 256, 32, rmask, gmask, bmask, amask);
    SDL_LockSurface(s);
    unsigned char* dst = static_cast<unsigned char*>(s->pixels);

    for (PHYSFS_uint32 color = 0; color < 256; color++) {

        for (PHYSFS_uint32 pal_id = 0; pal_id < num_pal; pal_id++) {
        PHYSFS_uint32 clut_id = g24.palIndex[pal_id];
        PHYSFS_uint32 off = 65536 * (clut_id / 64) + 4 * (clut_id % 64);

        *dst = g24.rawClut[off+color*256];
        ++dst;
        *dst = g24.rawClut[off+color*256+1];
        ++dst;
        *dst = g24.rawClut[off+color*256+2];
        ++dst;
        *dst = 0xff;
        ++dst;
        }

    }
    SDL_UnlockSurface(s);
    SDL_SaveBMP(s, fname);
    SDL_FreeSurface(s);
}

void main_loop()
{
    SDL_Event event;
    while (1) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    return;
                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_ESCAPE)
                        return;
                default:
                    break;
            }
        }
        SDL_Delay(100);
    }
}

void display_image(SDL_Surface *s)
{
    auto *screen = SDL_CreateWindow("OpenGTA", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, 0);
    auto *renderer = SDL_CreateRenderer(screen, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    SDL_RenderPresent(renderer);

    auto *surTex = SDL_CreateTextureFromSurface(renderer, s);

    SDL_RenderClear(renderer);
    SDL_Rect dstrect { 0, 0, s->w, s->h };
    SDL_RenderCopy(renderer, surTex, nullptr, &dstrect);
    SDL_RenderPresent(renderer);

    main_loop();

    SDL_DestroyTexture(surTex);
    SDL_DestroyRenderer(renderer);
}

int main(int argc, char *argv[])
{
    if (argc < 2 || argc > 4) {
        std::cerr << "Usage: " << argv[0] << "G24_FILE [INDEX]" << std::endl;
        return 1;
    }

    PHYSFS_init(argv[0]);
    SDL_Init(SDL_INIT_VIDEO);
    int idx = 0;

    PHYSFS_mount(PHYSFS_getBaseDir(), nullptr, 1);
    PHYSFS_mount("gtadata.zip", nullptr, 1);

    OpenGTA::Graphics24Bit graphics(argv[1]);
    dumpClut(graphics, "foo.bmp");
    if (argc > 2)
        idx = atoi(argv[2]);
    auto *sinfo = graphics.getSprite(idx);
    auto sbm = graphics.getSpriteBitmap(idx, -1, 0);
    SDL_Surface *image = get_image(sbm.get(), sinfo->w, sinfo->h);
    if (argc == 4)
        SDL_SaveBMP(image, argv[3]);
    else
        display_image(image);

    SDL_FreeSurface(image);
    SDL_Quit();

    return 0;
}
