#include "sfml/ConcreteFactory.h"

#include <stdexcept>

namespace bomberman::sfml {

ConcreteFactory::ConcreteFactory(std::shared_ptr<sf::RenderWindow> window,
                                 std::shared_ptr<sf::Texture> texture,
                                 std::shared_ptr<sf::Texture> tileTexture)
    : window(std::move(window)), texture(std::move(texture)), tileTexture(std::move(tileTexture))
{
}

std::shared_ptr<logic::Character> ConcreteFactory::createCharacter(std::size_t id,
                                                                   logic::EntityType type,
                                                                   logic::Vec2 position,
                                                                   logic::Vec2 size)
{
    auto character = std::make_shared<logic::Character>(id, type, position, size);
    attachView(character);
    return character;
}

std::shared_ptr<logic::Wall> ConcreteFactory::createWall(std::size_t id,
                                                         logic::EntityType type,
                                                         logic::Vec2 position,
                                                         logic::Vec2 size)
{
    auto wall = std::make_shared<logic::Wall>(id, type, position, size);
    attachView(wall);
    return wall;
}

std::shared_ptr<logic::Bomb> ConcreteFactory::createBomb(std::size_t id, logic::Vec2 position, logic::Vec2 size)
{
    auto bomb = std::make_shared<logic::Bomb>(id, position, size);
    attachView(bomb);
    return bomb;
}

std::shared_ptr<logic::Explosion> ConcreteFactory::createExplosion(std::size_t id,
                                                                   logic::Vec2 position,
                                                                   logic::Vec2 size,
                                                                   logic::ExplosionShape explosionShape)
{
    auto explosion = std::make_shared<logic::Explosion>(id, position, size, explosionShape);
    attachView(explosion);
    return explosion;
}

std::shared_ptr<logic::Exit> ConcreteFactory::createExit(std::size_t id, logic::Vec2 position, logic::Vec2 size)
{
    auto exit = std::make_shared<logic::Exit>(id, position, size);
    attachView(exit);
    return exit;
}

std::shared_ptr<logic::PowerUp> ConcreteFactory::createPowerUp(std::size_t id,
                                                               logic::Vec2 position,
                                                               logic::Vec2 size,
                                                               logic::PowerUpType powerUpType)
{
    auto powerUp = std::make_shared<logic::PowerUp>(id, position, size, powerUpType);
    attachView(powerUp);
    return powerUp;
}

void ConcreteFactory::clearViews()
{
    views.clear();
}

void ConcreteFactory::drawViews()
{
    for (int layer = 0; layer <= 3; ++layer) {
        for (const auto& view : views) {
            if (view->renderLayer() == layer) {
                view->draw();
            }
        }
    }
}

void ConcreteFactory::attachView(const std::shared_ptr<logic::Entity>& entity)
{
    std::shared_ptr<EntityView> view;
    switch (entity->getType()) {
    case logic::EntityType::Player:
    case logic::EntityType::Enemy:
        view = std::make_shared<CharacterView>(entity, window, texture, tileTexture);
        break;
    case logic::EntityType::Wall:
    case logic::EntityType::DestructibleBlock:
        view = std::make_shared<WallView>(entity, window, texture, tileTexture);
        break;
    case logic::EntityType::Bomb:
        view = std::make_shared<BombView>(entity, window, texture, tileTexture);
        break;
    case logic::EntityType::Explosion:
        view = std::make_shared<ExplosionView>(entity, window, texture, tileTexture);
        break;
    case logic::EntityType::PowerUp:
        view = std::make_shared<PowerUpView>(entity, window, texture, tileTexture);
        break;
    case logic::EntityType::Exit:
        view = std::make_shared<ExitView>(entity, window, texture, tileTexture);
        break;
    }

    if (view == nullptr) {
        throw std::runtime_error("Unsupported entity type for SFML view");
    }

    entity->addObserver(view);
    views.push_back(view);
}

}
