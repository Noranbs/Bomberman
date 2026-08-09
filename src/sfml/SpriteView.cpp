#include "bomberman/sfml/SpriteView.h"
#include "bomberman/sfml/Camera.h"
#include <SFML/Graphics/Color.hpp>

namespace bomberman::sfml {

SpriteView::SpriteView(std::weak_ptr<logic::Entity> entity,
                       std::shared_ptr<sf::RenderWindow> window,
                       std::shared_ptr<sf::Texture> texture)
    : entity(std::move(entity)), window(std::move(window)), texture(std::move(texture))
{
    configureSprite();
    syncTransform();
}

void SpriteView::onNotify(const logic::Event& event)
{
    if (event.type == logic::EventType::EntityMoved || event.type == logic::EventType::EntityDied) {
        syncTransform();
    }
}

void SpriteView::draw()
{
    const auto targetEntity = entity.lock();
    if (targetEntity == nullptr || !targetEntity->isAlive()) {
        return;
    }

    syncTransform();
    if (usesTexture) {
        window->draw(sprite);
    } else {
        window->draw(fallback);
    }
}

int SpriteView::renderLayer() const
{
    const auto targetEntity = entity.lock();
    if (targetEntity == nullptr) {
        return 0;
    }

    switch (targetEntity->getType()) {
    case logic::EntityType::Wall:
    case logic::EntityType::DestructibleBlock:
        return 0;
    case logic::EntityType::Bomb:
    case logic::EntityType::PowerUp:
        return 1;
    case logic::EntityType::Explosion:
        return 2;
    case logic::EntityType::Player:
    case logic::EntityType::Enemy:
        return 3;
    }

    return 0;
}

void SpriteView::syncTransform()
{
    const auto targetEntity = entity.lock();
    if (targetEntity == nullptr || window == nullptr) {
        return;
    }

    const Camera camera(window->getSize());
    const auto pixelPosition = camera.project(targetEntity->getPosition());
    const auto pixelSize = camera.projectSize(targetEntity->getSize());
    const auto fallbackSize = sf::Vector2f(pixelSize.x - 4.0F, pixelSize.y - 4.0F);

    fallback.setSize(fallbackSize);
    fallback.setOrigin(fallbackSize.x * 0.5F, fallbackSize.y * 0.5F);
    fallback.setPosition(pixelPosition);

    sprite.setOrigin(spriteOrigin);
    sprite.setPosition(pixelPosition);
    sprite.setScale(pixelSize.x / spriteSourceSize.x, pixelSize.y / spriteSourceSize.y);
}

void SpriteView::configureSprite()
{
    const auto targetEntity = entity.lock();
    if (targetEntity == nullptr) {
        return;
    }

    sprite.setTexture(*texture);
    fallback.setOutlineThickness(2.0F);

    switch (targetEntity->getType()) {
    case logic::EntityType::Player:
        usesTexture = true;
        spriteSourceSize = {24.0F, 32.0F};
        spriteOrigin = {12.0F, 16.0F};
        sprite.setTextureRect(sf::IntRect(0, 32, 24, 32));
        fallback.setFillColor(sf::Color(245, 245, 245));
        break;
    case logic::EntityType::Enemy:
        usesTexture = true;
        spriteSourceSize = {24.0F, 32.0F};
        spriteOrigin = {12.0F, 16.0F};
        sprite.setTextureRect(sf::IntRect(0, 32, 24, 32));
        sprite.setColor(sf::Color(255, 115, 115));
        fallback.setFillColor(sf::Color(190, 60, 70));
        break;
    case logic::EntityType::Wall:
        fallback.setFillColor(sf::Color(96, 116, 132));
        fallback.setOutlineColor(sf::Color(38, 49, 59));
        break;
    case logic::EntityType::DestructibleBlock:
        fallback.setFillColor(sf::Color(178, 112, 62));
        fallback.setOutlineColor(sf::Color(99, 56, 34));
        break;
    case logic::EntityType::Bomb:
        usesTexture = true;
        spriteSourceSize = {32.0F, 32.0F};
        spriteOrigin = {16.0F, 16.0F};
        sprite.setTextureRect(sf::IntRect(232, 708, 32, 32));
        fallback.setFillColor(sf::Color(24, 24, 28));
        break;
    case logic::EntityType::Explosion:
        usesTexture = true;
        spriteSourceSize = {24.0F, 24.0F};
        spriteOrigin = {12.0F, 12.0F};
        sprite.setTextureRect(sf::IntRect(352, 704, 24, 24));
        fallback.setFillColor(sf::Color(255, 190, 58, 210));
        fallback.setOutlineColor(sf::Color(255, 95, 32));
        break;
    case logic::EntityType::PowerUp:
        usesTexture = true;
        spriteSourceSize = {24.0F, 24.0F};
        spriteOrigin = {12.0F, 12.0F};
        if (const auto powerUpType = targetEntity->powerUpType(); powerUpType.has_value()) {
            switch (*powerUpType) {
            case logic::PowerUpType::Fire:
                sprite.setTextureRect(sf::IntRect(128, 712, 24, 24));
                break;
            case logic::PowerUpType::ExtraBomb:
                sprite.setTextureRect(sf::IntRect(32, 712, 24, 24));
                break;
            case logic::PowerUpType::Skates:
                sprite.setTextureRect(sf::IntRect(64, 712, 24, 24));
                break;
            }
        }
        fallback.setFillColor(sf::Color(230, 210, 80));
        break;
    }
}

}
