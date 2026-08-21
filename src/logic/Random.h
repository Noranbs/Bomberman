#ifndef BOMBERMAN_AP_LOGIC_RANDOM_H
#define BOMBERMAN_AP_LOGIC_RANDOM_H

#include <random>

namespace bomberman::logic {

/**
 * @brief Singleton wrapper around the random number generator.
 */
class Random final {
public:
    /**
     * @brief Returns the single Random object.
     */
    static Random& instance();

    Random(const Random&) = delete;
    Random& operator=(const Random&) = delete;

    /**
     * @brief Returns a random float between min and max.
     * @param min Smallest possible value.
     * @param max Biggest possible value.
     * @return Random float in the range.
     */
    [[nodiscard]] float getRandomFloat(float min, float max);

    /**
     * @brief Returns a random integer between min and max.
     * @param min Smallest possible value.
     * @param max Biggest possible value.
     * @return Random integer in the range.
     */
    [[nodiscard]] int getRandomInt(int min, int max);

    /**
     * @brief Returns true with the given probability.
     * @param probability Chance between 0 and 1.
     * @return True if the chance succeeds.
     */
    [[nodiscard]] bool rollChance(float probability);

private:
    Random();

    std::mt19937 rngEngine; ///< Random generator used for all random values.
};

}

#endif //BOMBERMAN_AP_LOGIC_RANDOM_H
