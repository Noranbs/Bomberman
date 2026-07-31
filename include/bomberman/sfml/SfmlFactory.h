#ifndef BOMBERMAN_AP_SFML_FACTORY_H
#define BOMBERMAN_AP_SFML_FACTORY_H

#include "bomberman/logic/EntityFactory.h"
#include "bomberman/sfml/SpriteView.h"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <memory>
#include <vector>

namespace bomberman::sfml {

class SfmlFactory final : public logic::EntityFactory {
public:
    SfmlFactory(std::shared_ptr<sf::RenderWindow> window, std::shared_ptr<sf::Texture> texture);

    std::shared_ptr<logic::Character> createCharacter(std::size_t id,
                                                       logic::EntityType type,
                                                       logic::Vec2 position,
                                                       logic::Vec2 size) override;
    std::shared_ptr<logic::Block> createBlock(std::size_t id,
                                               logic::EntityType type,
                                               logic::Vec2 position,
                                               logic::Vec2 size) override;
    std::shared_ptr<logic::PowerUp> createPowerUp(std::size_t id,
                                                   logic::Vec2 position,
                                                   logic::Vec2 size,
                                                   logic::PowerUpType powerUpType) override;

    void clearViews();
    void drawViews();

private:
    template <typename EntityT>
    void attachView(const std::shared_ptr<EntityT>& entity)
    {
        auto view = std::make_shared<SpriteView>(entity, window, texture);
        entity->addObserver(view);
        views.push_back(view);
    }

    std::shared_ptr<sf::RenderWindow> window;
    std::shared_ptr<sf::Texture> texture;
    std::vector<std::shared_ptr<SpriteView>> views{};
};

}

#endif //BOMBERMAN_AP_SFML_FACTORY_H
