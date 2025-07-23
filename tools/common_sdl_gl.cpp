
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_surface.h>

#include <GL/glu.h>

#include <util/log.h>

#ifndef VIEWGL_FOVY
#define VIEWGL_FOVY 60.0f
#endif
#ifndef VIEWGL_ZNEAR
#define VIEWGL_ZNEAR 0.1f
#endif
#ifndef VIEWGL_ZFAR
#define VIEWGL_ZFAR 250.0f
#endif

extern SDL_Surface *screen;
int videoFlags = 0;

int resize(int w, int h)
{
    GLfloat ratio;
    if (h == 0)
        h = 1;
    ratio = (GLfloat) w / (GLfloat) h;
    glViewport(0, 0, (GLint) w, (GLint) h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // glRotatef(180, 0, 0, 1);
    gluPerspective(VIEWGL_FOVY, ratio, VIEWGL_ZNEAR, VIEWGL_ZFAR);

    // glOrtho(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 300.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    return (1);
}

void initVideo(int w, int h, int bpp)
{
    SDL_DisplayMode videoInfo;
    SDL_Rect **modes;
    int i;

    if (SDL_GetCurrentDisplayMode(0, &videoInfo) != 0) {
        printf("VideoInfo query failed: %s\n", SDL_GetError());
        exit(1);
    }

    // SDL_ListModes is deprecated, switching to using SDL_GetNumDisplayModes and SDL_GetDisplayMode
    int mode_count = SDL_GetNumDisplayModes(0);
    if (mode_count < 1) {
        printf("No modes available!\n");
        exit(1);
    }

    printf("Available Modes %d\n", mode_count);
    for (i = 0; i < mode_count; ++i) {
        if (SDL_GetDisplayMode(0, i, &videoInfo) != 0) {
            printf("Could not get display mode for video display #%d: %s\n", i, SDL_GetError());
        } else {
            printf("%d: %d x %d\n", i, videoInfo.w, videoInfo.h);
        }
    }

    // if (modes == (SDL_Rect **) -1) {
    //     printf("All resolutions available.\n");
    // } else {
    //     printf("Available Modes %d\n", mode_count);
    //     for (i = 0; i < mode_count; ++i)
    //         printf("%d: %d x %d\n", i, modes[i]->w, modes[i]->h);
    // }

    videoFlags = SDL_WINDOW_OPENGL;
    videoFlags |= SDL_GL_DOUBLEBUFFER;
    // Removed SDL_HWPALETTE due to undeclared identifier
    // videoFlags |= SDL_RESIZABLE;
    // videoFlags |= SDL_FULLSCREEN;

    // if (videoInfo->hw_available) {
    //     INFO("Using HWSURFACE");
    //     videoFlags |= SDL_HWSURFACE;
    // } else {
    //     INFO("Using SWSURFACE");
    //     videoFlags |= SDL_SWSURFACE;
    // }
    videoFlags |= SDL_SWSURFACE;
    // if (videoInfo->blit_hw) {
    //     INFO("Using HWACCEL");
    //     videoFlags |= SDL_HWACCEL;
    // }
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    // screen = SDL_SetVideoMode(w, h, bpp, videoFlags);
    // if (!screen)
    //     ERROR("SDL failed to generate requested VideoSurface!");

    // resize(w, h);
}

void initGL()
{
    GLfloat LightAmbient[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    GLfloat LightDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat LightPosition[] = { 500.0f, 200.0f, 300.0f, 1.0f };

    // glEnable( GL_TEXTURE_2D );
    glShadeModel(GL_SMOOTH);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    // glClearDepth( 1.0f );
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    // glDepthFunc( GL_LEQUAL );
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glLightfv(GL_LIGHT0, GL_AMBIENT, LightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, LightDiffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, LightPosition);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glCullFace(GL_BACK);
    glPolygonMode(GL_FRONT, GL_FILL);
    glPolygonMode(GL_BACK, GL_LINE);
}

/*

SDL_Surface* createRGBASurface(int w, int h) {
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
  return SDL_CreateRGBSurface(SDL_SWSURFACE|SDL_SRCALPHA,
      64, 64, 32, rmask, gmask, bmask, amask);
}

SDL_Surface* createRGBSurface(int w, int h) {
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
  return SDL_CreateRGBSurface(SDL_SWSURFACE,
      w, h, 24, rmask, gmask, bmask, amask);
}
*/
