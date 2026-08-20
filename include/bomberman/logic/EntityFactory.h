#ifndef BOMBERMAN_AP_LOGIC_ENTITY_FACTORY_H
#define BOMBERMAN_AP_LOGIC_ENTITY_FACTORY_H

#include "bomberman/logic/Entity.h"

#include <memory>
#include <optional>

namespace bomberman::logic {

/**
 * @brief Abstract factory used by World to create entities.
 */
class EntityFactory {
public:
    virtual ~EntityFactory() = default;

    /**
     * @brief Creates a player or enemy character.
     */
    virtual std::shared_ptr<Character> createCharacter(std::size_t id,
                                                       EntityType type,
                                                       Vec2 position,
                                                       Vec2 size) = 0;

    /**
     * @brief Creates a block-like entity, such as a wall, bomb, or explosion.
     */
    virtual std::shared_ptr<Block> createBlock(std::size_t id,
                                               EntityType type,
                                               Vec2 position,
                                               Vec2 size,
                                               std::optional<ExplosionShape> explosionShape = std::nullopt) = 0;

    /**
     * @brief Creates a power-up that can be picked up by characters.
     */
    virtual std::shared_ptr<PowerUp> createPowerUp(std::size_t id,
                                                   Vec2 position,
                                                   Vec2 size,
                                                   PowerUpType powerUpType) = 0;
};

}

#endif //BOMBERMAN_AP_LOGIC_ENTITY_FACTORY_H
