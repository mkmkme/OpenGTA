#pragma once

#include <SDL_video.h>

namespace OpenGTA {
class Pedestrian;
}

void setGamma(SDL_Window *w, float v);
void explodePed() noexcept;
void addAutoPed() noexcept;

// important to keep the naming as is due to linking
// FIXME: fix this pls
void ai_step_fake(OpenGTA::Pedestrian *p);
