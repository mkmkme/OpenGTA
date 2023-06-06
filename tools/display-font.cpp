#include "common_sdl_gl.h"
#include "gl_base.h"
#include "gl_font.h"
#include "gl_screen.h"
#include "gui.h"

#include <GL/gl.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_opengl.h>
#include <iostream>
#include <physfs.h>

// FIXME: gui.cpp needs that
float screen_gamma = 1.0f;

bool handleKeyPress(SDL_Keysym *keysym)
{
    switch (keysym->sym) {
        case SDLK_ESCAPE:
            return true;
        // FIXME: this doesn't seem to do anything
        case 'a':
            glEnable(GL_CULL_FACE);
            printf("backfaces culled\n");
            return false;
        case 'b':
            glDisable(GL_CULL_FACE);
            printf("all faces drawn\n");
            return false;
        default:
            return false;
    }
}

// returns true when need to quit
void drawScene(GUI::Label *label, GUI::Manager &manager, OpenGL::Screen &screen)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // ...

    // TODO: These two snippets of gl* calls are necessary. Figure out why.

    glEnable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 640, 0, 480, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    label->text = "HELLO 1234567890 Hello !";

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    manager.draw();

    glEnable(GL_DEPTH_TEST);

    SDL_GL_SwapWindow(screen.get());
}

void main_loop(GUI::Label *label, GUI::Manager &manager, OpenGL::Screen &screen)
{
    SDL_Event event;
    int paused = 0;

    while (true) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                // FIXME: WINDOWEVENT_FOCUS_{GAINED,LOST} doesn't work as I expected. How does it work exactly?
                case SDL_WINDOWEVENT_FOCUS_GAINED:
                    INFO("Unpaused");
                    paused = 0;
                    break;
                case SDL_WINDOWEVENT_FOCUS_LOST:
                    INFO("Paused");
                    paused = 1;
                    break;
                case SDL_KEYDOWN:
                    if (handleKeyPress(&event.key.keysym)) {
                        return;
                    }
                    break;
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                    screen.resize(event.window.data1, event.window.data2);
                    break;
                case SDL_QUIT:
                    return;
                default:
                    break;
            }
        }
        if (!paused) {
            drawScene(label, manager, screen);
        }
    }
}

int main(int argc, char *argv[])
{
    PHYSFS_init("mapview");
    PHYSFS_mount(PHYSFS_getBaseDir(), nullptr, 1);
    PHYSFS_mount("gtadata.zip", nullptr, 1);

    OpenGL::Screen screen {};
    screen.activate(640, 480);

    GUI::Manager gm {};
    SDL_Rect rect;
    rect.x = 5;
    rect.y = 50;
    // STREET1.FON
    // FIXME: pass the string as an argument. Maybe even be fancy and validate it before passing to Label
    auto *fps_label = new GUI::Label(rect, "", "F_MTEXT.FON", 1);
    gm.add(fps_label, 5);

    main_loop(fps_label, gm, screen);

    SDL_Quit();
    PHYSFS_deinit();
    std::cout << "Goodbye" << std::endl;

    return 0;
}
