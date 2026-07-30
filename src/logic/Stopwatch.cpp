#include "bomberman/logic/Stopwatch.h"

namespace bomberman::logic {

Stopwatch& Stopwatch::instance()
{
    static Stopwatch instance;
    return instance;
}

void Stopwatch::reset()
{
    startTime = Clock::now();
    lastTime = startTime;
    frameDeltaTime = 0.0F;
}

float Stopwatch::updateFrameTime()
{
    const auto now = Clock::now();
    frameDeltaTime = std::chrono::duration<float>(now - lastTime).count();
    lastTime = now;
    return frameDeltaTime;
}

float Stopwatch::getTotalElapsedTime() const
{
    return std::chrono::duration<float>(Clock::now() - startTime).count();
}

Stopwatch::Stopwatch()
    : lastTime(Clock::now()), startTime(lastTime)
{
}

}
