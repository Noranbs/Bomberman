#include "bomberman/sfml/SpriteView.h"
#include "bomberman/sfml/Camera.h"
#include <SFML/Graphics/Color.hpp>

#include <algorithm>
#include <array>
#include <cmath>

namespace bomberman::sfml {

SpriteView::SpriteView(std::weak_ptr<logic::Entity> entity,
                       std::shared_ptr<sf::RenderWindow> window,
                       std::shared_ptr<sf::Texture> texture,
                       std::shared_ptr<sf::Texture> tileTexture)
    : entity(std::move(entity)),
      window(std::move(window)),
      texture(std::move(texture)),
      tileTexture(std::move(tileTexture))
{
    if (const auto targetEntity = this->entity.lock(); targetEntity != nullptr) {
        lastPosition = targetEntity->getPosition();
        hasLastPosition = true;
    }
    configureSprite();
    syncTransform();
}

void SpriteView::onNotify(const logic::Event& event)
{
    if (event.type == logic::EventType::EntityMoved) {
        const auto targetEntity = entity.lock();
        if (targetEntity != nullptr) {
            const auto position = targetEntity->getPosition();
            const bool changedPosition = !hasLastPosition ||
                                         std::abs(position.x - lastPosition.x) > 0.0001F ||
                                         std::abs(position.y - lastPosition.y) > 0.0001F;
            if (changedPosition) {
                lastPosition = position;
                hasLastPosition = true;
                lastMoveTime = std::chrono::steady_clock::now();
            }
        }
        syncTransform();
    } else if (event.type == logic::EventType::EntityDied) {
        const auto targetEntity = entity.lock();
        if (targetEntity != nullptr &&
            (targetEntity->getType() == logic::EntityType::Player || targetEntity->getType() == logic::EntityType::Enemy)) {
            deathAnimationActive = true;
            deathStart = std::chrono::steady_clock::now();
        }
        syncTransform();
    }
}

void SpriteView::draw()
{
    const auto targetEntity = entity.lock();
    if (targetEntity == nullptr) {
        return;
    }

    if (!targetEntity->isAlive() && !deathAnimationActive) {
        return;
    }

    if (deathAnimationActive) {
        const auto elapsed = std::chrono::steady_clock::now() - deathStart;
        if (std::chrono::duration<float>(elapsed).count() > 0.65F) {
            deathAnimationActive = false;
            return;
        }
    } else if (!targetEntity->isAlive()) {
        return;
    }

    updateAnimation();
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
    case logic::EntityType::Exit:
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
    auto spritePosition = pixelPosition;
    const auto targetType = targetEntity->getType();
    if ((targetType == logic::EntityType::Player || targetType == logic::EntityType::Enemy) && recentlyMoved()) {
        spritePosition.y += std::sin(animationSeconds() * 18.0F) * 2.0F;
    }
    sprite.setPosition(spritePosition);
    const float scaleX = pixelSize.x / spriteSourceSize.x;
    const float scaleY = pixelSize.y / spriteSourceSize.y;
    sprite.setScale((mirrored ? -scaleX : scaleX) * visualScale, scaleY * visualScale);
}

void SpriteView::configureSprite()
{
    const auto targetEntity = entity.lock();
    if (targetEntity == nullptr) {
        return;
    }

    fallback.setOutlineThickness(2.0F);

    switch (targetEntity->getType()) {
    case logic::EntityType::Player:
        usesTexture = true;
        sprite.setTexture(*texture);
        spriteSourceSize = {24.0F, 32.0F};
        spriteOrigin = {12.0F, 16.0F};
        sprite.setTextureRect(sf::IntRect(0, 32, 24, 32));
        fallback.setFillColor(sf::Color(245, 245, 245));
        break;
    case logic::EntityType::Enemy:
        usesTexture = true;
        sprite.setTexture(*texture);
        spriteSourceSize = {24.0F, 32.0F};
        spriteOrigin = {12.0F, 16.0F};
        sprite.setTextureRect(sf::IntRect(0, 32, 24, 32));
        sprite.setColor(sf::Color(255, 115, 115));
        fallback.setFillColor(sf::Color(190, 60, 70));
        break;
    case logic::EntityType::Wall:
        usesTexture = true;
        sprite.setTexture(*tileTexture);
        spriteSourceSize = {16.0F, 16.0F};
        spriteOrigin = {8.0F, 8.0F};
        sprite.setTextureRect(sf::IntRect(48, 48, 16, 16));
        fallback.setFillColor(sf::Color(96, 116, 132));
        fallback.setOutlineColor(sf::Color(38, 49, 59));
        break;
    case logic::EntityType::DestructibleBlock:
        usesTexture = true;
        sprite.setTexture(*tileTexture);
        spriteSourceSize = {16.0F, 16.0F};
        spriteOrigin = {8.0F, 8.0F};
        sprite.setTextureRect(sf::IntRect(80, 48, 16, 16));
        fallback.setFillColor(sf::Color(178, 112, 62));
        fallback.setOutlineColor(sf::Color(99, 56, 34));
        break;
    case logic::EntityType::Bomb:
        usesTexture = true;
        sprite.setTexture(*texture);
        spriteSourceSize = {32.0F, 32.0F};
        spriteOrigin = {16.0F, 16.0F};
        sprite.setTextureRect(sf::IntRect(232, 708, 32, 32));
        fallback.setFillColor(sf::Color(24, 24, 28));
        break;
    case logic::EntityType::Explosion:
        usesTexture = true;
        sprite.setTexture(*texture);
        spriteSourceSize = {24.0F, 24.0F};
        spriteOrigin = {12.0F, 12.0F};
        sprite.setTextureRect(sf::IntRect(352, 704, 24, 24));
        fallback.setFillColor(sf::Color(255, 190, 58, 210));
        fallback.setOutlineColor(sf::Color(255, 95, 32));
        break;
    case logic::EntityType::PowerUp:
        usesTexture = true;
        sprite.setTexture(*texture);
        spriteSourceSize = {24.0F, 24.0F};
        spriteOrigin = {12.0F, 12.0F};
        if (const auto powerUpType = targetEntity->powerUpType(); powerUpType.has_value()) {
            switch (*powerUpType) {
            case logic::PowerUpType::Fire:
                sprite.setTextureRect(sf::IntRect(0, 712, 24, 24));
                break;
            case logic::PowerUpType::ExtraBomb:
                sprite.setTextureRect(sf::IntRect(32, 712, 24, 24));
                break;
            case logic::PowerUpType::Skates:
                sprite.setTextureRect(sf::IntRect(64, 712, 24, 24));
                break;
            case logic::PowerUpType::Stars:
                sprite.setTextureRect(sf::IntRect(128, 712, 24, 24));
                break;
            case logic::PowerUpType::BlueGhost:
                sprite.setTextureRect(sf::IntRect(64, 768, 24, 24));
                break;
            case logic::PowerUpType::PunchGlove:
                sprite.setTextureRect(sf::IntRect(64, 740, 24, 24));
                break;
            case logic::PowerUpType::PurpleTear:
                sprite.setTextureRect(sf::IntRect(96, 712, 24, 24));
                break;
            case logic::PowerUpType::RedX:
                sprite.setTextureRect(sf::IntRect(96, 768, 24, 24));
                break;
            case logic::PowerUpType::WoodenClogs:
                sprite.setTextureRect(sf::IntRect(0, 740, 24, 24));
                break;
            case logic::PowerUpType::Skull:
                sprite.setTextureRect(sf::IntRect(128, 768, 24, 24));
                break;
            }
        }
        fallback.setFillColor(sf::Color(230, 210, 80));
        break;
    case logic::EntityType::Exit:
        usesTexture = true;
        sprite.setTexture(*texture);
        spriteSourceSize = {24.0F, 24.0F};
        spriteOrigin = {12.0F, 12.0F};
        sprite.setTextureRect(sf::IntRect(160, 712, 24, 24));
        fallback.setFillColor(sf::Color(90, 190, 255));
        fallback.setOutlineColor(sf::Color(210, 245, 255));
        break;
    }
}

void SpriteView::updateAnimation()
{
    const auto targetEntity = entity.lock();
    if (targetEntity == nullptr) {
        return;
    }

    mirrored = false;
    visualScale = 1.0F;
    sprite.setColor(sf::Color::White);

    const float seconds = animationSeconds();
    switch (targetEntity->getType()) {
    case logic::EntityType::Player:
    case logic::EntityType::Enemy: {
        int row = 32;

        if (const auto direction = targetEntity->facingDirection(); direction.has_value()) {
            switch (*direction) {
            case logic::Direction::Down:
                row = 32;
                break;
            case logic::Direction::Up:
                row = 64;
                break;
            case logic::Direction::Right:
                row = 32;
                sprite.setTextureRect(sf::IntRect(72, row, 24, 32));
                break;
            case logic::Direction::Left:
                row = 32;
                mirrored = true;
                sprite.setTextureRect(sf::IntRect(72, row, 24, 32));
                break;
            }
        } else {
            sprite.setTextureRect(sf::IntRect(0, row, 24, 32));
        }

        if (const auto direction = targetEntity->facingDirection();
            !direction.has_value() || (*direction != logic::Direction::Left && *direction != logic::Direction::Right)) {
            sprite.setTextureRect(sf::IntRect(0, row, 24, 32));
        }
        if (targetEntity->getType() == logic::EntityType::Enemy) {
            sprite.setColor(sf::Color(255, 115, 115));
        }
        if (deathAnimationActive) {
            const auto elapsed = std::chrono::steady_clock::now() - deathStart;
            const float progress = std::chrono::duration<float>(elapsed).count() / 0.65F;
            const auto alpha = static_cast<sf::Uint8>(255.0F * (1.0F - std::min(progress, 1.0F)));
            visualScale = 1.0F + progress * 0.35F;
            sprite.setColor(targetEntity->getType() == logic::EntityType::Enemy
                                ? sf::Color(255, 115, 115, alpha)
                                : sf::Color(255, 255, 255, alpha));
        }
        break;
    }
    case logic::EntityType::Bomb: {
        const auto pulse = static_cast<sf::Uint8>(190 + (static_cast<int>(seconds * 8.0F) % 2) * 65);
        sprite.setColor(sf::Color(255, pulse, pulse));
        break;
    }
    case logic::EntityType::Explosion: {
        const float life = std::fmod(seconds, 0.65F);
        const float progress = std::min(life / 0.65F, 1.0F);
        const float peak = 1.0F - std::abs(progress * 2.0F - 1.0F);
        visualScale = 0.35F + peak * 0.95F;
        const auto alpha = static_cast<sf::Uint8>(255.0F * (1.0F - progress * 0.35F));
        sprite.setColor(sf::Color(255, 255, 255, alpha));
        break;
    }
    case logic::EntityType::Wall:
    case logic::EntityType::DestructibleBlock:
    case logic::EntityType::PowerUp:
    case logic::EntityType::Exit:
        if (targetEntity->getType() == logic::EntityType::Exit) {
            const float pulse = 0.9F + std::sin(seconds * 6.0F) * 0.12F;
            visualScale = pulse;
            sprite.setColor(sf::Color(180, 230, 255));
        }
        break;
    }
}

float SpriteView::animationSeconds() const
{
    const auto elapsed = std::chrono::steady_clock::now() - animationStart;
    return std::chrono::duration<float>(elapsed).count();
}

bool SpriteView::recentlyMoved() const
{
    if (!hasLastPosition) {
        return false;
    }

    const auto elapsed = std::chrono::steady_clock::now() - lastMoveTime;
    return std::chrono::duration<float>(elapsed).count() < 0.12F;
}

}
