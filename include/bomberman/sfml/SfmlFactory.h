#ifndef BOMBERMAN_AP_SFML_FACTORY_H
#define BOMBERMAN_AP_SFML_FACTORY_H

#include "bomberman/logic/EntityFactory.h"
#include "bomberman/sfml/SpriteView.h"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <memory>
#include <optional>
#include <vector>

namespace bomberman::sfml {

/**
 * @brief Concrete factory that creates logic entities and their SFML views.
 */
class SfmlFactory final : public logic::EntityFactory {
public:
    /**
     * @brief Creates the factory with the shared window and textures.
     * @param window Window used by created views.
     * @param texture Main sprite sheet.
     * @param tileTexture Tile sprite sheet.
     */
    SfmlFactory(std::shared_ptr<sf::RenderWindow> window,
                std::shared_ptr<sf::Texture> texture,
                std::shared_ptr<sf::Texture> tileTexture);

    /**
     * @brief Creates a character and attaches a SpriteView.
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
     * @brief Creates a block and attaches a SpriteView.
     * @param id Entity id.
     * @param type Block type.
     * @param position Block position.
     * @param size Block size.
     * @param explosionShape Optional explosion shape.
     * @return Created block.
     */
    std::shared_ptr<logic::Block> createBlock(std::size_t id,
                                               logic::EntityType type,
                                               logic::Vec2 position,
                                               logic::Vec2 size,
                                               std::optional<logic::ExplosionShape> explosionShape = std::nullopt) override;

    /**
     * @brief Creates a power-up and attaches a SpriteView.
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
    template <typename EntityT>
    void attachView(const std::shared_ptr<EntityT>& entity)
    {
        auto view = std::make_shared<SpriteView>(entity, window, texture, tileTexture);
        entity->addObserver(view);
        views.push_back(view);
    }

    std::shared_ptr<sf::RenderWindow> window; ///< Window used by all views.
    std::shared_ptr<sf::Texture> texture; ///< Main sprite sheet.
    std::shared_ptr<sf::Texture> tileTexture; ///< Tile sprite sheet.
    std::vector<std::shared_ptr<SpriteView>> views{}; ///< All active entity views.
};

}

#endif //BOMBERMAN_AP_SFML_FACTORY_H
