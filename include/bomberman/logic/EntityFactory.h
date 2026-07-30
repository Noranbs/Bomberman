#ifndef BOMBERMAN_AP_LOGIC_ENTITY_FACTORY_H
#define BOMBERMAN_AP_LOGIC_ENTITY_FACTORY_H

#include "bomberman/logic/Entity.h"

#include <memory>

namespace bomberman::logic {

// Abstract Factory interface for creating game entities (Abstract Factory Pattern)
class EntityFactory {
public:
    virtual ~EntityFactory() = default;

    // Creates a new character instance (Player or Enemy)
    virtual std::shared_ptr<Character> createCharacter(std::size_t id,
                                                       EntityType type,
                                                       Vec2 position,
                                                       Vec2 size) = 0;

    // Creates a new block instance (Wall, Breakable Block, Bomb, or Explosion tile)
    virtual std::shared_ptr<Block> createBlock(std::size_t id,
                                               EntityType type,
                                               Vec2 position,
                                               Vec2 size) = 0;

    // Creates a new pickable power-up item instance
    virtual std::shared_ptr<PowerUp> createPowerUp(std::size_t id,
                                                   Vec2 position,
                                                   Vec2 size,
                                                   PowerUpType powerUpType) = 0;
};

}

#endif //BOMBERMAN_AP_LOGIC_ENTITY_FACTORY_H
