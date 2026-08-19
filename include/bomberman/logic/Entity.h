#ifndef BOMBERMAN_AP_LOGIC_ENTITY_H
#define BOMBERMAN_AP_LOGIC_ENTITY_H

#include "bomberman/logic/Event.h"
#include "bomberman/logic/Geometry.h"

#include <cstddef>
#include <memory>
#include <optional>
#include <vector>

namespace bomberman::logic {

enum class EntityType {
    Player,
    Enemy,
    Wall,
    DestructibleBlock,
    Bomb,
    Explosion,
    PowerUp,
    Exit
};

enum class Direction {
    Down,
    Left,
    Right,
    Up
};

enum class PowerUpType {
    Fire,
    ExtraBomb,
    Skates,
    Stars,
    PunchGlove,
    PurpleTear,
    RedX,
    WoodenClogs,
    Skull
};

enum class ExplosionShape {
    Center,
    Horizontal,
    Vertical
};

class Entity {
public:
    Entity(std::size_t id, EntityType type, Vec2 position, Vec2 size);
    virtual ~Entity() = default;

    std::size_t getId() const { return entityId; }

    EntityType getType() const { return entityType; }

    Vec2 getPosition() const { return pos; }

    Vec2 getSize() const { return dimensions; }

    Rect getBounds() const;

    bool isAlive() const { return aliveState; }

    [[nodiscard]] virtual bool blocksMovement() const;
    [[nodiscard]] virtual std::optional<PowerUpType> powerUpType() const;
    [[nodiscard]] virtual std::optional<ExplosionShape> explosionShape() const;
    [[nodiscard]] virtual std::optional<Direction> facingDirection() const;

    void setPosition(Vec2 position);
    void killEntity();

    void addObserver(const std::weak_ptr<Observer>& observer);
    void notify(const Event& event);

private:
    std::size_t entityId{0};
    EntityType entityType{EntityType::Wall};
    Vec2 pos{};
    Vec2 dimensions{};
    bool aliveState{true};
    std::vector<std::weak_ptr<Observer>> observers{};
};

class Character final : public Entity {
public:
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

    void setDirection(Direction direction);
    void setBombRadius(int radius);
    void setBombCapacity(int capacity);
    void setSpeed(float speed);
    void boostMovementSpeed(float amount);
    void reduceMovementSpeed(float amount);
    void freeze(float seconds);
    void disableBombs(float seconds);
    void updateStatus(float deltaTime);
    void enableBombKick();
    void enableRubberBombs();
    void resetPowerUps();

    void expandExplosionRange();

    void increaseMaxBombs();

    [[nodiscard]] bool tryPlaceBomb();

    void replenishBomb();

private:
    Direction facingDir{Direction::Down};
    float moveSpeed{0.5F};
    float freezeTimer{0.0F};
    float noBombTimer{0.0F};
    int explosionRadius{1};
    int maxBombs{1};
    int activeBombs{0};
    bool bombKick{false};
    bool rubberBombs{false};
};

class Block final : public Entity {
public:
    Block(std::size_t id, EntityType type, Vec2 position, Vec2 size);
    Block(std::size_t id, EntityType type, Vec2 position, Vec2 size, ExplosionShape explosionShape);

    [[nodiscard]] std::optional<ExplosionShape> explosionShape() const override;

private:
    std::optional<ExplosionShape> explosionShapeKind{};
};

class PowerUp final : public Entity {
public:
    PowerUp(std::size_t id, Vec2 position, Vec2 size, PowerUpType powerUpType);

    [[nodiscard]] std::optional<PowerUpType> powerUpType() const override;

private:
    PowerUpType powerUpKind{PowerUpType::Fire};
};

}

#endif //BOMBERMAN_AP_LOGIC_ENTITY_H
