#ifndef BOMBERMAN_AP_LOGIC_SCORE_H
#define BOMBERMAN_AP_LOGIC_SCORE_H

#include "logic/Event.h"

#include <filesystem>
#include <vector>

namespace bomberman::logic {

/**
 * @brief One saved high score.
 */
struct ScoreEntry {
    /// Stored score value.
    int score{0};
};

/**
 * @brief Keeps the current score and the top five saved scores.
 */
class Score final : public Observer {
public:
    /**
     * @brief Creates a score tracker using the given score file.
     * @param scoreFile File where high scores are loaded and saved.
     */
    explicit Score(std::filesystem::path scoreFile);

    /**
     * @brief Updates the score when a game event happens.
     * @param event Event that changes the score.
     */
    void onNotify(const Event& event) override;

    int getCurrentScore() const { return currentPoints; }

    const std::vector<ScoreEntry>& getHighScores() const { return topScores; }

    /**
     * @brief Clears the score for a new game.
     */
    void resetCurrentScore();

    /**
     * @brief Adds points for the time the player stays alive.
     * @param seconds Time to convert to score.
     */
    void addSurvivalTimeScore(float seconds);

    /**
     * @brief Loads saved high scores from the score file.
     */
    void load();

    /**
     * @brief Saves the current score into the top-five list.
     */
    void saveCurrentScore();

private:
    void sortAndKeepTopFive();

    std::filesystem::path scorePath;             ///< File used for high scores.
    int currentPoints{0};                        ///< Current game score.
    float pendingSurvivalPoints{0.0F};           ///< Fractional survival points not added yet.
    std::vector<ScoreEntry> topScores{};         ///< Saved top five scores.
};

}

#endif //BOMBERMAN_AP_LOGIC_SCORE_H
