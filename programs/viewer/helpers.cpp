#include "helpers.h"

#include <array>

#include "util/errors.h"
#include "util/log.h"

#include "core/active-style.h"
#include "core/id_sys.h"
#include "core/spritemanager.h"

void setGamma(SDL_Window *w, float v)
{
    std::array<uint16_t, 256> ramp;
    for (int i = 0; i < 256; i++) {
        auto f = static_cast<float>(i) / 255.0f;
        f = pow(f, 1.0f / v);
        ramp[i] = uint16_t(f) * 65535;
    }
    SDL_SetWindowGammaRamp(w, ramp.data(), ramp.data(), ramp.data());
}

void explodePed() noexcept
{
    try {
        OpenGTA::Pedestrian &ped = OpenGTA::SpriteManager::Instance().getPed(0xffffffff);
        auto p = ped.pos;
        p.y += 0.2f;
        OpenGTA::SpriteManager::Instance().createExplosion(p);
    } catch (Util::UnknownKey &e) {
        WARN("Cannot place explosion - press F4 to switch to player-mode first!");
    }
}

void addAutoPed() noexcept
{
    try {
        OpenGTA::Pedestrian &pr = OpenGTA::SpriteManager::Instance().getPed(0xffffffff);
        int id = OpenGTA::TypeIdBlackBox::Instance().requestId();
        auto v = pr.pos;
        v.y += 0.9f;
        // INFO << v.x << " " << v.y << " " << v.z << std::endl;
        Sint16 remap = OpenGTA::ActiveStyle::Instance().get().getRandomPedRemapNumber();
        OpenGTA::Pedestrian p(glm::vec3(0.2f, 0.5f, 0.2f), v, id, remap);
        OpenGTA::Pedestrian &pr2 = OpenGTA::SpriteManager::Instance().add(std::move(p));
        pr2.switchToAnim(1);
        INFO("now {} peds", OpenGTA::SpriteManager::Instance().getPeds().size());

        // pr2.m_control = &OpenGTA::nullAI;
    } catch (Util::UnknownKey &e) {
        WARN("Cannot place peds now - press F4 to switch to player-mode first!");
    }
}

void ai_step_fake(OpenGTA::Pedestrian *p)
{
    OpenGTA::Pedestrian &pr = OpenGTA::SpriteManager::Instance().getPed(0xffffffff);
    float t_angle = Util::xz_angle(p->pos, pr.pos);
    // INFO << "dist " << Util::distance(p->pos, pr.pos) << std::endl;
    // INFO << "angle " << t_angle << std::endl;
    // INFO << "myrot: " << p->rot << std::endl;
    if (glm::distance(p->pos, pr.pos) > 3) {
        p->m_control.setTurnLeft(false);
        p->m_control.setTurnRight(false);
        if (t_angle > p->rot)
            p->m_control.setTurnLeft(true);
        else
            p->m_control.setTurnRight(true);
    } else {
        p->m_control.setMoveForward(true);
        static thread_local std::mt19937 generator(std::random_device {}());
        std::uniform_int_distribution<int> distribution(0, 4);
        int k = distribution(generator);
        if (k == 0) {
            p->m_control.setTurnLeft(false);
            p->m_control.setTurnRight(false);
        } else if (k == 1) {
            p->m_control.setTurnLeft(true);
            p->m_control.setTurnRight(false);
        } else if (k == 2) {
            p->m_control.setTurnLeft(false);
            p->m_control.setTurnRight(true);
        }
    }
}
