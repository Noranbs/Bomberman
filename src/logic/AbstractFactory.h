#ifndef BOMBERMAN_AP_LOGIC_ABSTRACT_FACTORY_H
#define BOMBERMAN_AP_LOGIC_ABSTRACT_FACTORY_H

#include "logic/Entity.h"

#include <memory>

namespace bomberman::logic {

/**
 * @brief Abstract factory used by World to create entities.
 */
class AbstractFactory {
public:
    virtual ~AbstractFactory() = default;

    /**
     * @brief Creates a player or enemy character.
     */
    virtual std::shared_ptr<Character> createCharacter(std::size_t id,
                                                       EntityType type,
                                                       Vec2 position,
                                                       Vec2 size) = 0;

    /**
     * @brief Creates an indestructible or destructible wall.
     */
    virtual std::shared_ptr<Wall> createWall(std::size_t id, EntityType type, Vec2 position, Vec2 size) = 0;

    /**
     * @brief Creates a bomb.
     */
    virtual std::shared_ptr<Bomb> createBomb(std::size_t id, Vec2 position, Vec2 size) = 0;

    /**
     * @brief Creates one explosion tile.
     */
    virtual std::shared_ptr<Explosion> createExplosion(std::size_t id,
                                                       Vec2 position,
                                                       Vec2 size,
                                                       ExplosionShape explosionShape) = 0;

    /**
     * @brief Creates the level exit.
     */
    virtual std::shared_ptr<Exit> createExit(std::size_t id, Vec2 position, Vec2 size) = 0;

    /**
     * @brief Creates a power-up that can be picked up by characters.
     */
    virtual std::shared_ptr<PowerUp> createPowerUp(std::size_t id,
                                                   Vec2 position,
                                                   Vec2 size,
                                                   PowerUpType powerUpType) = 0;
};

}

#endif //BOMBERMAN_AP_LOGIC_ABSTRACT_FACTORY_H
