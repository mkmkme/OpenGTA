#include "viewer.h"

#include <SDL.h>

#include <fmt/base.h>

#include "graphics/cityview.h"
#include "graphics/spritecache.h"
#include "lua-addon/stackguard.h"
#include "util/file_helper.h"
#include "util/log.h"

#include "core/active-style.h"
#include "core/ai.h"
#include "core/main-msg-lookup.h"
#include "core/spritemanager.h"
#include "helpers.h"
#include "viewer-config.h"

namespace OpenGTA {

namespace Globals {
extern bool done;
}

void Viewer::quit()
{
    SDL_Quit();
    fmt::print("Goodbye\n");
}

void Viewer::screenGammaCallback(float v)
{
    screen_gamma_ = v;
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
    auto *o = guiManager_.findObject(GUI::GAMMA_LABEL_ID);
    if (auto *l = dynamic_cast<GUI::Label *>(o))
        l->text = "Gamma: " + std::to_string(v);
}

Viewer::Viewer(ViewerConfig &&config)
    : config_ { std::move(config) }
    , luaVM_ { screen_, camera_ }
    , physfs_context_ { config_.getProgName().data() }
{
}

void Viewer::initialize()
{
    physfs_context_.tryMount(Util::FileHelper::ModDataPath().c_str(), false);

    // check for a configfile
    if (physfs_context_.exists("config")) {
        const auto config_as_string = Util::PhysFSFile("config").readAll();

        lua_State *L = luaVM_.getInternalState();
        Util::LuaStackguard guard(L);

        if (luaL_loadbuffer(L, config_as_string.c_str(), config_as_string.size(), "config"))
            throw Util::ScriptError("Error running string: {}", lua_tostring(L, -1));
        lua_newtable(L);
        lua_pushvalue(L, -1);
        // lua_setglobal(L, "config");
        lua_setfield(L, -2, "config");
        if (lua_pcall(L, 0, 0, 0))
            throw Util::ScriptError("Error running string: {}", lua_tostring(L, -1));

        config_.tryParseLuaConfig(luaVM_);
    }
    // fullscreen before first video init; only chance to set it on win32
    screen_.setFullScreenFlag(config_.isFullscreen());
    screen_.setVSyncMode(config_.isVsync() ? OpenGL::VSyncMode::SDL : OpenGL::VSyncMode::NoSync);

    // create screen
    screen_.activate(config_.getWindowWidth(), config_.getWindowHeight());

    if (ImageUtil::supportedMaxAnisoDegree >= config_.getAnisotropicFilterDegree()) {
        ImageUtil::supportedMaxAnisoDegree = config_.getAnisotropicFilterDegree();
    } else {
        WARN(
            "Using filter degree {}, requested {} not supported",
            ImageUtil::supportedMaxAnisoDegree,
            config_.getAnisotropicFilterDegree()
        );
    }

    // SDL_SetGamma(config_.getScreenGamma(), config_.getScreenGamma(), config_.getScreenGamma());
    WARN("SDL_SetGamma to be called!");

    ImageUtil::mipmapTextures = config_.mipmapTextures();

    // before any graphics are loaded
    OpenGL::SpriteCache::Instance().setScale2x(config_.isScale2x());

    // FIXME: basic gui setup; should not be here
    SDL_Rect rect { .x = 5, .y = 50 };
    fps_label_ = new GUI::Label(rect, "", "F_MTEXT.FON", 1);
    // fps_label->borderColor.r = fps_label->borderColor.unused = 200;
    guiManager_.add(fps_label_, 5);
}

void Viewer::createPedAt(const glm::vec3 &v)
{
    OpenGTA::Pedestrian p({ 0.2f, 0.5f, 0.2f }, v, 0xffffffff);
    p.setRemap(OpenGTA::ActiveStyle::Instance().get().getRandomPedRemapNumber());
    INFO("using remap: {}", p.getRemap());
    OpenGTA::Pedestrian &pr = OpenGTA::SpriteManager::Instance().add(std::move(p));
    pr.switchToAnim(1);
    localPlayer_.setCtrl(pr.m_control);
}

void Viewer::showGammaConfig()
{
    if (gamma_slide_) {
        SDL_Rect r { .x = static_cast<int>(screen_.width() / 2),
                     .y = static_cast<int>(screen_.height() / 2),
                     .w = 200,
                     .h = 30 };

        auto *sb = new GUI::ScrollBar(GUI::GAMMA_SCROLLBAR_ID, r);
        sb->color.r = sb->color.g = sb->color.b = 180;
        sb->color.a = 255;
        sb->innerColor.r = 250;
        sb->value = screen_gamma_ / 2;
        sb->changeCB = GUI::ScrollBar::SC_Functor([this](float v) { screenGammaCallback(v); });
        guiManager_.add(sb, 90);

        r.y += 40;
        auto *l = new GUI::Label(GUI::GAMMA_LABEL_ID, r, "Gamma: " + std::to_string(screen_gamma_), "F_MTEXT.FON", 1);
        guiManager_.add(l, 80);

        OpenGL::Screen::setSystemMouseCursor(true);

    } else {
        guiManager_.removeById(GUI::GAMMA_SCROLLBAR_ID);
        guiManager_.removeById(GUI::GAMMA_LABEL_ID);
        OpenGL::Screen::setSystemMouseCursor(false);
    }
}

void Viewer::createIngameUI()
{
    {
        SDL_Rect r { .x = static_cast<int>((screen_.width() / 2) - 50),
                     .y = static_cast<int>(screen_.height() - 32),
                     .w = 100,
                     .h = 32 };
        SDL_Rect rs { .x = 0, .y = 0, .w = 16, .h = 16 };
        guiManager_.cacheStyleArrowSprite(16, -1);
        guiManager_.cacheStyleArrowSprite(17, -1);
        std::vector<uint16_t> anim2frames { 16, 17 };
        guiManager_.createAnimation(anim2frames, 10, 2);
        wanted_level_ = new GUI::AnimStatusDisplay(GUI::WANTED_LEVEL_ID, r, rs, 2);
        /*
        wantedLevel->borderColor.r = wantedLevel->borderColor.g = wantedLevel->borderColor.b =
        wantedLevel->borderColor.unused = 255; wantedLevel->drawBorder = 1;
        */
        guiManager_.add(wanted_level_, 50);
    }

    {
        SDL_Rect r;
        r.x = screen_.width() - 5;
        r.y = screen_.height() - 30;
        cash_label_ = new GUI::Label(GUI::CASH_ID, r, "0", "F_MTEXT.FON", 1);
        cash_label_->align = 1;
        guiManager_.add(cash_label_, 50);
    }
}

void Viewer::updateIngameUI()
{
    if (wanted_level_ != nullptr) {
        wanted_level_->number = localPlayer_.getWantedLevel();
    }

    if (cash_label_ != nullptr) {
        cash_label_->text = std::to_string(localPlayer_.getCash());
    }
}

void Viewer::removeIngameUI()
{
    if (wanted_level_ != nullptr) {
        guiManager_.remove(wanted_level_);
        wanted_level_ = nullptr;
    }

    if (cash_label_ != nullptr) {
        guiManager_.remove(cash_label_);
        cash_label_ = nullptr;
    }
}

void Viewer::carToggle()
{
    Pedestrian &pped = localPlayer_.getPed();
    auto pos = pped.pos;
    auto &cars = OpenGTA::SpriteManager::Instance().getCars();
    float min_dist = 360;
    auto j = cars.end();
    for (auto it = cars.begin(); it != cars.end(); ++it) {
        if (float tmp_dist = glm::distance(pos, it->second.pos); tmp_dist < min_dist) {
            j = it;
            min_dist = tmp_dist;
        }
    }
    assert(j != cars.end());
    const auto &car = j->second;
    fmt::print("{} {} {}, {}\n", car.id(), car.pos.x, car.pos.y, car.pos.z);
    glm::vec3 p_door(car.carInfo.door[0].rpx / 64.0f, 0, car.carInfo.door[0].rpy / 64.0f);

    auto p_door_global = car.transformCoords(p_door);
    p_door_global.y += 0.2f;
    fmt::print("{}, {}, {}\n", p_door_global.x, p_door_global.y, p_door_global.z);
    test_dot_ = p_door_global;
    // pped.aiMode = 1;
    // pped.aiData.pos1 = p_door_global;
    AI::Pedestrian::walk_pavement(&pped);
}

void Viewer::drawMapMode()
{
    SDL_Event event;
    OpenGL::PagedTexture map_tex = city_->renderMap2Texture();
    bool done_map = false;
    OpenGL::Screen::setSystemMouseCursor(true);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    INFO("{}", map_tex.coords[1].u);
    while (!done_map) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    done_map = true;
                    break;
                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym) {
                        case SDLK_ESCAPE:
                            done_map = true;
                            break;
                        default:
                            break;
                    }
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    INFO("{} {}", event.button.x / 600.0f * 255, event.button.y / 600.0f * 255);
                    map_position_[0] = event.button.x / 600.0f * 255;
                    map_position_[2] = event.button.y / 600.0f * 255;
                    // mapPos[1] = 10;
                    done_map = true;
                    break;
                case SDL_MOUSEMOTION:
                    INFO("Mouse move: x {} y {}", event.motion.x, event.motion.y);
                    break;
                default:
                    break;
            }
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        screen_.setFlatProjection();
        glBindTexture(GL_TEXTURE_2D, map_tex.inPage);
        // glMatrixMode(GL_TEXTURE);
        // if (_scale < 1)
        //   glScalef(_scale, _scale, 1);

        const uint32_t h = screen_.height();
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

        const OpenGTA::Map::LocationMap &lmap = OpenGTA::ActiveMap::Instance().get().getLocationMap();
        OpenGTA::Map::LocationMap::const_iterator i = lmap.begin();
        glDisable(GL_TEXTURE_2D);
        while (i != lmap.end()) {
            if (i->first == 2) {
                i++;
                continue;
            }
            // uint8_t l_type = i->first;
            float l_x, l_y;
            l_x = i->second.x / 255.0f * h; // *  map_tex.coords[1].u;
            l_y = i->second.y / 255.0f * h; // * map_tex.coords[1].u;
            // INFO << int(l_type) << ": " << l_x << " " << l_y << std::endl;
            glBegin(GL_LINE_STRIP);
            glVertex2f(l_x - 5, l_y - 5);
            glVertex2f(l_x + 5, l_y - 5);
            glVertex2f(l_x + 5, l_y + 5);
            glVertex2f(l_x - 5, l_y + 5);

            glEnd();
            ++i;
        }

        glEnable(GL_TEXTURE_2D);
        SDL_GL_SwapWindow(screen_.get());
        SDL_Delay(20);
    }
    OpenGL::Screen::setSystemMouseCursor(false);
    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    // the texture class doesn't cleanup!
    glDeleteTextures(1, &map_tex.inPage);
}

void Viewer::printPosition() const noexcept
{
    const auto &v = camera_.getCenter();
    const auto &e = camera_.getEye();
    const auto &u = camera_.getUp();
    if (!city_->getViewMode()) {
        fmt::print("{}: {}\n", config_.getMapFile(), city_->getCurrentSector()->getFullName());
        fmt::print("camera.setCenter({}, {}, {})\n", v.x, v.y, v.z);
        fmt::print("camera.setEye({}, {}, {})\n", e.x, e.y, e.z);
        fmt::print("camera.setUp({}, {}, {})\n", u.x, u.y, u.z);
        fmt::print("city_view:setVisibleRange({})\n", city_->getVisibleRange());
        fmt::print("city_view:setTopDownView( false )\n");
    } else {
        const auto cp = city_->getCamPos();
        fmt::print("{}: {}\n", config_.getMapFile(), city_->getCurrentSector()->getFullName());
        fmt::print("city_view:setCamPosition({}, {}, {})\n", cp[0], cp[1], cp[2]);
        fmt::print("city_view:setVisibleRange({})\n", city_->getVisibleRange());
        fmt::print("city_view:setTopDownView( true )\n");
    }
}

void Viewer::handleKeyPress(SDL_Keysym *keysym)
{
    const auto cp = city_->getCamPos();
    map_position_ = { cp[0], cp[1], cp[2] };
    switch (keysym->sym) {
        case SDLK_ESCAPE:
            OpenGTA::Globals::done = true;
            break;
        case SDLK_LEFT:
            map_position_[0] -= 1.0f;
            camera_.translateBy({ -1, 0, 0 });
            break;
        case SDLK_RIGHT:
            map_position_[0] += 1.0f;
            camera_.translateBy({ 1, 0, 0 });
            break;
        case SDLK_UP:
            map_position_[2] -= 1.0f;
            camera_.translateBy({ 0, 0, -1 });
            break;
        case SDLK_DOWN:
            map_position_[2] += 1.0f;
            camera_.translateBy({ 0, 0, 1 });
            break;
        case SDLK_SPACE:
            camera_.setSpeed(0.0f);
            break;
        case SDLK_F2:
            draw_bbox_ = !draw_bbox_;
            OpenGTA::SpriteManager::Instance().setDrawBBox(draw_bbox_);
            break;
        case SDLK_F3:
            draw_texture_border_ = !draw_texture_border_;
            OpenGTA::SpriteManager::Instance().setDrawTexBorder(draw_texture_border_);
            break;
        case SDLK_F4:
            follow_mode_ = !follow_mode_;
            if (follow_mode_) {
                // SDL_EnableKeyRepeat( 0, SDL_DEFAULT_REPEAT_INTERVAL );
                city_->setViewMode(false);
                auto p = camera_.getEye();
                createPedAt(p);
                camera_.setVectors({ p.x, 10, p.z }, { p.x, 9.0f, p.z }, { 0, 0, -1 });
                camera_.setFollowMode(OpenGTA::SpriteManager::Instance().getPed(0xffffffff).pos);
                camera_.setCamGravity(true);
            } else {
                // SDL_EnableKeyRepeat( 100, SDL_DEFAULT_REPEAT_INTERVAL );
                camera_.setVectors(camera_.getEye(), camera_.getEye() + glm::vec3(1, -1, 1), glm::vec3(0, 1, 0));
                camera_.setCamGravity(false);
                camera_.releaseFollowMode();
                OpenGTA::SpriteManager::Instance().removePed(0xffffffff);
                OpenGTA::SpriteManager::Instance().removeDeadPeds();

                removeIngameUI();
            }
            break;
        case SDLK_RETURN:
            carToggle();
            break;
        case SDLK_F5:
            draw_heading_arrows_ = !draw_heading_arrows_;
            city_->setDrawHeadingArrows(draw_heading_arrows_);
            break;
        case SDLK_F6:
            drawMapMode();
            break;
        case SDLK_F7:
            explodePed();
            break;
        case SDLK_F8:
            addAutoPed();
            break;
        case SDLK_F9:
            city_->setDrawTextured(!city_->getDrawTextured());
            break;
        case SDLK_F10:
            city_->setDrawLines(!city_->getDrawLines());
            break;
        case SDLK_F12:
            gamma_slide_ = !gamma_slide_;
            showGammaConfig();
            break;
        case SDLK_LSHIFT:
            localPlayer_.getCtrl().toggleRunning();
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
            // OpenGTA::SpriteManager::Instance().getPed(0xffffffff).equip(0);
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
// FIXME: simply ignored on windows for now
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
            rotating_mode_ = !rotating_mode_;
            camera_.setRotating(rotating_mode_);
            break;
        case 'g':
            camera_gravity_ = !camera_gravity_; // TODO: rename
            camera_.setCamGravity(camera_gravity_);
            break;
        case 't':
            map_position_ = { 128, 230, 128 };
            city_->setVisibleRange(128);
            break;
        case 'p':
            printPosition();
            break;
        case '+':
            map_position_[1] += 1.0f;
            camera_.translateBy(glm::vec3(0, 1, 0));
            break;
        case '-':
            map_position_[1] -= 1.0f;
            camera_.translateBy(glm::vec3(0, -1, 0));
            break;
        case 'x':
            city_->setViewMode(false);
            city_->setVisibleRange(city_->getVisibleRange() * 2);
            break;
        case 'y':
            break;
        case 'z':
            city_->setViewMode(true);
            city_->setVisibleRange(city_->getVisibleRange() / 2);
            break;
        case '.':
            city_->setVisibleRange(city_->getVisibleRange() - 1);
            INFO(" new visible range {}", city_->getVisibleRange());
            break;
        case ',':
            city_->setVisibleRange(city_->getVisibleRange() + 1);
            INFO(" new visible range {}", city_->getVisibleRange());
            break;
        case SDLK_PRINTSCREEN:
            screen_.makeScreenshot("screenshot.bmp");
            break;
        default:
            return;
    }
    city_->setPosition(map_position_[0], map_position_[1], map_position_[2]);
}

void Viewer::handleKeyUp(SDL_Keysym *keysym)
{
    auto &pctrl = localPlayer_.getCtrl();
    switch (keysym->sym) {
        case 'j':
            pctrl.releaseTurnLeft();
            break;
        case 'l':
            pctrl.releaseTurnRight();
            break;
        case 'i':
            pctrl.releaseMoveForward();
            break;
        case 'k':
            pctrl.releaseMoveBack();
            break;
        case SDLK_LCTRL:
            pctrl.setFireWeapon(false);
            break;
        default:
            break;
    }
}

void Viewer::drawScene(uint32_t ticks)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    screen_.set3DProjection();
    city_->draw(ticks);

    glColor3f(1, 0, 0);
    glDisable(GL_TEXTURE_2D);
    glBegin(GL_POINTS);
    glVertex3f(test_dot_.x, test_dot_.y, test_dot_.z); // TODO: is this needed?
    glEnd();
    glEnable(GL_TEXTURE_2D);
    glColor3f(1, 1, 1);

    screen_.setFlatProjection();
    glDisable(GL_DEPTH_TEST);

    glPushMatrix();
    glTranslatef(10, 10, 0);
    OpenGL::DrawableFont &m_font = OpenGTA::FontCache::Instance().getFont("F_MTEXT.FON", 1);
    m_font.drawString(city_->getCurrentSector()->getFullName());
    glPopMatrix();

    /*glPushMatrix();
    glTranslatef(5, 50, 0);
    std::ostringstream strstr;
    strstr << fps << " fps";
    m_font->drawString(strstr.str());
    glPopMatrix();*/

    guiManager_.draw();

    num_frames_drawn_ += 1;
    glEnable(GL_DEPTH_TEST);

    SDL_GL_SwapWindow(screen_.get());
}

void Viewer::run()
{
    initialize();

    test_dot_ = glm::vec3(-1, -1, -1);
    SDL_Event event;
    const char *lang = std::getenv("OGTA_LANG"); // NOLINT(concurrency-mt-unsafe)
    if (!lang)
        lang = std::getenv("LANG"); // NOLINT(concurrency-mt-unsafe)
    if (!lang)
        lang = "en";
    OpenGTA::MainMsgLookup::Instance().load(Util::FileHelper::Lang2MsgFilename(lang));

    glEnable(GL_TEXTURE_2D);
    glPolygonMode(GL_FRONT, GL_FILL);
    glEnable(GL_CULL_FACE);

    // glEnable(GL_BLEND);
    // glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 2 / 255.0f); // 0);

    INFO("loading map: {}", config_.getMapFile());
    INFO("loading style: {}", config_.getStyleFile());

    city_ = std::make_unique<CityView>(config_.getMapFile(), config_.getStyleFile(), screen_, camera_);
    // if (city_blocks_area > -1)
    // city->setVisibleRange(city_blocks_area);
    city_->setPosition(map_position_[0], map_position_[1], map_position_[2]);

    camera_.setVectors(glm::vec3(12, 20, 12), glm::vec3(13.0f, 19.0f, 13.0f), glm::vec3(0, 1, 0));

    createIngameUI();

#ifdef TIMER_OPENSTEER_CLOCK
    Timer &timer = Timer::Instance();
    timer.update();
    last_tick_ = timer.getRealTime();
    // timer.setSimulationRunning(true);
#else
    last_tick_ = SDL_GetTicks();
#endif

    luaVM_.setCityView(*city_);
    luaVM_.setMap(OpenGTA::ActiveMap::Instance().get());
    if (!config_.getScriptFile().empty())
        luaVM_.runFile(config_.getScriptFile().data());
    bool vm_tick_ok = true;
    script_last_tick_ = last_tick_;

    while (!OpenGTA::Globals::done) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_WINDOWEVENT_FOCUS_GAINED:
                    paused_ = false;
                    break;
                case SDL_WINDOWEVENT_FOCUS_LOST:
                    paused_ = true;
                    break;
                case SDL_KEYDOWN:
                    if (event.key.repeat == 0) {
                        handleKeyPress(&event.key.keysym);
                    }
                    break;
                case SDL_KEYUP:
                    handleKeyUp(&event.key.keysym);
                    break;
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                    screen_.resize(event.window.data1, event.window.data2);
                    break;
                case SDL_QUIT:
                    OpenGTA::Globals::done = true;
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
        uint32_t now_ticks = timer.getRealTime();
#else
        uint32_t now_ticks = SDL_GetTicks();
#endif
        OpenGTA::SpriteManager::Instance().update(now_ticks, localPlayer_);
        city_->getBlockAnimCtrl().update(now_ticks);
        guiManager_.update(now_ticks);
        updateIngameUI();
        if (!paused_) {
            if (!SDL_WasInit(SDL_INIT_VIDEO)) {
                throw Util::NotSupported("SDL not initialized");
            }
            drawScene(now_ticks - last_tick_);
            last_tick_ = now_ticks;
            if (vm_tick_ok && (now_ticks - script_last_tick_ > 100)) {
                try {
                    luaVM_.callSimpleFunction("game_tick");
                    script_last_tick_ = now_ticks;
                } catch (Util::Exception &e) {
                    vm_tick_ok = false;
                    ERROR("Disabling script game_tick because of error: {}", e.what());
                }
            }
        }
        OpenGTA::SpriteManager::Instance().creationArea.setRects(city_->getActiveRect(), city_->getOnScreenRect());

        if (now_ticks - fps_last_tick_ > 2000) {
            uint32_t fps = num_frames_drawn_ / 2;
            num_frames_drawn_ = 0;
            fps_last_tick_ = now_ticks;
            fps_label_->text = std::to_string(fps) + " fps";
            luaVM_.setGlobalInt("current_fps", fps);
        }
    }

    luaVM_.runFile("scripts/dump_config.lua");
}

} // namespace OpenGTA
