#include <iostream>
#include <memory>
#include <SDL_image.h>
#include <SDL_opengl.h>
#include <unistd.h>
#include <physfs.h>
#include "gl_screen.h"
#include "gl_pagedtexture.h"
#include "log.h"
#include "physfsrwops.h"
#include "m_exceptions.h"
#include "gui.h"
#include "timer.h"
#include "dataholder.h"
#include "gl_spritecache.h"
#include "m_exceptions.h"


GUI::Manager guiManager;

void on_exit() {
  SDL_Quit();
  PHYSFS_deinit();
  std::cout << "Goodbye" << std::endl;
}

void turn_anim_off(float) {
  GUI::AnimatedTextureObject * obj= (GUI::AnimatedTextureObject *)guiManager.findObject(2);
  obj->animation->set(obj->animation->get(), Util::Animation::STOP);
  INFO("Stopped animation");
}

void font_play(float b) {
  //INFO << b << std::endl;
  GUI::Object * obj = guiManager.findObject(5);
  obj->color.r = obj->color.g = obj->color.b = obj->color.unused = Uint8((1.0f - b) * 255);
}

void run_init(OpenGL::Screen & screen) {
  PHYSFS_init("ogta");
  PHYSFS_mount(PHYSFS_getBaseDir(), nullptr, 1);
  PHYSFS_mount("gtadata.zip", nullptr, 1);
  if (getenv("OGTA_MOD"))
    PHYSFS_mount(getenv("OGTA_MOD"), nullptr, 0);
  screen.activate(arg_screen_w, arg_screen_h);
  screen.setSystemMouseCursor(true);
  glEnable(GL_ALPHA_TEST);
  glAlphaFunc(GL_GREATER, 0);

  OpenGTA::ActiveStyle::Instance().load("STYLE001.G24");

  SDL_EnableKeyRepeat( 100, SDL_DEFAULT_REPEAT_INTERVAL );

  std::vector<uint16_t> frame_nums(8);
  for (int i = 0; i < 8; ++i) {
    guiManager.cacheImageRAW("F_LOGO" + std::to_string(i) + ".RAW", 100+i);
    frame_nums[i] = 100+i;
  }
  guiManager.createAnimation(frame_nums, 5, 1);

  SDL_Rect r;
  r.x = 0; r.y = 0; r.h = 312 * screen.height() / 480; r.w = screen.width();
  std::string rawfile("F_LOWER1.RAW");
  guiManager.cacheImageRAW(rawfile, 99);
  GUI::TexturedObject * b2 = new GUI::TexturedObject(1, r, 99);
  guiManager.add(b2, 2);
  r.y = 312 * screen.height() / 480;
  r.h = 168 * screen.height() / 480;
  GUI::AnimatedTextureObject * b3 = new GUI::AnimatedTextureObject(2, r, 1);
  guiManager.add(b3, 2);

  Timer & t = Timer::Instance();


  ImageUtil::WidthHeightPair whp = guiManager.cacheStyleArrowSprite(16, -1);
  guiManager.cacheStyleArrowSprite(17, -1);
  std::vector<uint16_t> anim2frames(2);
  anim2frames[0] = 16;
  anim2frames[1] = 17;
  guiManager.createAnimation(anim2frames, 10, 2);
  r.x = 200;
  r.y = 200;
  r.w = whp.first * 2; r.h = whp.second * 2;
  GUI::AnimatedTextureObject * b5 = new GUI::AnimatedTextureObject(1, r, 2);
  guiManager.add(b5, 3);

  whp = guiManager.cacheStyleArrowSprite(2, -1);
  r.w = whp.first;
  r.h = whp.second;
  GUI::Pager * pager = new GUI::Pager(3, r, 2, "STREET1.FON", 1);

  guiManager.add(pager, 5);

  /*Timer::CallbackType cmd(turn_anim_off);
  t.registerCallback(false, cmd, t.getRealTime() + 2000);

  Timer::CallbackType cmd2(font_play);
  t.registerCallback(false, cmd2, t.getRealTime() + 100, t.getRealTime() + 3000);
  */
}

void handleKeyPress( SDL_keysym *keysym ) {
  switch ( keysym->sym ) {
    case SDLK_ESCAPE:
      global_Done = 1;
      break;
    default:
      break;
  }
}

void draw_menu(OpenGL::Screen & screen) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  screen.setFlatProjection();
  glDisable(GL_DEPTH_TEST);

  guiManager.draw();


  glEnable(GL_DEPTH_TEST);

  SDL_GL_SwapBuffers();
}

void run_main(OpenGL::Screen & screen) {
  SDL_Event event;
  Timer & t = Timer::Instance();
  t.update();
  //Uint32 now_ticks = SDL_GetTicks();
  Uint32 now_ticks = t.getRealTime();
  while(!global_Done) {
    while (SDL_PollEvent(&event)) {
      switch(event.type) {
        case SDL_KEYDOWN:
          handleKeyPress(&event.key.keysym);
          break;
        case SDL_MOUSEBUTTONDOWN:
          guiManager.receive(event.button);
          break;
          /*case SDL_KEYUP:
            handleKeyUp(&event.key.keysym);
            break;*/
        case SDL_VIDEORESIZE:
          screen.resize(event.resize.w, event.resize.h);
          break;
        case SDL_QUIT:
          global_Done = 1;
          break;
        default:
          break;
      }
    }
    draw_menu(screen);
    //now_ticks = SDL_GetTicks();
    t.update();
    now_ticks = t.getRealTime(); 
    guiManager.update(now_ticks);
    SDL_Delay(20);
  }
  // otherwise only at exit, which... troubles loki::smallobject
  guiManager.clearCache();
  Timer::Instance().clearAllEvents();
}

int main(int argc, char* argv[]) {
  atexit(on_exit);
  OpenGL::Screen screen {};
  run_init(screen);
  run_main(screen);
  return 0;
}
