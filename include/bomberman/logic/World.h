#ifndef BOMBERMAN_AP_LOGIC_WORLD_H
#define BOMBERMAN_AP_LOGIC_WORLD_H

#include "bomberman/logic/EntityFactory.h"
#include "bomberman/logic/Score.h"

#include <memory>
#include <optional>
#include <vector>

namespace bomberman::logic {

enum class Action {
    MoveLeft,
    MoveRight,
    MoveUp,
    MoveDown,
    StopHorizontal,
    StopVertical,
    PlaceBomb,
    KickBomb
};

class World final {
public:
    explicit World(std::shared_ptr<EntityFactory> factory);

    void startNewGame();
    void startNewGameAtLevel(int level);
    void startNextLevel();
    void update(float deltaTime);
    void handlePlayerAction(Action action, bool active);

    const std::vector<std::shared_ptr<Entity>>& entities() const { return entitiesList; }
    std::shared_ptr<Character> player() const { return playerChar; }
    int rows() const { return numRows; }
    int cols() const { return numCols; }
    Vec2 tileSize() const { return tileDimensions; }
    std::shared_ptr<Score> score() const { return scoreTracker; }
    float elapsedTime() const { return totalTime; }
    int playerLives() const { return remainingPlayerLives; }
    int currentLevel() const { return levelNumber; }
    bool finalLevelComplete() const { return hasWon && levelNumber >= maxLevel; }
    int enemiesAlive() const;
    bool playerWon() const { return hasWon; }

private:
    static constexpr int maxLevel{3};

    struct BombState {
        std::shared_ptr<Block> entity{};
        std::weak_ptr<Character> owner{};
        float timer{2.0F};
        int radius{1};
        bool blocksOwner{false};
        bool exploded{false};
        Vec2 moveDirection{};
        int targetRow{0};
        int targetCol{0};
        bool rubberBounce{false};
    };

    struct ExplosionState {
        std::shared_ptr<Block> entity{};
        float timer{0.65F};
    };

    struct EnemyAiState {
        std::weak_ptr<Character> enemy{};
        Vec2 input{};
        float decisionTimer{0.0F};
        float bombCooldown{0.0F};
        bool escapingOwnBomb{false};
        int escapeBombRow{0};
        int escapeBombCol{0};
        int escapeBombRadius{1};
        bool hasTarget{false};
        int targetRow{0};
        int targetCol{0};
        std::vector<Vec2> escapePath{};
    };

    struct PlayerStats {
        int bombRadius{1};
        int bombCapacity{1};
        float speed{0.5F};
        bool canKickBombs{false};
        bool hasRubberBombs{false};
    };

    std::size_t nextId();
    Vec2 tileCenter(int row, int col) const;
    std::pair<int, int> tileForPosition(Vec2 position) const;
    [[nodiscard]] bool isInsideArena(int row, int col) const;
    [[nodiscard]] bool tileContains(EntityType type, int row, int col) const;
    [[nodiscard]] bool canEnterTile(const Character& character, int row, int col) const;
    [[nodiscard]] bool isTileDangerous(int row, int col) const;
    [[nodiscard]] bool isTileThreatenedByBomb(int row, int col, int bombRow, int bombCol, int radius) const;
    [[nodiscard]] bool hasEscapeFromBomb(const Character& character, int bombRow, int bombCol, int radius) const;
    [[nodiscard]] std::optional<Vec2> findEscapeInput(const Character& character,
                                                      int bombRow,
                                                      int bombCol,
                                                      int radius,
                                                      int maxDistanceOverride = -1) const;
    [[nodiscard]] std::optional<std::vector<Vec2>> findEscapePath(const Character& character,
                                                                   int bombRow,
                                                                   int bombCol,
                                                                   int radius,
                                                                   int maxDistanceOverride) const;
    [[nodiscard]] std::optional<Vec2> findStraightEscapeInput(const Character& character,
                                                              int bombRow,
                                                              int bombCol,
                                                              int radius) const;
    Vec2 directionForStep(int rowStep, int colStep) const;
    Vec2 chooseEnemyInput(Character& enemy);
    [[nodiscard]] bool collidesWithSolid(const Character& character, Vec2 target) const;
    [[nodiscard]] bool bombBlocksCharacter(const Entity& bomb, const Character& character) const;
    [[nodiscard]] bool hasBombAt(int row, int col) const;
    [[nodiscard]] bool canBombMoveTo(int row, int col, std::size_t movingBombId) const;
    [[nodiscard]] bool hasAdjacentDestructibleBlock(int row, int col) const;
    [[nodiscard]] bool levelObjectivesComplete() const;

    void createArena();
    void clearArenaEntities();
    void applyEnemyLevelBonus(Character& enemy);
    void savePlayerStats();
    void restorePlayerStats();
    void resetPlayerStats();
    void updateLevelExit();
    void createLevelExit();
    void createBlock(EntityType type, int row, int col);
    void moveCharacter(Character& character, Vec2 velocity, float deltaTime);
    void placePlayerBomb();
    void placeBombFor(const std::shared_ptr<Character>& character);
    void kickPlayerBomb();
    void updateMovingBomb(BombState& bomb, float deltaTime);
    void updateEnemies(float deltaTime);
    void moveEnemyTowardTarget(Character& enemy, EnemyAiState& ai, float deltaTime);
    void createRandomPowerUp(int row, int col);
    void updatePowerUpPickups();
    void collectPowerUps(Character& character);
    void applyPowerUp(Character& character, PowerUpType type);
    void updateBombs(float deltaTime);
    void updateExplosions(float deltaTime);
    void explodeBomb(std::size_t index);
    void createExplosionTile(int row, int col);
    bool applyExplosionToTile(int row, int col, bool playerOwnedBomb);
    void damagePlayer();
    void removeDeadTransientEntities();

    std::shared_ptr<EntityFactory> factory;
    std::shared_ptr<Score> scoreTracker;
    std::vector<std::shared_ptr<Entity>> entitiesList{};
    std::shared_ptr<Character> playerChar{};
    std::vector<std::shared_ptr<Character>> enemiesList{};
    std::vector<BombState> bombsList{};
    std::vector<ExplosionState> explosionsList{};
    std::vector<std::shared_ptr<PowerUp>> powerUpsList{};
    std::shared_ptr<Block> levelExit{};
    std::vector<EnemyAiState> enemyAiStates{};
    std::size_t entityCounter{1};
    int numRows{13};
    int numCols{15};
    Vec2 tileDimensions{};
    Vec2 playerInput{};
    float totalTime{0.0F};
    int remainingPlayerLives{5};
    int levelNumber{1};
    PlayerStats savedPlayerStats{};
    bool hasWon{false};
};

}

#endif //BOMBERMAN_AP_LOGIC_WORLD_H
