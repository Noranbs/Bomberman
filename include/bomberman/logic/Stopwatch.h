#ifndef BOMBERMAN_AP_LOGIC_STOPWATCH_H
#define BOMBERMAN_AP_LOGIC_STOPWATCH_H

#include <chrono>

namespace bomberman::logic {

class Stopwatch final {
public:
    static Stopwatch& instance();

    Stopwatch(const Stopwatch&) = delete;
    Stopwatch& operator=(const Stopwatch&) = delete;

    void reset();
    float updateFrameTime();

    float getDeltaTime() const { return frameDeltaTime; }

    float getTotalElapsedTime() const;

private:
    using Clock = std::chrono::steady_clock;

    Stopwatch();

    Clock::time_point lastTime;
    Clock::time_point startTime;
    float frameDeltaTime{0.0F};
};

}

#endif //BOMBERMAN_AP_LOGIC_STOPWATCH_H
