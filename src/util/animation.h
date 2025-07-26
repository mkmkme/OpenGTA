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
#ifndef UTIL_ANIMATION_H
#define UTIL_ANIMATION_H
#include <cstdint>
#include <functional>

namespace Util {

/** Capsules play-frames-in-sequence logic.
 *
 * This class only knows about the number of frames
 * and the fps; it does have multiple flags that
 * switch the behaviour whenever the 'last' frame is
 * finished.
 */
class Animation {
public:
    enum class Status : uint8_t {
        Stopped = 0,
        PlayForward,
        PlayBackward,
    };
    enum class OnDone : uint8_t {
        Stop = 0,
        Reverse,
        Loop,
        FCallback,
    };
    Animation(uint16_t numFrames, uint16_t fps) noexcept;
    [[nodiscard]] uint16_t getCurrentFrameNumber() const noexcept { return currentFrame; }
    void set(Status doThis, OnDone done = OnDone::Stop) noexcept
    {
        status = doThis;
        onDone = done;
    }
    [[nodiscard]] Status get() const { return status; }
    [[nodiscard]] OnDone getDone() const { return onDone; }
    void jumpToFrame(uint16_t num, Status andDo);
    void update(uint32_t nowTicks);
    using CallbackType = std::function<void()>;
    void setCallback(CallbackType cb) { callback = std::move(cb); }

    uint16_t currentFrame;
    uint16_t numFrames;
    uint32_t delay;

protected:
    void flipFrame(bool forward);
    void isDone();
    Status status;
    OnDone onDone;
    uint32_t lastChangeTicks;

    CallbackType callback;
};
} // namespace Util

#endif
