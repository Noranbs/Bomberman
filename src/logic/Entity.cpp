#include "logic/Entity.h"
#include <algorithm>

namespace bomberman::logic {

void Subject::addObserver(const std::weak_ptr<Observer>& observer)
{
    observers.push_back(observer);
}

void Subject::notify(const Event& event)
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

EntityModel::EntityModel(std::size_t id, EntityType type, Vec2 position, Vec2 size)
    : entityId(id), entityType(type), pos(position), dimensions(size)
{
}

Rect EntityModel::getBounds() const
{
    return {pos, {dimensions.x * 0.5F, dimensions.y * 0.5F}};
}

bool EntityModel::blocksMovement() const
{
    return entityType == EntityType::Wall || entityType == EntityType::DestructibleBlock ||
           entityType == EntityType::Bomb;
}

std::optional<PowerUpType> EntityModel::powerUpType() const
{
    return std::nullopt;
}

std::optional<ExplosionShape> EntityModel::explosionShape() const
{
    return std::nullopt;
}

std::optional<Direction> EntityModel::facingDirection() const
{
    return std::nullopt;
}

void EntityModel::setPosition(Vec2 position)
{
    pos = position;
    notify({EventType::EntityMoved, entityId, 0});
}

void EntityModel::killEntity()
{
    if (!aliveState) {
        return;
    }
    aliveState = false;
    notify({EventType::EntityDied, entityId, 0});
}

Character::Character(std::size_t id, EntityType type, Vec2 position, Vec2 size)
    : EntityModel(id, type, position, size)
{
}

void Character::boostMovementSpeed(float amount)
{
    moveSpeed += amount;
}

void Character::reduceMovementSpeed(float amount)
{
    moveSpeed = std::max(0.1F, moveSpeed - amount);
}

void Character::freeze(float seconds)
{
    freezeTimer = std::max(freezeTimer, seconds);
}

void Character::disableBombs(float seconds)
{
    noBombTimer = std::max(noBombTimer, seconds);
}

void Character::updateStatus(float deltaTime)
{
    freezeTimer = std::max(0.0F, freezeTimer - deltaTime);
    noBombTimer = std::max(0.0F, noBombTimer - deltaTime);
}

void Character::enableBombKick()
{
    bombKick = true;
}

void Character::enableRubberBombs()
{
    rubberBombs = true;
}

void Character::resetPowerUps()
{
    moveSpeed = 0.5F;
    freezeTimer = 0.0F;
    noBombTimer = 0.0F;
    explosionRadius = 1;
    maxBombs = 1;
    activeBombs = std::min(activeBombs, maxBombs);
    bombKick = false;
    rubberBombs = false;
}

std::optional<Direction> Character::facingDirection() const
{
    return facingDir;
}

void Character::setDirection(Direction direction)
{
    if (facingDir == direction) {
        return;
    }
    facingDir = direction;
    notify({EventType::EntityMoved, getId(), 0});
}

void Character::setBombRadius(int radius)
{
    explosionRadius = std::max(1, radius);
}

void Character::setBombCapacity(int capacity)
{
    maxBombs = std::max(1, capacity);
    activeBombs = std::min(activeBombs, maxBombs);
}

void Character::setSpeed(float speed)
{
    moveSpeed = std::max(0.1F, speed);
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
    if (noBombTimer > 0.0F) {
        return false;
    }
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

Wall::Wall(std::size_t id, EntityType type, Vec2 position, Vec2 size)
    : EntityModel(id, type, position, size)
{
}

Bomb::Bomb(std::size_t id, Vec2 position, Vec2 size)
    : EntityModel(id, EntityType::Bomb, position, size)
{
}

Explosion::Explosion(std::size_t id, Vec2 position, Vec2 size, ExplosionShape explosionShape)
    : EntityModel(id, EntityType::Explosion, position, size), explosionShapeKind(explosionShape)
{
}

std::optional<ExplosionShape> Explosion::explosionShape() const
{
    return explosionShapeKind;
}

Exit::Exit(std::size_t id, Vec2 position, Vec2 size)
    : EntityModel(id, EntityType::Exit, position, size)
{
}

PowerUp::PowerUp(std::size_t id, Vec2 position, Vec2 size, PowerUpType powerUpType)
    : EntityModel(id, EntityType::PowerUp, position, size), powerUpKind(powerUpType)
{
}

std::optional<PowerUpType> PowerUp::powerUpType() const
{
    return powerUpKind;
}

}
