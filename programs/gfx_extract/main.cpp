/************************************************************************
 * Copyright (c) 2005-2007 tok@openlinux.org.uk                          *
 *                                                                       *
 * This software is provided as-is, without any express or implied       *
 * warranty. In no event will the authors be held liable for any         *
 * damages arising from the use of this software.                        *
 *                                                                       *
 * Permission is granted to anyone to use this software for any purpose, *
 * including commercial applications, and to alter it and redistribute   *
 * it freely, subject to the following restrictions:                     *
 *                                                                       *
 * 1. The origin of this software must not be misrepresented; you must   *
 * not claim that you wrote the original software. If you use this       *
 * software in a product, an acknowledgment in the product documentation *
 * would be appreciated but is not required.                             *
 *                                                                       *
 * 2. Altered source versions must be plainly marked as such, and must   *
 * not be misrepresented as being the original software.                 *
 *                                                                       *
 * 3. This notice may not be removed or altered from any source          *
 * distribution.                                                         *
 ************************************************************************/
#include <iostream>

#ifdef _MSC_VER
#define SDL_MAIN_HANDLED
#endif

#include <SDL.h>
#include <SDL_video.h>

#include <fmt/format.h>

#include <cxxopts.hpp>

#ifdef DUMP_DELTA_DEBUG
#include <fcntl.h>

#include <sys/stat.h>
#include <sys/types.h>
#endif

#include <core/active-style.h>
#include <core/graphics-base.h>
#include <core/sprite-info.h>

#include <util/errors.h>
#include <util/file-manager.h>
#include <util/set.h>

SDL_Surface *image = nullptr;

void display_image(SDL_Surface *s)
{
    SDL_Window *window = SDL_CreateWindow("Display Image", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, s);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);
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

SDL_Surface *get_image(std::span<const UInt8> rp, unsigned int w, unsigned int h)
{
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
    SDL_Surface *s = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 32, rmask, gmask, bmask, amask);
    SDL_LockSurface(s);
    const auto *src = rp.data();
    auto *dst = static_cast<unsigned char *>(s->pixels);
    memcpy(dst, src, w * h * 4);
    SDL_UnlockSurface(s);
    return s;
}

int main(int argc, char *argv[])
{
    const Util::PhysFSContext pfs(argv[0]);
    std::string file;
    SDL_Init(SDL_INIT_VIDEO);

    int c = 0;
    int mode = 0;
    int remap = -1;
    int delta = 0;
    Util::Set delta_as_set(32, (unsigned char *) &delta);
    bool delta_set = false;
    bool rgba = true;

    int idx = 0;
    int section = 0;

    cxxopts::Options options { "gfx_extract", "Extract and display graphics from OpenGTA style files" };
    // clang-format off
    options.add_options()
        ("i,info", "Display information about the style file")
        ("d,display", "Display the image")
        ("e,extract", "Extract the image to out.bmp")
        ("s,section", "Section to extract (0=side, 1=lid, 2=aux, 3=sprite)", cxxopts::value<int>(section))
        ("x,index", "Index of the block to extract", cxxopts::value<int>(idx))
        ("f,file", "Style file to load", cxxopts::value<std::string>(file))
        ("r,remap", "Remap index to use", cxxopts::value<int>(remap))
        ("a,delta", "Delta index to use", cxxopts::value<int>(delta))
        ("A,delta-set", "Delta set to use", cxxopts::value<int>())
        ("help", "Print help and exit");
    // clang-format on

    try {
        auto result = options.parse(argc, argv);
        if (result.count("help")) {
            fmt::print("{}\n", options.help());
            fmt::print("Where section 0 are 'side blocks', 1 'lid blocks', 2 'aux blocks' and 3 are 'sprites'\n");
            fmt::print("You can apply remaps (--remap N) and deltas (--delta N) but the relative indices are\n");
            fmt::print("not always correct (== experimental).\n");
            return 0;
        }
        if (result.count("info")) {
            mode = 0;
        }
        if (result.count("display")) {
            mode |= 1;
        }
        if (result.count("extract")) {
            mode |= 2;
        }
        if (result.count("delta-set")) {
            delta_set = true;
            auto dset = result["delta-set"].as<int>();
            delta_as_set.set_item(dset, true);
        }
    } catch (const cxxopts::exceptions::exception &e) {
        fmt::print(stderr, "Error parsing options: {}\n", e.what());
        return 1;
    }

    if (file.empty()) {
        std::cerr << "Error: no data file selected" << std::endl;
        return 1;
    }
    if (!pfs.exists(file.data())) {
        std::cerr << "File does not exist in searchpath: " << file << std::endl;
        return 1;
    }
    try {
        // exception handling of the constructor doesn't work here; urgh...
        OpenGTA::ActiveStyle::Instance().load(file);
        OpenGTA::GraphicsBase &graphics = OpenGTA::ActiveStyle::Instance().get();
        if (delta_set)
            graphics.setDeltaHandling(true);

        if (!mode)
            return 0;

        switch (section) {
            case 0:
                graphics.getSide(idx, 0, rgba);
                image = get_image(graphics.getTmpBuffer(rgba), 64, 64);
                break;
            case 1:
                graphics.getLid(idx, 0, rgba);
                image = get_image(graphics.getTmpBuffer(rgba), 64, 64);
                break;
            case 2:
                graphics.getAux(idx, 0, rgba);
                image = get_image(graphics.getTmpBuffer(rgba), 64, 64);
                break;
            case 3:
                const auto &sprite = graphics.getSprite(idx);
                std::cout << "Sprite is " << int(sprite.w) << "x" << int(sprite.h) << " with "
                          << int(sprite.deltaCount) << " deltas" << std::endl;
                auto sbitmap = graphics.getSpriteBitmap(idx, remap, delta);
                image = get_image(sbitmap, sprite.w, sprite.h);
#ifdef DUMP_DELTA_DEBUG
                if (delta && !delta_set) {
                    std::cout << "dumping delta" << std::endl;
                    OpenGTA::GraphicsBase::DeltaInfo &dinfo = sprite->delta[delta - 1];
                    int dump_fd = open("delta.raw", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
                    write(dump_fd, dinfo.ptr, dinfo.size);
                    close(dump_fd);
                }
#endif
                break;
        }

        if (!image) {
            std::cerr << "Error: image pointer is NULL; aborting" << std::endl;
            return 1;
        }
        if (mode & 1) {
            display_image(image);
        }
        if (mode & 2) {
            SDL_SaveBMP(image, "out.bmp");
        }
    } catch (const Util::Exception &e) {
        std::cerr << "Exception occured: " << e.what() << std::endl;
        return 1;
    }

    if (image)
        SDL_FreeSurface(image);
    SDL_Quit();

    return 0;
}
