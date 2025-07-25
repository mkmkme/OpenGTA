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
#include "util/animation.h"

#include "util/errors.h"
#include "util/log.h"

namespace Util {
Animation::Animation(uint16_t num, uint16_t fps) noexcept
{
    status = Status::Stopped;
    numFrames = num;
    currentFrame = 0;
    delay = 1000 / fps;
    lastChangeTicks = 0;
}

Animation::Animation(Animation &&other) noexcept = default;
Animation &Animation::operator=(Animation &&other) noexcept = default;
Animation::Animation(const Animation &other) noexcept = default;
Animation &Animation::operator=(const Animation &other) noexcept = default;

void Animation::update(uint32_t nowTicks)
{
    if (status == Status::Stopped)
        return;
    if (lastChangeTicks == 0)
        lastChangeTicks = nowTicks;
    if (nowTicks < lastChangeTicks + delay)
        return;
    lastChangeTicks = nowTicks;
    if (status == Status::PlayForward)
        flipFrame(true);
    else if (status == Status::PlayBackward)
        flipFrame(false);
}

void Animation::flipFrame(bool forward = true)
{
    if (forward) {
        if (currentFrame < numFrames - 1)
            ++currentFrame;
        else if (currentFrame == numFrames - 1)
            isDone();
    } else {
        if (currentFrame == 0)
            isDone();
        else
            --currentFrame;
    }
}

void Animation::jumpToFrame(const uint16_t num, const Status andDo)
{
    if (num >= numFrames)
        throw Util::OutOfRange("Jumping to frame {} is out of range (< {})", num, numFrames);
    currentFrame = num;
    status = andDo;
}

void Animation::isDone()
{
    if (onDone == OnDone::Stop) {
        status = Status::Stopped;
        return;
    }
    if (onDone == OnDone::Reverse) {
        status = (status == Status::PlayForward) ? Status::PlayBackward : Status::PlayForward;
        return;
    }
    if (onDone == OnDone::Loop) {
        if (status == Status::PlayForward)
            jumpToFrame(0, Status::PlayForward);
        else if (status == Status::PlayBackward)
            jumpToFrame(numFrames - 1, Status::PlayBackward);
        return;
    }
    status = Status::Stopped;
    if (onDone == OnDone::FCallback) {
        if (callback)
            callback();
        else
            ERROR("Wanted to call callback, but nobody was there");
    }
}
} // namespace Util
