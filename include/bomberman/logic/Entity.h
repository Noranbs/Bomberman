#ifndef BOMBERMAN_AP_LOGIC_ENTITY_H
#define BOMBERMAN_AP_LOGIC_ENTITY_H

#include "bomberman/logic/Event.h"
#include "bomberman/logic/Geometry.h"

#include <cstddef>
#include <memory>
#include <optional>
#include <vector>

namespace bomberman::logic {

/**
 * @brief All entity kinds that can exist in the world.
 */
enum class EntityType {
    Player,             ///< Human controlled character.
    Enemy,              ///< Computer controlled character.
    Wall,               ///< Indestructible block.
    DestructibleBlock,  ///< Block that bombs can destroy.
    Bomb,               ///< Bomb placed by a character.
    Explosion,          ///< Temporary explosion tile.
    PowerUp,            ///< Pickable item.
    Exit                ///< Exit to the next level.
};

/**
 * @brief Direction used for character movement and animations.
 */
enum class Direction {
    Down,   ///< Facing down.
    Left,   ///< Facing left.
    Right,  ///< Facing right.
    Up      ///< Facing up.
};

/**
 * @brief All power-up and power-down item types.
 */
enum class PowerUpType {
    Fire,         ///< Increases bomb radius.
    ExtraBomb,    ///< Increases bomb capacity.
    Skates,       ///< Increases movement speed.
    Stars,        ///< Gives bonus score.
    PunchGlove,   ///< Allows bomb kicking.
    PurpleTear,   ///< Makes kicked bombs bounce.
    RedX,         ///< Blocks bomb placement for a short time.
    WoodenClogs,  ///< Lowers movement speed.
    Skull         ///< Harmful item.
};

/**
 * @brief Shape hint used by explosion tiles.
 */
enum class ExplosionShape {
    Center,      ///< Center tile of the explosion.
    Horizontal,  ///< Horizontal explosion tile.
    Vertical     ///< Vertical explosion tile.
};

/**
 * @brief Base class for every object in the logic world.
 */
class Entity {
public:
    /**
     * @brief Creates an entity with an id, type, position, and size.
     * @param id Unique entity id.
     * @param type Kind of entity.
     * @param position Center position in world coordinates.
     * @param size Size in world coordinates.
     */
    Entity(std::size_t id, EntityType type, Vec2 position, Vec2 size);
    virtual ~Entity() = default;

    std::size_t getId() const { return entityId; }

    EntityType getType() const { return entityType; }

    Vec2 getPosition() const { return pos; }

    Vec2 getSize() const { return dimensions; }

    /**
     * @brief Returns the rectangle used for collision checks.
     */
    Rect getBounds() const;

    bool isAlive() const { return aliveState; }

    /**
     * @brief Says if this entity blocks movement.
     */
    [[nodiscard]] virtual bool blocksMovement() const;

    /**
     * @brief Returns the power-up type if this entity is a power-up.
     */
    [[nodiscard]] virtual std::optional<PowerUpType> powerUpType() const;

    /**
     * @brief Returns the explosion shape if this entity is an explosion tile.
     */
    [[nodiscard]] virtual std::optional<ExplosionShape> explosionShape() const;

    /**
     * @brief Returns the facing direction if this entity is a character.
     */
    [[nodiscard]] virtual std::optional<Direction> facingDirection() const;

    /**
     * @brief Moves the entity and notifies observers.
     * @param position New center position.
     */
    void setPosition(Vec2 position);

    /**
     * @brief Marks the entity as dead and notifies observers.
     */
    void killEntity();

    /**
     * @brief Adds an observer that listens to this entity.
     * @param observer Observer stored as weak pointer.
     */
    void addObserver(const std::weak_ptr<Observer>& observer);

    /**
     * @brief Sends an event to all observers.
     * @param event Event to send.
     */
    void notify(const Event& event);

private:
    std::size_t entityId{0};                     ///< Unique id of the entity.
    EntityType entityType{EntityType::Wall};     ///< Type of the entity.
    Vec2 pos{};                                  ///< Center position in the world.
    Vec2 dimensions{};                           ///< Size in the world.
    bool aliveState{true};                       ///< False when the entity is dead.
    std::vector<std::weak_ptr<Observer>> observers{}; ///< Objects listening to this entity.
};

/**
 * @brief A moving player or enemy.
 */
class Character final : public Entity {
public:
    /**
     * @brief Creates a player or enemy character.
     * @param id Unique entity id.
     * @param type Player or Enemy.
     * @param position Start position.
     * @param size Collision size.
     */
    Character(std::size_t id, EntityType type, Vec2 position, Vec2 size);

    Direction getDirection() const { return facingDir; }
    [[nodiscard]] std::optional<Direction> facingDirection() const override;

    float getSpeed() const { return moveSpeed; }

    float getSpeedMultiplier() const { return moveSpeed; }
    bool isFrozen() const { return freezeTimer > 0.0F; }
    bool canKickBombs() const { return bombKick; }
    bool hasRubberBombs() const { return rubberBombs; }

    int getBombRadius() const { return explosionRadius; }

    int getAvailableBombs() const { return maxBombs - activeBombs; }

    int getBombCapacity() const { return maxBombs; }

    /**
     * @brief Sets the direction used for movement and animation.
     * @param direction New facing direction.
     */
    void setDirection(Direction direction);

    /**
     * @brief Sets the bomb explosion radius.
     * @param radius New radius in tiles.
     */
    void setBombRadius(int radius);

    /**
     * @brief Sets how many bombs can be active at the same time.
     * @param capacity New bomb capacity.
     */
    void setBombCapacity(int capacity);

    /**
     * @brief Sets the movement speed.
     * @param speed New speed value.
     */
    void setSpeed(float speed);

    /**
     * @brief Increases movement speed.
     * @param amount Speed amount to add.
     */
    void boostMovementSpeed(float amount);

    /**
     * @brief Decreases movement speed but keeps a minimum value.
     * @param amount Speed amount to remove.
     */
    void reduceMovementSpeed(float amount);

    /**
     * @brief Stops the character from moving for a short time.
     * @param seconds Freeze duration.
     */
    void freeze(float seconds);

    /**
     * @brief Stops the character from placing bombs for a short time.
     * @param seconds No bomb duration.
     */
    void disableBombs(float seconds);

    /**
     * @brief Updates temporary status effects.
     * @param deltaTime Time since the last update.
     */
    void updateStatus(float deltaTime);

    /**
     * @brief Allows the character to kick bombs.
     */
    void enableBombKick();

    /**
     * @brief Allows kicked bombs to bounce back.
     */
    void enableRubberBombs();

    /**
     * @brief Resets all power-up effects to the default values.
     */
    void resetPowerUps();

    /**
     * @brief Increases bomb range by one.
     */
    void expandExplosionRange();

    /**
     * @brief Increases bomb capacity by one.
     */
    void increaseMaxBombs();

    /**
     * @brief Tries to reserve one bomb slot.
     */
    [[nodiscard]] bool tryPlaceBomb();

    /**
     * @brief Gives one bomb slot back after a bomb explodes.
     */
    void replenishBomb();

private:
    Direction facingDir{Direction::Down}; ///< Direction used for movement and sprite.
    float moveSpeed{0.5F};                ///< Current movement speed.
    float freezeTimer{0.0F};              ///< Time left where movement is blocked.
    float noBombTimer{0.0F};              ///< Time left where bomb placement is blocked.
    int explosionRadius{1};               ///< Bomb radius in tiles.
    int maxBombs{1};                      ///< Maximum active bombs.
    int activeBombs{0};                   ///< Bombs currently placed by this character.
    bool bombKick{false};                 ///< True if this character can kick bombs.
    bool rubberBombs{false};              ///< True if kicked bombs bounce.
};

/**
 * @brief Entity used for walls, bombs, explosions, and the exit.
 */
class Block final : public Entity {
public:
    /**
     * @brief Creates a block without an explosion shape.
     * @param id Unique entity id.
     * @param type Block entity type.
     * @param position Center position.
     * @param size Block size.
     */
    Block(std::size_t id, EntityType type, Vec2 position, Vec2 size);

    /**
     * @brief Creates a block with an explosion shape.
     * @param id Unique entity id.
     * @param type Block entity type.
     * @param position Center position.
     * @param size Block size.
     * @param explosionShape Shape used for drawing explosions.
     */
    Block(std::size_t id, EntityType type, Vec2 position, Vec2 size, ExplosionShape explosionShape);

    [[nodiscard]] std::optional<ExplosionShape> explosionShape() const override;

private:
    std::optional<ExplosionShape> explosionShapeKind{}; ///< Optional shape for explosion blocks.
};

/**
 * @brief Pickable item that changes character stats or score.
 */
class PowerUp final : public Entity {
public:
    /**
     * @brief Creates a power-up item.
     * @param id Unique entity id.
     * @param position Center position.
     * @param size Collision size.
     * @param powerUpType Type of item.
     */
    PowerUp(std::size_t id, Vec2 position, Vec2 size, PowerUpType powerUpType);

    [[nodiscard]] std::optional<PowerUpType> powerUpType() const override;

private:
    PowerUpType powerUpKind{PowerUpType::Fire}; ///< Type of this power-up.
};

}

#endif //BOMBERMAN_AP_LOGIC_ENTITY_H
