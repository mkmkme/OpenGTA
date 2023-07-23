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

// Prevent SDL from overriding main().
#define SDL_MAIN_HANDLED

#include <array>
#include <filesystem>
#include <iostream>
#include <iomanip>
#include <string_view>
#include <SDL2/SDL_opengl.h>
#include <cxxopts.hpp>
#include "blockanim.h"
#include "car-info.h"
#include "config.h"
#include "dataholder.h"
#include "entity_controller.h"
#include "file_helper.h"
#include "gl_camera.h"
#include "gl_cityview.h"
#include "gl_font.h"
#include "gl_screen.h"
#include "gl_spritecache.h"
#include "id_sys.h"
#include "localplayer.h"
#include "log.h"
#include "m_exceptions.h"
#include "navdata.h"
#include "spritemanager.h"
#include "lua_addon/lua_vm.h"
#include "lua_addon/lua_stackguard.h"
#include "gui.h"
#include "font_cache.h"
#include "ai.h"

int global_Done;
int global_Restart;
GLfloat mapPos[3] = {12.0f, 12.0f, 20.0f};

OpenGTA::CityView *city = nullptr;
GUI::Label * fps_label = nullptr;

GUI::AnimStatusDisplay* wantedLevel = nullptr;
GUI::Label*             cashLabel   = nullptr;

int city_num = 0;
constexpr std::array styles_8 = { "STYLE001.GRY", "STYLE002.GRY", "STYLE003.GRY" };
constexpr std::array styles_24 = { "STYLE001.G24", "STYLE002.G24", "STYLE003.G24" };
constexpr std::array cities = { "NYC.CMP", "SANB.CMP", "MIAMI.CMP" };
std::string specific_map;
std::string specific_style;

uint32_t num_frames_drawn = 0;
uint32_t last_tick;
uint32_t fps_last_tick;
uint32_t script_last_tick;
uint32_t arg_screen_w = 640;
uint32_t arg_screen_h = 480;
bool rotate = false;
bool cam_grav = false;
bool draw_arrows = false;
bool bbox_toggle = false;
bool texsprite_toggle = false;
bool follow_toggle = false;
float anisotropic_filter_degree = 2.0f;
int mipmap_textures = -1;
int vsync_config = -1;
int city_blocks_area = -1;
int config_scale2x = 1;
OpenGTA::SpriteObject::Animation pedAnim(0, 0);
#ifdef OGTA_DEFAULT_GRAPHICS_G24
bool highcolor_data = true;
#else
bool highcolor_data = false;
#endif
bool full_screen = false;

std::string script_file;
int paused = 0;
bool gamma_slide = false;
float screen_gamma = 1.0f;

Vector3D test_dot(-1, -1, -1);

class OpenGTAViewer {
public:
  OpenGTAViewer();

  void init(std::string_view progname);
  void run();
  void quit();

  void screenGammaCallback(float v);

private:
  void handleKeyPress(SDL_Keysym *keysym);
  void createPedAt(const Vector3D& v);
  void showGammaConfig();

  GUI::Manager guiManager_;
  OpenGL::Screen screen_;
  OpenGL::Camera camera_;
  OpenGTA::Script::LuaVM luaVM_;
  OpenGTA::LocalPlayer localPlayer_;
};

OpenGTAViewer::OpenGTAViewer()
  : guiManager_ {}
  , screen_ {}
  , camera_ {}
  , luaVM_ { screen_, camera_ }
  , localPlayer_ {}
{
}


void OpenGTAViewer::quit() {
  SDL_Quit();
  delete city;
  PHYSFS_deinit();
  std::cout << "Goodbye" << std::endl;
}

void print_version_info() {
#define PRINT_FORMATED(spaces) std::setw(spaces) << std::left <<
#define PRINT_OFFSET PRINT_FORMATED(19)
  std::cout << PRINT_OFFSET "OpenGTA version:" << OGTA_VERSION_INFO << std::endl <<
  PRINT_OFFSET "Lua version:" << LUA_RELEASE << std::endl <<
  PRINT_OFFSET "sound support:" <<
#ifdef WITH_SOUND
  "yes" <<
#else
  "no" <<
#endif
  std::endl <<
  PRINT_OFFSET "SDL_image support:" <<
#ifdef WITH_SDL_IMAGE
  "yes" <<
#else
  "no" <<
#endif
  std::endl <<

  PRINT_OFFSET "vsync support:" <<
#ifdef HAVE_SDL_VSYNC
  "yes" <<
#else
  "no" <<
#endif
  std::endl <<
  PRINT_OFFSET "scale2x support:" <<
#ifdef DO_SCALE2X
  "yes" <<
#else
  "no" <<
#endif
  std::endl <<

#ifdef OGTA_DEFAULT_DATA_PATH
  PRINT_OFFSET "data-path:" << "[" OGTA_DEFAULT_DATA_PATH "]" << std::endl <<
#endif
#ifdef OGTA_DEFAULT_MOD_PATH
  PRINT_OFFSET "mod-path:" << "[" OGTA_DEFAULT_MOD_PATH "]" << std::endl <<
#endif
  PRINT_OFFSET "default graphics:" <<
#ifdef OGTA_DEFAULT_GRAPHICS_G24
  "G24 - 24 bit" <<
#else
  "GRY - 8 bit" <<
#endif
  std::endl;
}

namespace {

void create_ingame_gui(GUI::Manager &gm, OpenGL::Screen &screen)
{
  assert(!wantedLevel);
  {
    SDL_Rect r;
    r.h = 32;
    r.x = screen.width() / 2 - 50;
    r.y = screen.height() - r.h;
    r.w = 100;
    SDL_Rect rs;
    rs.x = rs.y = 0;
    rs.w = rs.h = 16;
    gm.cacheStyleArrowSprite(16, -1);
    gm.cacheStyleArrowSprite(17, -1);
    std::vector<uint16_t> anim2frames(2);
    anim2frames[0] = 16;
    anim2frames[1] = 17;
    gm.createAnimation(anim2frames, 10, 2);
    wantedLevel = new GUI::AnimStatusDisplay(GUI::WANTED_LEVEL_ID, r, rs, 2);
    /*
    wantedLevel->borderColor.r = wantedLevel->borderColor.g = wantedLevel->borderColor.b = wantedLevel->borderColor.unused = 255;
    wantedLevel->drawBorder = 1;
    */
    gm.add(wantedLevel, 50);
  }
  assert(!cashLabel);
  {
    SDL_Rect r;
    r.x = screen.width() - 5;
    r.y = screen.height() - 30;
    cashLabel = new GUI::Label(GUI::CASH_ID, r, "0", "F_MTEXT.FON", 1);
    cashLabel->align = 1;
    gm.add(cashLabel, 50);
  }
}

void update_ingame_gui_values(OpenGTA::LocalPlayer &pc) {
  if (wantedLevel)
    wantedLevel->number = pc.getWantedLevel();

  if (cashLabel)
    cashLabel->text = std::to_string(pc.getCash());
}

void remove_ingame_gui(GUI::Manager &gm) {
  if (wantedLevel)
    gm.remove(wantedLevel);
  wantedLevel = nullptr;

  if (cashLabel)
    gm.remove(cashLabel);
  cashLabel = nullptr;
}

}

void parse_args(int argc, char *argv[])
{
    cxxopts::Options options { "viewer", "Demo program for OpenGTA" };
    // clang-format off
    options.positional_help("[CITY_NUMBER]").add_options()
        ("s", "Path to Lua script to execute", cxxopts::value<std::string>(script_file))
        ("a", "Anisotropic filter degree: 1.0 = disabled", cxxopts::value<float>(anisotropic_filter_degree))
        ("c", "Color mode: 0 = 8bit GRY, 1 = 24bit G24", cxxopts::value<bool>(highcolor_data))
        ("m", "Load specified map", cxxopts::value<std::string>(specific_map))
        ("M", "Texture mipmaps: 0 = disable, 1 = enable", cxxopts::value<int>(mipmap_textures))
        ("g", "Load specified style", cxxopts::value<std::string>(specific_style))
        ("w", "Set screen width", cxxopts::value<uint32_t>(arg_screen_w))
        ("h", "Set screen height", cxxopts::value<uint32_t>(arg_screen_h))
        ("l", "Log level (default: 2; 0, 1, 2, 3)", cxxopts::value<int>()->default_value("2"))
        ("f", "Fullscreen mode on start", cxxopts::value<bool>(full_screen))
        ("v", "Vertical sync: 0 = disable, 1 = try with SDL", cxxopts::value<int>(vsync_config))
        ("V,version", "Print version and exit")
        ("x", "Scale2x sprites: 0 = disable, 1 = enable", cxxopts::value<int>(config_scale2x))
        ("help", "Print help and exit")
        ;
    // clang-format on

    options.parse_positional({"city"});

    try {
      auto result = options.parse(argc, argv);
      if (result.count("help")) {
          fmt::print("{}", options.help());
          exit(0);
      }
      if (result.count("version")) {
          print_version_info();
          exit(0);
      }
      if (result.count("l")) {
          auto log_level = result["l"].as<int>();
          switch (log_level) {
              case 0:
                  Util::Log::setOutputLevel(Util::LogLevel::error);
                  break;
              case 1:
                  Util::Log::setOutputLevel(Util::LogLevel::warn);
                  break;
              case 2:
                  Util::Log::setOutputLevel(Util::LogLevel::info);
                  break;
              case 3:
                  Util::Log::setOutputLevel(Util::LogLevel::debug);
                  break;
              default:
                  fmt::print(stderr, "Invalid log level, falling back to info");
                  Util::Log::setOutputLevel(Util::LogLevel::info);
                  break;
          }
      }
      if (result.count("city")) {
          city_num = result["city"].as<int>();
      }
    } catch (const cxxopts::exceptions::exception& e) {
        fmt::print(stderr, "Error parsing options: {}\n", e.what());
        exit(1);
    }
}

namespace {
  void setGamma(SDL_Window * w, float v) {
    Uint16 ramp[256];
    for (int i = 0; i < 256; i++) {
        auto f = static_cast<float>(i) / 255.0f;
        f = pow(f, 1.0f / v);
        ramp[i] = Uint16(f) * 65535;
    }
    SDL_SetWindowGammaRamp(w, ramp, ramp, ramp);
  }
}

void OpenGTAViewer::screenGammaCallback(float v) {
  screen_gamma = v;
  setGamma(screen_.get(), v);
  lua_State *L = luaVM_.getInternalState();
  int top = lua_gettop(L);
  lua_getglobal(L, "config");
  if (lua_type(L, -1) != LUA_TTABLE) {
    lua_pop(L, 1);
    lua_newtable(L);
    lua_pushvalue(L, -1);
    lua_setglobal(L, "config");
  }
  uint8_t sf = OpenGTA::ActiveStyle::Instance().get().getFormat();
  if (sf)
    luaVM_.setFloat("screen_gamma_g24", v);
  else
    luaVM_.setFloat("screen_gamma_gry", v);
  lua_settop(L, top);
  GUI::Object * o = guiManager_.findObject(GUI::GAMMA_LABEL_ID);
  if (auto l = dynamic_cast<GUI::Label*>(o))
    l->text = "Gamma: " + std::to_string(v);
  /*
  Object * o2 = Manager::Instance().findObject(1001);
  if (o2) {
    static_cast<ImageStatusDisplay*>(o2)->number = screen_gamma * 3;
  }*/

}

void OpenGTAViewer::init(std::string_view progname) {
  // physfs
  PHYSFS_init(progname.data());

  // physfs-ogta
  const auto &data_path = Util::FileHelper::BaseDataPath();
  if (std::filesystem::exists(data_path))
    PHYSFS_mount(data_path.c_str(), nullptr, 1);
  else
    WARN("Could not load data-source: {}", data_path);

  PHYSFS_mount(PHYSFS_getBaseDir(), nullptr, 1);

  const auto &mod_path = Util::FileHelper::ModDataPath();
  if (std::filesystem::exists(mod_path))
    PHYSFS_mount(mod_path.c_str(), nullptr, 0);

  // check for a configfile
  if (PHYSFS_exists("config")) {
    const auto config_as_string = Util::FileHelper::BufferFromVFS(
      Util::FileHelper::OpenReadVFS("config"));
    
    try {
      //vm.runString(config_as_string);
      lua_State *L = luaVM_.getInternalState();
      Util::LGUARD(L);
      if (luaL_loadbuffer(L, config_as_string.c_str(), config_as_string.size(), "config"))
        throw E_SCRIPTERROR("Error running string: " + std::string(lua_tostring(L, -1)));
      lua_newtable(L);
      lua_pushvalue(L, -1);
      // lua_setglobal(L, "config");
      lua_setfield(L, -2, "config");
      if (lua_pcall(L, 0, 0, 0))
        throw E_SCRIPTERROR("Error running string: " + std::string(lua_tostring(L, -1)));
    }
    catch (const Util::ScriptError & e) {
      std::cerr << "Error in config-file: " << e.what() << std::endl;
      exit(1);
    }

    lua_State *L = luaVM_.getInternalState();
    lua_getglobal(L, "config");
    if (lua_type(L, 1) == LUA_TTABLE) {
      luaVM_.tryGetBool("use_g24_graphics", highcolor_data);

      int tmpInt;
      if (luaVM_.tryGetInt("screen_width", tmpInt))
        arg_screen_w = tmpInt;
      if (luaVM_.tryGetInt("screen_height", tmpInt))
        arg_screen_h = tmpInt;
      if (luaVM_.tryGetInt("screen_vsync", tmpInt))
        screen_.setVSyncMode(static_cast<OpenGL::VSyncMode>(tmpInt));

      luaVM_.tryGetBool("full_screen", full_screen);

      float fov = screen_.fieldOfView();
      float np = screen_.nearPlane();
      float fp = screen_.farPlane();
      luaVM_.tryGetFloat("gl_field_of_view", fov);
      luaVM_.tryGetFloat("gl_near_plane", np);
      luaVM_.tryGetFloat("gl_far_plane", fp);
      screen_.setupGlVars(fov, np, fp);

      luaVM_.tryGetBool("gl_mipmap_textures", ImageUtil::mipmapTextures);

      bool tmpBool;
      if (luaVM_.tryGetBool("scale2x_sprites", tmpBool))
        OpenGL::SpriteCache::Instance().setScale2x(tmpBool);

      luaVM_.tryGetInt("active_area_size", city_blocks_area);
    }
    // can't check for gl-extensions now

  }
  //INFO << "AREA:: " << city_blocks_area << std::endl;


  // check both width & height defined
  if ((arg_screen_h && !arg_screen_w) || (!arg_screen_h && arg_screen_w)) {
    WARN("Invalid screen specified: {}x{} - using default",
         arg_screen_w,
         arg_screen_h);
    arg_screen_h = 640; arg_screen_w = 480;
  }
  
  // fullscreen before first video init; only chance to set it on win32
  screen_.setFullScreenFlag(full_screen);
  if (vsync_config != -1)
    screen_.setVSyncMode(OpenGL::VSyncMode{ static_cast<uint8_t>(vsync_config) });

  // create screen
  screen_.activate(arg_screen_w, arg_screen_h);

  // more setup; that requires an active screen
  lua_State *L = luaVM_.getInternalState();
  if (lua_type(L, 1) == LUA_TTABLE) {
    float tmpFloat;
    if (luaVM_.tryGetFloat("gl_anisotropic_textures", tmpFloat) &&
        ImageUtil::supportedMaxAnisoDegree >= tmpFloat)
      ImageUtil::supportedMaxAnisoDegree = tmpFloat;
    
    if (highcolor_data)
      luaVM_.tryGetFloat("screen_gamma_g24", screen_gamma);
    else
      luaVM_.tryGetFloat("screen_gamma_gry", screen_gamma);
    // FIXME: Find a replacement
    // SDL_SetGamma(v, v, v);
    WARN("SDL_SetGamma to be called!");
  }
  lua_settop(L, 0);
  if (ImageUtil::supportedMaxAnisoDegree >= anisotropic_filter_degree)
    ImageUtil::supportedMaxAnisoDegree = anisotropic_filter_degree;
  else
    WARN("Using filter degree {}, requested {} not supported",
         ImageUtil::supportedMaxAnisoDegree,
         anisotropic_filter_degree);

  switch(mipmap_textures) {
    case -1:
      break;
    case 0:
      ImageUtil::mipmapTextures = false;
      break;
    default:
      ImageUtil::mipmapTextures = true;
      break;
  }
  
  // before any graphics are loaded
  OpenGL::SpriteCache::Instance().setScale2x(config_scale2x);

  // FIXME: basic gui setup; should not be here
  SDL_Rect rect;
  rect.x = 5;
  rect.y = 50;
  fps_label = new GUI::Label(rect, "", "F_MTEXT.FON", 1);
  //fps_label->borderColor.r = fps_label->borderColor.unused = 200;
  guiManager_.add(fps_label, 5);
}


void print_position(OpenGL::Camera & camera) {
  Vector3D & v = camera.getCenter();
  Vector3D & e = camera.getEye();
  Vector3D & u = camera.getUp();
  if (!city->getViewMode()) {
  std::cout << cities[city_num] << ": " << city->getCurrentSector()->getFullName() << std::endl <<
  "camera.setCenter(" << v.x << ", " << v.y << ", " << v.z << ")" << std::endl <<
  "camera.setEye(" << e.x << ", " << e.y << ", " << e.z << ")" << std::endl <<
  "camera.setUp(" << u.x << ", " << u.y << ", " << u.z << ")" << std::endl <<
  "city_view:setVisibleRange(" << city->getVisibleRange() << ")" << std::endl <<
  "city_view:setTopDownView( false )" << std::endl;
  }
  else {
    GLfloat* cp = city->getCamPos();
    std::cout << cities[city_num] << ": " << city->getCurrentSector()->getFullName() << std::endl <<
    "city_view:setCamPosition(" << cp[0] << ", " << cp[1] << ", " << cp[2] << ")" << std::endl <<
    "city_view:setVisibleRange(" << city->getVisibleRange() << ")" << std::endl <<
    "city_view:setTopDownView( true )" << std::endl;

  }

}

void handleKeyUp(SDL_Keysym *keysym, OpenGTA::LocalPlayer & player) {
  switch (keysym->sym) {
    case 'j': player.getCtrl().releaseTurnLeft(); break;
    case 'l': player.getCtrl().releaseTurnRight(); break;
    case 'i': player.getCtrl().releaseMoveForward(); break;
    case 'k': player.getCtrl().releaseMoveBack(); break;
    case SDLK_LCTRL: player.getCtrl().setFireWeapon(false); break;
    default: break;
  }
}

// void draw_mapmode();

void OpenGTAViewer::createPedAt(const Vector3D& v) {
  OpenGTA::Pedestrian p(Vector3D(0.2f, 0.5f, 0.2f), v, 0xffffffff);
  p.remap = OpenGTA::ActiveStyle::Instance().get().getRandomPedRemapNumber();
  INFO("using remap: {}", p.remap);
  OpenGTA::Pedestrian & pr = OpenGTA::SpriteManager::Instance().add(p);
  pr.switchToAnim(1);
  localPlayer_.setCtrl(pr.m_control);
  create_ingame_gui(guiManager_, screen_);
}

void explode_ped() {
  try {
  OpenGTA::Pedestrian & ped = OpenGTA::SpriteManager::Instance().getPed(0xffffffff);
  Vector3D p(ped.pos);
  p.y += 0.2f;
  OpenGTA::SpriteManager::Instance().createExplosion(p);
  }
  catch (Util::UnknownKey & e) {
    WARN("Cannot place explosion - press F4 to switch to player-mode first!");
  }
}

void zoomToTrain(int k) {
/*
  OpenGTA::TrainSegment & ts = OpenGTA::SpriteManager::Instance().getTrainById(k);
  Vector3D p(ts.pos);
  p.y += 9;
  OpenGL::Camera::Instance().interpolate(p, 1, 30000);
*/

}

#include "cell_iterator.h"

namespace OpenGTA {
  void ai_step_fake(OpenGTA::Pedestrian *p) {
    try {
    OpenGTA::Pedestrian & pr = OpenGTA::SpriteManager::Instance().getPed(0xffffffff);
    float t_angle = Util::xz_angle(p->pos, pr.pos);
    //INFO << "dist " << Util::distance(p->pos, pr.pos) << std::endl;
    //INFO << "angle " << t_angle << std::endl;
    //INFO << "myrot: " << p->rot << std::endl;
    if (Util::distance(p->pos, pr.pos) > 3) {
      p->m_control.setTurnLeft(false);
      p->m_control.setTurnRight(false);
      if (t_angle > p->rot)
        p->m_control.setTurnLeft(true);
      else
        p->m_control.setTurnRight(true);
    }
    else {
      p->m_control.setMoveForward(true);
      int k = rand() % 5;
      if (k == 0) {
        p->m_control.setTurnLeft(false);
        p->m_control.setTurnRight(false);
      }
      else if (k == 1) {
        p->m_control.setTurnLeft(true);
        p->m_control.setTurnRight(false);
      }
      else if (k == 2) {
        p->m_control.setTurnLeft(false);
        p->m_control.setTurnRight(true);
      }
    }
    }
    catch (Util::UnknownKey & e) {
    }

  }
}

void add_auto_ped() {
  try {
  OpenGTA::Pedestrian & pr = OpenGTA::SpriteManager::Instance().getPed(0xffffffff);
  int id = OpenGTA::TypeIdBlackBox::Instance().requestId();
  Vector3D v(pr.pos);
  v.y += 0.9f;
  //INFO << v.x << " " << v.y << " " << v.z << std::endl;
  Sint16 remap = OpenGTA::ActiveStyle::Instance().get().getRandomPedRemapNumber();
  OpenGTA::Pedestrian p(Vector3D(0.2f, 0.5f, 0.2f), v, id, remap);
  OpenGTA::Pedestrian & pr2 = OpenGTA::SpriteManager::Instance().add(p);
  pr2.switchToAnim(1);
  INFO("now {} peds", OpenGTA::SpriteManager::Instance().getPeds().size());

  //pr2.m_control = &OpenGTA::nullAI;
  }
  catch (Util::UnknownKey & e) {
    WARN("Cannot place peds now - press F4 to switch to player-mode first!");
  }
}

void toggle_player_run(OpenGTA::LocalPlayer & player) {
  OpenGTA::PedController * pc = &player.getCtrl();
  if (!pc) {
    WARN("no player yet!");
    return;
  }
  if (!pc->getRunning()) 
    pc->setRunning(true);
  else
    pc->setRunning(false);
}

void OpenGTAViewer::showGammaConfig() {
  if (gamma_slide) {
  SDL_Rect r;

  r.x = screen_.width() / 2;
  r.y = screen_.height() / 2;
  r.w = 200;
  r.h = 30;

  auto * sb = new GUI::ScrollBar(GUI::GAMMA_SCROLLBAR_ID, r);
  sb->color.r = sb->color.g = sb->color.b = 180;
  sb->color.a = 255;
  sb->innerColor.r = 250;
  sb->value = screen_gamma/2;
  sb->changeCB = GUI::ScrollBar::SC_Functor([this](float v){ screenGammaCallback(v); });
  guiManager_.add(sb, 90);

  r.y += 40;
  auto *l = new GUI::Label(GUI::GAMMA_LABEL_ID,
                                 r,
                                 "Gamma: " + std::to_string(screen_gamma),
                                 "F_MTEXT.FON",
                                 1);
  guiManager_.add(l, 80);

  screen_.setSystemMouseCursor(true);

  }
  else {
    guiManager_.removeById(GUI::GAMMA_SCROLLBAR_ID);
    guiManager_.removeById(GUI::GAMMA_LABEL_ID);
    screen_.setSystemMouseCursor(false);
  }
}

void car_toggle(OpenGTA::LocalPlayer & player) {
  OpenGTA::Pedestrian & pped = player.getPed();
  Vector3D pos = pped.pos;
  auto &cars = OpenGTA::SpriteManager::Instance().getCars();
  float min_dist = 360;
  auto j = cars.end();
  for (auto it = cars.begin(); it != cars.end(); ++it) {
    if (float tmp_dist = Util::distance(pos, it->second.pos); tmp_dist < min_dist) {
      j = it;
      min_dist = tmp_dist;
    }
  }
  assert(j != cars.end());
  auto &car = j->second;
  std::cout << car.id() << " " << car.pos.x << ", " << car.pos.y << ", " << car.pos.z << std::endl;
  Vector3D p_door(car.carInfo.door[0].rpx / 64.0f, 0, car.carInfo.door[0].rpy / 64.0f);

  Vector3D p_door_global = Transform(p_door, car.m_M);
  p_door_global.y += 0.2f;
  std::cout << p_door_global.x << ", " << p_door_global.y << ", " << p_door_global.z << std::endl;
  test_dot = p_door_global;
  //pped.aiMode = 1;
  //pped.aiData.pos1 = p_door_global;
  OpenGTA::AI::Pedestrian::walk_pavement(&pped);

}

void draw_mapmode(OpenGL::Screen &);

void OpenGTAViewer::handleKeyPress(SDL_Keysym *keysym) {
  GLfloat* cp = city->getCamPos();
  mapPos[0] = cp[0]; mapPos[1] = cp[1]; mapPos[2] = cp[2];
  switch ( keysym->sym ) {
    case SDLK_ESCAPE:
      global_Done = 1;
      break;
    case SDLK_LEFT:
      mapPos[0] -= 1.0f;
      camera_.translateBy(Vector3D(-1, 0, 0));
      break;
    case SDLK_RIGHT:
      mapPos[0] += 1.0f;
      camera_.translateBy(Vector3D(1, 0, 0));
      break;
    case SDLK_UP:
      mapPos[2] -= 1.0f;
      camera_.translateBy(Vector3D(0, 0, -1));
      break;
    case SDLK_DOWN:
      mapPos[2] += 1.0f;
      camera_.translateBy(Vector3D(0, 0, 1));
      break;
    case SDLK_SPACE:
      camera_.setSpeed(0.0f);
      break;
    case SDLK_F2:
      bbox_toggle = !bbox_toggle;
      OpenGTA::SpriteManager::Instance().setDrawBBox(bbox_toggle);
      break;
    case SDLK_F3:
      texsprite_toggle = !texsprite_toggle;
      OpenGTA::SpriteManager::Instance().setDrawTexBorder(texsprite_toggle);
      break;
    case SDLK_F4:
      follow_toggle = !follow_toggle;
      if (follow_toggle) {
        // SDL_EnableKeyRepeat( 0, SDL_DEFAULT_REPEAT_INTERVAL );
        city->setViewMode(false);
        Vector3D p(camera_.getEye());
        createPedAt(p);
        camera_.setVectors( Vector3D(p.x, 10, p.z), Vector3D(p.x, 9.0f, p.z), Vector3D(0, 0, -1) );
        camera_.setFollowMode(OpenGTA::SpriteManager::Instance().getPed(0xffffffff).pos);
        camera_.setCamGravity(true);
      }
      else {
        // SDL_EnableKeyRepeat( 100, SDL_DEFAULT_REPEAT_INTERVAL );
        camera_.setVectors(camera_.getEye(), 
          Vector3D(camera_.getEye() + Vector3D(1, -1, 1)), Vector3D(0, 1, 0));
        camera_.setCamGravity(false);
        camera_.releaseFollowMode();
        OpenGTA::SpriteManager::Instance().removePed(0xffffffff);
        OpenGTA::SpriteManager::Instance().removeDeadPeds();
        remove_ingame_gui(guiManager_);
      }
      break;
    case SDLK_RETURN:
      car_toggle(localPlayer_);
      break;
    case SDLK_F5:
      draw_arrows = !draw_arrows;
      city->setDrawHeadingArrows(draw_arrows);
      break;
    case SDLK_F6:
      draw_mapmode(screen_);
      break;
    case SDLK_F7:
      explode_ped();
      break;
    case SDLK_F8:
      add_auto_ped();
      break;
    case SDLK_F9:
      city->setDrawTextured(!city->getDrawTextured());
      break;
    case SDLK_F10:
      city->setDrawLines(!city->getDrawLines());
      break;
    case SDLK_F12:
      gamma_slide = !gamma_slide;
      showGammaConfig();
      break;
    case SDLK_LSHIFT:
      toggle_player_run(localPlayer_);
      break;
    /*
    case SDLK_F6:
      tex_flip = !tex_flip;
      INFO << "flipping: " << tex_flip << std::endl;
      city->setTexFlipTest(tex_flip);
      break;
    */
    case SDLK_LCTRL:
      localPlayer_.getCtrl().setFireWeapon();
      break;
    case '1':
      localPlayer_.getCtrl().setActiveWeapon(1);
      break;
    case '2':
      localPlayer_.getCtrl().setActiveWeapon(2);
      break;
    case '3':
      localPlayer_.getCtrl().setActiveWeapon(3);
      break;
    case '4':
      localPlayer_.getCtrl().setActiveWeapon(4);
      break;
//    case '5':
//      //OpenGTA::SpriteManager::Instance().getPed(0xffffffff).equip(5);
//      break;
//    case '6':
//      //OpenGTA::SpriteManager::Instance().getPed(0xffffffff).equip(6);
//      break;
//    case '7':
//      //OpenGTA::SpriteManager::Instance().getPed(0xffffffff).equip(7);
//      break;
//    case '8':
//      //OpenGTA::SpriteManager::Instance().getPed(0xffffffff).equip(8);
//      break;
//    case '9':
//      //OpenGTA::SpriteManager::Instance().getPed(0xffffffff).equip(9);
      /*
      ped_anim -= 1; if (ped_anim < 0) ped_anim = 0;
      pedAnim.firstFrameOffset = ped_anim;
      INFO << "switching to sprite: " << ped_anim << std::endl;
      OpenGTA::SpriteManager::Instance().getPed(0xffffffff).setAnimation(pedAnim);
      */
      break;
    case '0':
      localPlayer_.getCtrl().setActiveWeapon(0);
      /*
      ped_anim += 1; if (ped_anim > 200) ped_anim = 200;
      pedAnim.firstFrameOffset = ped_anim;
      INFO << "switching to sprite: " << ped_anim << std::endl;
      OpenGTA::SpriteManager::Instance().getPed(0xffffffff).setAnimation(pedAnim);
      */
      //OpenGTA::SpriteManager::Instance().getPed(0xffffffff).equip(0);
      break;
    case 'w':
      camera_.setSpeed(0.2f);
      break;
    case 's':
      camera_.setSpeed(-0.2f);
      break;
    case 'j':
      localPlayer_.getCtrl().setTurnLeft();
      break;
    case 'l':
      localPlayer_.getCtrl().setTurnRight();
      break;
    case 'i':
      localPlayer_.getCtrl().setMoveForward();
      break;
    case 'k':
      localPlayer_.getCtrl().setMoveBack();
      break;
    case 'f':
//FIXME: simply ignored on windows for now
#ifndef _WIN32
      screen_.toggleFullscreen();
#endif
#if 0
#ifdef _WIN32
      city->resetTextures();
      //m_font->resetTextures();
      OpenGL::SpriteCache::Instance().clearAll();
#endif
#endif
      break;
    case 'r':
      rotate = !rotate;
      camera_.setRotating(rotate);
      break;
    case 'g':
      cam_grav = !cam_grav;
      camera_.setCamGravity(cam_grav);
      break;
    case 't':
      mapPos[0] = mapPos[2] = 128;
      mapPos[1] = 230;
      city->setVisibleRange(128);
      break;
    case 'p':
      print_position(camera_);
      break;
    case '+':
      mapPos[1] += 1.0f;
      camera_.translateBy(Vector3D(0, 1, 0));
      break;
    case '-':
      mapPos[1] -= 1.0f;
      camera_.translateBy(Vector3D(0, -1, 0));
      break;
    case 'x':
      city->setViewMode(false);
      city->setVisibleRange(city->getVisibleRange() * 2);
      break;
    case 'y':
      break;
    case 'z':
      city->setViewMode(true);
      city->setVisibleRange(city->getVisibleRange() / 2);
      break;
    case '.':
      city->setVisibleRange(city->getVisibleRange()-1);
      INFO(" new visible range {}", city->getVisibleRange());
      break;
    case ',':
      city->setVisibleRange(city->getVisibleRange()+1);
      INFO(" new visible range {}", city->getVisibleRange());
      break;
    case SDLK_PRINTSCREEN:
      screen_.makeScreenshot("screenshot.bmp"); 
      break;
    default:
      return; 
  }
  city->setPosition(mapPos[0], mapPos[1], mapPos[2]);

}

void drawScene(Uint32 ticks, GUI::Manager &manager, OpenGL::Screen &screen) {
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  screen.set3DProjection();
  city->draw(ticks);

  glColor3f(1, 0, 0);
  glDisable(GL_TEXTURE_2D);
  glBegin(GL_POINTS);
  glVertex3f(test_dot.x, test_dot.y, test_dot.z);
  glEnd();
  glEnable(GL_TEXTURE_2D);
  glColor3f(1, 1, 1);


  screen.setFlatProjection();
  glDisable(GL_DEPTH_TEST);
  
  glPushMatrix();
  glTranslatef(10, 10, 0);
  OpenGL::DrawableFont &m_font = OpenGTA::FontCache::Instance().getFont("F_MTEXT.FON", 1);
  m_font.drawString(city->getCurrentSector()->getFullName());
  glPopMatrix();
  
  /*glPushMatrix();
  glTranslatef(5, 50, 0);
  std::ostringstream strstr;
  strstr << fps << " fps";
  m_font->drawString(strstr.str());
  glPopMatrix();*/

  manager.draw();
 
  num_frames_drawn += 1;
  glEnable(GL_DEPTH_TEST);

  SDL_GL_SwapWindow(screen.get());
}

void draw_mapmode(OpenGL::Screen &screen) {
  SDL_Event event;
  OpenGL::PagedTexture map_tex = city->renderMap2Texture();
  bool done_map = false;
  screen.setSystemMouseCursor(true);
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glDisable(GL_DEPTH_TEST);
  INFO("{}", map_tex.coords[1].u);
  while(!done_map) {
    while (SDL_PollEvent(&event)) {
      switch(event.type) {
        case SDL_QUIT:
          done_map = true;
          break;
        case SDL_KEYDOWN:
          switch(event.key.keysym.sym) {
            case SDLK_ESCAPE:
              done_map = true;
              break;
            default:
              break;
          }
          break;
        case SDL_MOUSEBUTTONDOWN:
          INFO("{} {}",
               event.button.x  / 600.0f * 255,
               event.button.y / 600.0f * 255);
          mapPos[0] = event.button.x  / 600.0f * 255;
          mapPos[2] = event.button.y / 600.0f * 255;
          //mapPos[1] = 10;
          done_map = true;
          break;
        case SDL_MOUSEMOTION:
          std::cout << "Mouse move: x " << event.motion.x << " y " << 
            event.motion.y << std::endl;
          break;
        default:
          break;
      }
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    screen.setFlatProjection();
    glBindTexture(GL_TEXTURE_2D, map_tex.inPage);
    //glMatrixMode(GL_TEXTURE);
    //if (_scale < 1)
    //  glScalef(_scale, _scale, 1);


    uint32_t h = screen.height();
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0);
    glVertex2i(0, 0);
    glTexCoord2f(map_tex.coords[1].u, 0);
    glVertex2i(h, 0);
    glTexCoord2f(map_tex.coords[1].u, map_tex.coords[1].v);
    glVertex2i(h, h);
    glTexCoord2f(0, map_tex.coords[1].v);
    glVertex2i(0, h);
    glEnd();

    const OpenGTA::Map::LocationMap & lmap = OpenGTA::ActiveMap::Instance().get().getLocationMap();
    OpenGTA::Map::LocationMap::const_iterator i = lmap.begin();
    glDisable(GL_TEXTURE_2D);
    while (i != lmap.end()) {
      if (i->first == 2) {
        i++;
        continue;
      }
      // uint8_t l_type = i->first;
      float l_x, l_y;
      l_x = i->second.x / 255.0f * h;// *  map_tex.coords[1].u;
      l_y = i->second.y / 255.0f * h;// * map_tex.coords[1].u;
      //INFO << int(l_type) << ": " << l_x << " " << l_y << std::endl;
      glBegin(GL_LINE_STRIP);
      glVertex2f(l_x - 5, l_y - 5);
      glVertex2f(l_x + 5, l_y - 5);
      glVertex2f(l_x + 5, l_y + 5);
      glVertex2f(l_x - 5, l_y + 5);

      glEnd();
      ++i;
    }

    glEnable(GL_TEXTURE_2D);
    SDL_GL_SwapWindow(screen.get());
    SDL_Delay(20);

  }
  screen.setSystemMouseCursor(false);
  glEnable(GL_DEPTH_TEST);
  glMatrixMode(GL_TEXTURE);
  glLoadIdentity();
  // the texture class doesn't cleanup!
  glDeleteTextures(1, &map_tex.inPage); 
}

void OpenGTAViewer::run() {
  SDL_Event event;
  const char * lang = getenv("OGTA_LANG");
  if (!lang)
    lang = getenv("LANG");
  if (!lang)
    lang = "en";
  OpenGTA::MainMsgLookup::Instance().load(Util::FileHelper::Lang2MsgFilename(lang));

  //m_font = new OpenGL::DrawableFont();
  //m_font->loadFont("F_MTEXT.FON");
  //m_font->setScale(1);

  glEnable(GL_TEXTURE_2D);
  glPolygonMode(GL_FRONT, GL_FILL);
  glEnable(GL_CULL_FACE);

  //glEnable(GL_BLEND);
  //glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  glEnable(GL_ALPHA_TEST);
  glAlphaFunc(GL_GREATER, 2/255.0f);//0);

  city = new OpenGTA::CityView(screen_, camera_);
  if (!specific_map.empty() && !specific_style.empty()) {
    city->loadMap(specific_map, specific_style);
  }
  else {
    if (highcolor_data)
      city->loadMap(cities[city_num], styles_24[city_num]);
    else
      city->loadMap(cities[city_num], styles_8[city_num]);
  }
  if (city_blocks_area > -1)
    city->setVisibleRange(city_blocks_area);
  city->setPosition(mapPos[0], mapPos[1], mapPos[2]);

  //cam.setVectors( Vector3D(4, 10, 4), Vector3D(4, 0.0f, 4.0f), Vector3D(0, 0, -1) );
  camera_.setVectors( Vector3D(12, 20, 12), Vector3D(13.0f, 19.0f, 13.0f), Vector3D(0, 1, 0) );

#ifdef TIMER_OPENSTEER_CLOCK
  Timer & timer = Timer::Instance();
  timer.update();
  last_tick = timer.getRealTime();
  //timer.setSimulationRunning(true);
#else
  last_tick = SDL_GetTicks();
#endif

  luaVM_.setCityView(*city);
  luaVM_.setMap(OpenGTA::ActiveMap::Instance().get());
  if (!script_file.empty())
    luaVM_.runFile(script_file.c_str());
  bool vm_tick_ok = true;
  script_last_tick = last_tick;
  
  while(!global_Done) {
    while (SDL_PollEvent(&event)) {
      switch(event.type) {
        case SDL_WINDOWEVENT_FOCUS_GAINED:
          paused = 0;
          break;
        case SDL_WINDOWEVENT_FOCUS_LOST:
          paused = 1;
          break;
        case SDL_KEYDOWN:
          if (event.key.repeat == 0) {
            handleKeyPress(&event.key.keysym);
          }
          break;
        case SDL_KEYUP:
          handleKeyUp(&event.key.keysym, localPlayer_);
          break;
        case SDL_WINDOWEVENT_SIZE_CHANGED:
          screen_.resize(event.window.data1, event.window.data2);
          break;
        case SDL_QUIT:
          global_Done = 1;
          break;
        case SDL_MOUSEMOTION:
          //std::cout << "Mouse move: x " << float(event.motion.x)/screen->w << " y " << float(event.motion.y)/screen->h << std::endl;
          break;
        case SDL_MOUSEBUTTONDOWN:
          guiManager_.receive(event.button, screen_.height());
          break;
        default:
          break;
      }
    }
#ifdef TIMER_OPENSTEER_CLOCK
    timer.update();
    Uint32 now_ticks = timer.getRealTime();
#else
    Uint32 now_ticks = SDL_GetTicks();
#endif
    OpenGTA::SpriteManager::Instance().update(now_ticks, localPlayer_);
    city->blockAnims->update(now_ticks);
    guiManager_.update(now_ticks);
    update_ingame_gui_values(localPlayer_);
    if (!paused) {
      drawScene(now_ticks - last_tick, guiManager_, screen_);
    last_tick = now_ticks;
      if (vm_tick_ok && (now_ticks - script_last_tick > 100)) {
        try {
          luaVM_.callSimpleFunction("game_tick");
          script_last_tick = now_ticks;
        }
        catch (Exception & e) {
          vm_tick_ok = false;
          ERROR("Disabling script game_tick because of error: {}", e.what());
        }
      }
    }
    OpenGTA::SpriteManager::Instance().creationArea.setRects(
      city->getActiveRect(), city->getOnScreenRect());

//#ifdef TIMER_OPENSTEER_CLOCK
//    fps = int(timer.clock.getSmoothedFPS());
//#else
    if (now_ticks - fps_last_tick > 2000) {
      uint32_t fps = num_frames_drawn / 2;
      num_frames_drawn = 0;
      fps_last_tick = now_ticks;
      fps_label->text = std::to_string(fps) + " fps";
      luaVM_.setGlobalInt("current_fps", fps);
    }
//#endif
//    SDL_Delay(10);
  }

  luaVM_.runFile("scripts/dump_config.lua");

}

int main(int argc, char* argv[]) {
  if (argc > 1)
    parse_args(argc, argv);

  OpenGTAViewer app;

  app.init(argv[0]);
   try {
    app.run();
  } catch (const std::exception &e) {
    ERROR("Exception occurred: {}", e.what());
    throw;
  }
  app.quit();

  return 0;
}
