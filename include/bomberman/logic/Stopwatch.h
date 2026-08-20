#ifndef BOMBERMAN_AP_LOGIC_STOPWATCH_H
#define BOMBERMAN_AP_LOGIC_STOPWATCH_H

#include <chrono>

namespace bomberman::logic {

/**
 * @brief Singleton that measures frame time for the game loop.
 */
class Stopwatch final {
public:
    /**
     * @brief Returns the single Stopwatch object.
     */
    static Stopwatch& instance();

    Stopwatch(const Stopwatch&) = delete;
    Stopwatch& operator=(const Stopwatch&) = delete;

    /**
     * @brief Restarts the stopwatch.
     */
    void reset();

    /**
     * @brief Updates and returns the time since the previous frame.
     * @return Delta time in seconds.
     */
    float updateFrameTime();

    /**
     * @brief Returns the last measured frame time.
     * @return Delta time in seconds.
     */
    float getDeltaTime() const { return frameDeltaTime; }

    /**
     * @brief Returns the time since the last reset.
     * @return Total time in seconds.
     */
    float getTotalElapsedTime() const;

private:
    using Clock = std::chrono::steady_clock;

    Stopwatch();

    Clock::time_point lastTime;       ///< Time point of the previous frame.
    Clock::time_point startTime;      ///< Time point of the last reset.
    float frameDeltaTime{0.0F};       ///< Last frame duration in seconds.
};

}

#endif //BOMBERMAN_AP_LOGIC_STOPWATCH_H
