#ifndef BOMBERMAN_AP_LOGIC_SCORE_H
#define BOMBERMAN_AP_LOGIC_SCORE_H

#include "bomberman/logic/Event.h"

#include <filesystem>
#include <memory>
#include <vector>

namespace bomberman::logic {

struct ScoreEntry {
    int score{0};
};

class Score final : public Observer, public std::enable_shared_from_this<Score> {
public:
    explicit Score(std::filesystem::path scoreFile);

    void onNotify(const Event& event) override;

    int getCurrentScore() const { return currentPoints; }

    const std::vector<ScoreEntry>& getHighScores() const { return topScores; }

    void resetCurrentScore();

    void addSurvivalTimeScore(float seconds);

    void load();
    void saveCurrentScore();

private:
    void sortAndKeepTopFive();

    std::filesystem::path scorePath;
    int currentPoints{0};
    std::vector<ScoreEntry> topScores{};
};

}

#endif //BOMBERMAN_AP_LOGIC_SCORE_H
