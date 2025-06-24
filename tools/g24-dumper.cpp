#include <algorithm>
#include <cassert>
#include <iostream>

#include <fmt/base.h>

#ifdef _MSC_VER
#define SDL_MAIN_HANDLED
#endif

#include <physfs.h>

#include <SDL2/SDL.h>
#include <core/graphics-24bit.h>
#include <core/sprite-info.h>

#include <util/file-manager.h>

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
    memcpy(dst, rp, w * h * 4);
    SDL_UnlockSurface(s);
    return s;
}

namespace OpenGTA {
void dumpClut(const OpenGTA::Graphics24Bit &g24, const char *fname)
{
    assert(g24.pagedClutSize % 1024 == 0);
    const UInt32 num_pal = g24.paletteIndexSize / 2;
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
    SDL_Surface *s = SDL_CreateRGBSurface(0, num_pal, 256, 32, rmask, gmask, bmask, amask);
    SDL_LockSurface(s);
    unsigned char *dst = static_cast<unsigned char *>(s->pixels);

    for (UInt32 color = 0; color < 256; color++) {
        for (UInt32 pal_id = 0; pal_id < num_pal; pal_id++) {
            const auto clut_id = g24.palIndex[pal_id];
            const auto off = (65536 * (clut_id / 64)) + (4 * (clut_id % 64));

            std::copy_n(g24.rawClut + off + (color * 256), 3, dst);
            dst[3] = 0xff;
            dst += 4;
        }
    }
    SDL_UnlockSurface(s);
    SDL_SaveBMP(s, fname);
    SDL_FreeSurface(s);
}
} // namespace OpenGTA

void main_loop()
{
    SDL_Event event;
    while (true) {
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
        fmt::print(stderr, "Usage: {} G24_FILE [INDEX]\n", argv[0]);
        return 1;
    }

    const Util::PhysFSContext pfs(argv[0]);

    SDL_Init(SDL_INIT_VIDEO);
    int idx = 0;

    OpenGTA::Graphics24Bit graphics(argv[1]);
    dumpClut(graphics, "foo.bmp");
    if (argc > 2) {
        idx = strtol(argv[2], nullptr, 10);
    }
    const auto &sinfo = graphics.getSprite(idx);
    auto sbm = graphics.getSpriteBitmap(idx, -1, 0);
    SDL_Surface *image = get_image(sbm.data(), sinfo.w, sinfo.h);
    if (argc == 4)
        SDL_SaveBMP(image, argv[3]);
    else
        display_image(image);

    SDL_FreeSurface(image);
    SDL_Quit();

    return 0;
}
