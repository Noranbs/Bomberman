#include "bomberman/sfml/SfmlFactory.h"

namespace bomberman::sfml {

SfmlFactory::SfmlFactory(std::shared_ptr<sf::RenderWindow> window, std::shared_ptr<sf::Texture> texture)
    : window(std::move(window)), texture(std::move(texture))
{
}

std::shared_ptr<logic::Character> SfmlFactory::createCharacter(std::size_t id,
                                                               logic::EntityType type,
                                                               logic::Vec2 position,
                                                               logic::Vec2 size)
{
    auto character = std::make_shared<logic::Character>(id, type, position, size);
    attachView(character);
    return character;
}

std::shared_ptr<logic::Block> SfmlFactory::createBlock(std::size_t id,
                                                       logic::EntityType type,
                                                       logic::Vec2 position,
                                                       logic::Vec2 size)
{
    auto block = std::make_shared<logic::Block>(id, type, position, size);
    attachView(block);
    return block;
}

std::shared_ptr<logic::PowerUp> SfmlFactory::createPowerUp(std::size_t id,
                                                           logic::Vec2 position,
                                                           logic::Vec2 size,
                                                           logic::PowerUpType powerUpType)
{
    auto powerUp = std::make_shared<logic::PowerUp>(id, position, size, powerUpType);
    attachView(powerUp);
    return powerUp;
}

void SfmlFactory::clearViews()
{
    views.clear();
}

void SfmlFactory::drawViews()
{
    for (int layer = 0; layer <= 3; ++layer) {
        for (const auto& view : views) {
            if (view->renderLayer() == layer) {
                view->draw();
            }
        }
    }
}

}
