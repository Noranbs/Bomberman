#ifndef BOMBERMAN_AP_LOGIC_WORLD_H
#define BOMBERMAN_AP_LOGIC_WORLD_H

#include "logic/AbstractFactory.h"
#include "logic/Score.h"

#include <memory>
#include <optional>
#include <vector>

namespace bomberman::logic {

/**
 * @brief Player actions that the representation layer can send to the world.
 */
enum class Action {
    MoveLeft,       ///< Start moving left.
    MoveRight,      ///< Start moving right.
    MoveUp,         ///< Start moving up.
    MoveDown,       ///< Start moving down.
    StopHorizontal, ///< Stop horizontal movement.
    StopVertical,   ///< Stop vertical movement.
    PlaceBomb,      ///< Place a bomb.
    KickBomb        ///< Kick an adjacent bomb.
};

class World;

/**
 * @brief Command interface for player actions.
 */
class PlayerCommand {
public:
    virtual ~PlayerCommand() = default;

    /**
     * @brief Applies this command to the world.
     * @param world World that receives the action.
     * @param active True when the action starts or is held.
     */
    virtual void execute(World& world, bool active) const = 0;
};

class MoveLeftCommand final : public PlayerCommand {
public:
    void execute(World& world, bool active) const override;
};

class MoveRightCommand final : public PlayerCommand {
public:
    void execute(World& world, bool active) const override;
};

class MoveUpCommand final : public PlayerCommand {
public:
    void execute(World& world, bool active) const override;
};

class MoveDownCommand final : public PlayerCommand {
public:
    void execute(World& world, bool active) const override;
};

class StopHorizontalCommand final : public PlayerCommand {
public:
    void execute(World& world, bool active) const override;
};

class StopVerticalCommand final : public PlayerCommand {
public:
    void execute(World& world, bool active) const override;
};

class PlaceBombCommand final : public PlayerCommand {
public:
    void execute(World& world, bool active) const override;
};

class KickBombCommand final : public PlayerCommand {
public:
    void execute(World& world, bool active) const override;
};

/**
 * @brief Main logic class that owns the arena and updates the game rules.
 *
 * World does not use SFML. It moves characters, places bombs, checks
 * collisions, updates enemies, creates power-ups, and decides when a level is won.
 */
class World final : public Subject {
public:
    /**
     * @brief Creates a world that uses the given entity factory.
     * @param factory Factory used to create all entities.
     */
    explicit World(std::shared_ptr<AbstractFactory> factory);

    /**
     * @brief Starts a new game from level 1.
     */
    void startNewGame();

    /**
     * @brief Starts a new game at a specific level.
     * @param level Level number to start from.
     */
    void startNewGameAtLevel(int level);

    /**
     * @brief Starts the next level and keeps saved player upgrades.
     */
    void startNextLevel();

    /**
     * @brief Updates the complete game logic.
     * @param deltaTime Time since the last update.
     */
    void update(float deltaTime);

    /**
     * @brief Handles player input translated by the SFML layer.
     * @param action Action to apply.
     * @param active True when the action starts or is held.
     */
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
    /**
     * @brief Returns the number of enemies that are still alive.
     */
    int enemiesAlive() const;
    bool playerWon() const { return hasWon; }

private:
    static constexpr int maxLevel{3};

    struct BombState {
        std::shared_ptr<Bomb> entity{};   ///< Bomb entity in the world.
        std::weak_ptr<Character> owner{}; ///< Character that placed the bomb.
        float timer{2.0F};                ///< Seconds until explosion.
        int radius{1};                    ///< Explosion radius in tiles.
        bool blocksOwner{false};          ///< True after the owner leaves the bomb.
        bool exploded{false};             ///< True when the bomb already exploded.
        Vec2 moveDirection{};             ///< Direction for a kicked bomb.
        int targetRow{0};                 ///< Tile row a moving bomb is going to.
        int targetCol{0};                 ///< Tile column a moving bomb is going to.
        bool rubberBounce{false};         ///< True if this bomb bounces when blocked.
    };

    struct ExplosionState {
        std::shared_ptr<Explosion> entity{}; ///< Explosion entity in the world.
        float timer{0.65F};              ///< Seconds before the explosion disappears.
    };

    struct EnemyAiState {
        std::weak_ptr<Character> enemy{}; ///< Enemy controlled by this AI state.
        Vec2 input{};                     ///< Current movement input.
        float decisionTimer{0.0F};        ///< Time until the next AI decision.
        float bombCooldown{0.0F};         ///< Time before this enemy may place another bomb.
        bool escapingOwnBomb{false};      ///< True while running away from its own bomb.
        int escapeBombRow{0};             ///< Row of the bomb to escape from.
        int escapeBombCol{0};             ///< Column of the bomb to escape from.
        int escapeBombRadius{1};          ///< Radius of the bomb to escape from.
        bool hasTarget{false};            ///< True if the enemy is moving to a target tile.
        int targetRow{0};                 ///< Target tile row.
        int targetCol{0};                 ///< Target tile column.
        std::vector<Vec2> escapePath{};   ///< Planned escape movement steps.
    };

    struct PlayerStats {
        int bombRadius{1};          ///< Saved bomb radius.
        int bombCapacity{1};        ///< Saved bomb capacity.
        float speed{0.5F};          ///< Saved movement speed.
        bool canKickBombs{false};   ///< Saved bomb-kick upgrade.
        bool hasRubberBombs{false}; ///< Saved rubber-bomb upgrade.
    };

    std::size_t nextId();
    Vec2 tileCenter(int row, int col) const;
    std::pair<int, int> tileForPosition(Vec2 position) const;
    [[nodiscard]] bool isInsideArena(int row, int col) const;
    [[nodiscard]] bool tileContains(EntityType type, int row, int col) const;
    [[nodiscard]] bool canEnterTile(const Character& character, int row, int col) const;
    [[nodiscard]] bool isTileDangerous(int row, int col) const;
    [[nodiscard]] bool isTileThreatenedByBomb(int row, int col, int bombRow, int bombCol, int radius) const;
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
    void createWall(EntityType type, int row, int col);
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

    std::shared_ptr<AbstractFactory> factory;              ///< Factory used to create entities.
    std::shared_ptr<Score> scoreTracker;                 ///< Score manager for the game.
    std::vector<std::shared_ptr<Entity>> entitiesList{}; ///< All entities in the arena.
    std::shared_ptr<Character> playerChar{};             ///< Player character.
    std::vector<std::shared_ptr<Character>> enemiesList{}; ///< Enemy characters.
    std::vector<BombState> bombsList{};                  ///< Active bombs.
    std::vector<ExplosionState> explosionsList{};        ///< Active explosions.
    std::vector<std::shared_ptr<PowerUp>> powerUpsList{}; ///< Active power-ups.
    std::shared_ptr<Exit> levelExit{};                   ///< Exit entity after enemies are defeated.
    std::vector<EnemyAiState> enemyAiStates{};           ///< AI state for each enemy.
    std::size_t entityCounter{1};                        ///< Counter used for unique ids.
    int numRows{13};                                     ///< Arena row count.
    int numCols{15};                                     ///< Arena column count.
    Vec2 tileDimensions{};                               ///< Size of one tile in world coordinates.
    Vec2 playerInput{};                                  ///< Current player movement input.
    float totalTime{0.0F};                               ///< Time alive in the current game.
    int remainingPlayerLives{5};                         ///< Player lives left.
    int levelNumber{1};                                  ///< Current level number.
    PlayerStats savedPlayerStats{};                      ///< Player upgrades kept between levels.
    bool hasWon{false};                                  ///< True after reaching the level exit.
};

}

#endif //BOMBERMAN_AP_LOGIC_WORLD_H
