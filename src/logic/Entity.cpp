#include "bomberman/logic/Entity.h"
#include <algorithm>

namespace bomberman::logic {

Entity::Entity(std::size_t id, EntityType type, Vec2 position, Vec2 size)
    : entityId(id), entityType(type), pos(position), dimensions(size)
{
}

Rect Entity::getBounds() const
{
    return {pos, {dimensions.x * 0.5F, dimensions.y * 0.5F}};
}

bool Entity::blocksMovement() const
{
    return entityType == EntityType::Wall || entityType == EntityType::DestructibleBlock ||
           entityType == EntityType::Bomb;
}

std::optional<PowerUpType> Entity::powerUpType() const
{
    return std::nullopt;
}

std::optional<ExplosionShape> Entity::explosionShape() const
{
    return std::nullopt;
}

void Entity::setPosition(Vec2 position)
{
    pos = position;
    notify({EventType::EntityMoved, entityId, 0});
}

void Entity::killEntity()
{
    if (!aliveState) {
        return;
    }
    aliveState = false;
    notify({EventType::EntityDied, entityId, 0});
}

void Entity::addObserver(const std::weak_ptr<Observer>& observer)
{
    observers.push_back(observer);
}

void Entity::notify(const Event& event)
{
    observers.erase(std::remove_if(observers.begin(),
                                   observers.end(),
                                   [&event](const std::weak_ptr<Observer>& observer) {
                                       const auto locked = observer.lock();
                                       if (locked == nullptr) {
                                           return true;
                                       }
                                       locked->onNotify(event);
                                       return false;
                                   }),
                    observers.end());
}

Character::Character(std::size_t id, EntityType type, Vec2 position, Vec2 size)
    : Entity(id, type, position, size)
{
}

void Character::boostMovementSpeed(float amount)
{
    moveSpeed += amount;
}

void Character::expandExplosionRange()
{
    ++explosionRadius;
}

void Character::increaseMaxBombs()
{
    ++maxBombs;
}

bool Character::tryPlaceBomb()
{
    if (activeBombs >= maxBombs) {
        return false;
    }
    ++activeBombs;
    return true;
}

void Character::replenishBomb()
{
    if (activeBombs > 0) {
        --activeBombs;
    }
}

Block::Block(std::size_t id, EntityType type, Vec2 position, Vec2 size)
    : Entity(id, type, position, size)
{
}

Block::Block(std::size_t id, EntityType type, Vec2 position, Vec2 size, ExplosionShape explosionShape)
    : Entity(id, type, position, size), explosionShapeKind(explosionShape)
{
}

std::optional<ExplosionShape> Block::explosionShape() const
{
    return explosionShapeKind;
}

PowerUp::PowerUp(std::size_t id, Vec2 position, Vec2 size, PowerUpType powerUpType)
    : Entity(id, EntityType::PowerUp, position, size), powerUpKind(powerUpType)
{
}

std::optional<PowerUpType> PowerUp::powerUpType() const
{
    return powerUpKind;
}

}
