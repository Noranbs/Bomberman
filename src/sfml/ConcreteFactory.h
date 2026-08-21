#ifndef BOMBERMAN_AP_SFML_CONCRETE_FACTORY_H
#define BOMBERMAN_AP_SFML_CONCRETE_FACTORY_H

#include "logic/AbstractFactory.h"
#include "sfml/EntityView.h"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <memory>
#include <vector>

namespace bomberman::sfml {

/**
 * @brief Concrete factory that creates logic entities and their SFML views.
 */
class ConcreteFactory final : public logic::AbstractFactory {
public:
    /**
     * @brief Creates the factory with the shared window and textures.
     * @param window Window used by created views.
     * @param texture Main sprite sheet.
     * @param tileTexture Tile sprite sheet.
     */
    ConcreteFactory(std::shared_ptr<sf::RenderWindow> window,
                    std::shared_ptr<sf::Texture> texture,
                    std::shared_ptr<sf::Texture> tileTexture);

    /**
     * @brief Creates a character and attaches a CharacterView.
     * @param id Entity id.
     * @param type Player or Enemy.
     * @param position Start position.
     * @param size Collision size.
     * @return Created character.
     */
    std::shared_ptr<logic::Character> createCharacter(std::size_t id,
                                                       logic::EntityType type,
                                                       logic::Vec2 position,
                                                       logic::Vec2 size) override;

    /**
     * @brief Creates a wall and attaches a WallView.
     */
    std::shared_ptr<logic::Wall> createWall(std::size_t id,
                                            logic::EntityType type,
                                            logic::Vec2 position,
                                            logic::Vec2 size) override;

    /**
     * @brief Creates a bomb and attaches a BombView.
     */
    std::shared_ptr<logic::Bomb> createBomb(std::size_t id, logic::Vec2 position, logic::Vec2 size) override;

    /**
     * @brief Creates an explosion and attaches an ExplosionView.
     */
    std::shared_ptr<logic::Explosion> createExplosion(std::size_t id,
                                                      logic::Vec2 position,
                                                      logic::Vec2 size,
                                                      logic::ExplosionShape explosionShape) override;

    /**
     * @brief Creates an exit and attaches an ExitView.
     */
    std::shared_ptr<logic::Exit> createExit(std::size_t id, logic::Vec2 position, logic::Vec2 size) override;

    /**
     * @brief Creates a power-up and attaches a PowerUpView.
     * @param id Entity id.
     * @param position Power-up position.
     * @param size Power-up size.
     * @param powerUpType Type of power-up.
     * @return Created power-up.
     */
    std::shared_ptr<logic::PowerUp> createPowerUp(std::size_t id,
                                                   logic::Vec2 position,
                                                   logic::Vec2 size,
                                                   logic::PowerUpType powerUpType) override;

    /**
     * @brief Removes all views when a new level or game starts.
     */
    void clearViews();

    /**
     * @brief Draws all views in layer order.
     */
    void drawViews();

private:
    void attachView(const std::shared_ptr<logic::Entity>& entity);

    std::shared_ptr<sf::RenderWindow> window; ///< Window used by all views.
    std::shared_ptr<sf::Texture> texture; ///< Main sprite sheet.
    std::shared_ptr<sf::Texture> tileTexture; ///< Tile sprite sheet.
    std::vector<std::shared_ptr<EntityView>> views{}; ///< All active entity views.
};

}

#endif //BOMBERMAN_AP_SFML_CONCRETE_FACTORY_H
