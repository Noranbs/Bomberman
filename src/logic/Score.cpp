#include "bomberman/logic/Score.h"

#include <algorithm>
#include <fstream>
#include <stdexcept>

namespace bomberman::logic {

Score::Score(std::filesystem::path scoreFile)
    : scorePath(std::move(scoreFile))
{
    load();
}

void Score::onNotify(const Event& event)
{
    switch (event.type) {
    case EventType::BlockDestroyed:
        currentPoints += 50;
        break;
    case EventType::PowerUpCollected:
        currentPoints += 100;
        break;
    case EventType::EnemyKilled:
        currentPoints += 250;
        break;
    case EventType::PlayerWon:
        currentPoints += 1000;
        saveCurrentScore();
        break;
    case EventType::PlayerLost:
        currentPoints -= 250;
        saveCurrentScore();
        break;
    default:
        break;
    }
}

void Score::resetCurrentScore()
{
    currentPoints = 0;
    pendingSurvivalPoints = 0.0F;
}

void Score::addSurvivalTimeScore(float seconds)
{
    pendingSurvivalPoints += seconds * 2.0F;
    const int wholePoints = static_cast<int>(pendingSurvivalPoints);
    if (wholePoints <= 0) {
        return;
    }

    currentPoints += wholePoints;
    pendingSurvivalPoints -= static_cast<float>(wholePoints);
}

void Score::load()
{
    topScores.clear();
    std::ifstream input(scorePath);
    if (!input.is_open()) {
        return;
    }

    int value = 0;
    while (input >> value) {
        topScores.push_back({value});
    }
    sortAndKeepTopFive();
}

void Score::saveCurrentScore()
{
    topScores.push_back({currentPoints});
    sortAndKeepTopFive();

    std::ofstream output(scorePath);
    if (!output.is_open()) {
        throw std::runtime_error("Unable to write high score file: " + scorePath.string());
    }

    for (const auto& entry : topScores) {
        output << entry.score << '\n';
    }
}

void Score::sortAndKeepTopFive()
{
    std::sort(topScores.begin(), topScores.end(), [](const ScoreEntry& lhs, const ScoreEntry& rhs) {
        return lhs.score > rhs.score;
    });

    if (topScores.size() > 5) {
        topScores.resize(5);
    }
}

}
