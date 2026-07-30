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
    PowerUp
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
    Skates
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

    float getSpeed() const { return moveSpeed; }

    float getSpeedMultiplier() const { return moveSpeed / 0.55F; }

    int getBombRadius() const { return explosionRadius; }

    int getAvailableBombs() const { return maxBombs - activeBombs; }

    int getBombCapacity() const { return maxBombs; }

    void setDirection(Direction direction) { facingDir = direction; }
    void boostMovementSpeed(float amount);

    void expandExplosionRange();

    void increaseMaxBombs();

    [[nodiscard]] bool tryPlaceBomb();

    void replenishBomb();

private:
    Direction facingDir{Direction::Down};
    float moveSpeed{0.55F};
    int explosionRadius{1};
    int maxBombs{1};
    int activeBombs{0};
};

class Block final : public Entity {
public:
    Block(std::size_t id, EntityType type, Vec2 position, Vec2 size);
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
