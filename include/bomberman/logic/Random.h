#ifndef BOMBERMAN_AP_LOGIC_RANDOM_H
#define BOMBERMAN_AP_LOGIC_RANDOM_H

#include <random>

namespace bomberman::logic {

class Random final {
public:
    static Random& instance();

    Random(const Random&) = delete;
    Random& operator=(const Random&) = delete;

    [[nodiscard]] float getRandomFloat(float min, float max);
    [[nodiscard]] int getRandomInt(int min, int max);
    [[nodiscard]] bool rollChance(float probability);

private:
    Random();

    std::mt19937 rngEngine;
};

}

#endif //BOMBERMAN_AP_LOGIC_RANDOM_H
