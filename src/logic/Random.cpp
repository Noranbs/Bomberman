#include "bomberman/logic/Random.h"
#include <chrono>

namespace bomberman::logic {

Random& Random::instance()
{
    static Random instance;
    return instance;
}

float Random::getRandomFloat(float min, float max)
{
    std::uniform_real_distribution<float> dist(min, max);
    return dist(rngEngine);
}

int Random::getRandomInt(int min, int max)
{
    std::uniform_int_distribution<int> dist(min, max);
    return dist(rngEngine);
}

bool Random::rollChance(float probability)
{
    return getRandomFloat(0.0F, 1.0F) < probability;
}

Random::Random()
    : rngEngine(static_cast<std::mt19937::result_type>(
          std::chrono::steady_clock::now().time_since_epoch().count()))
{
}

}
