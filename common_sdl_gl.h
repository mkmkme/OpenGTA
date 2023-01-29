#ifndef SDL_GL_COMMON_FUNCS_H
#define SDL_GL_COMMON_FUNCS_H
#include <SDL2/SDL.h>

int resize(int w, int h);
void initVideo(int w, int h, int bpp);
void initGL();
//void ERROR(const char* s);

//SDL_Surface* createRGBSurface(int w, int h);
//SDL_Surface* createRGBASurface(int w, int h);

extern int videoFlags;
extern int global_Done;

#endif
