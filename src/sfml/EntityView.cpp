#include "sfml/EntityView.h"
#include "sfml/Camera.h"

#include <SFML/Graphics/Color.hpp>

#include <algorithm>
#include <cmath>
#include <utility>

namespace bomberman::sfml {

EntityView::EntityView(std::weak_ptr<logic::Entity> entity,
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
}

void EntityView::initializeView()
{
    fallback.setOutlineThickness(2.0F);
    configureSprite();
    syncTransform();
}

void EntityView::onNotify(const logic::Event& event)
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
    } else if (usesDeathAnimation() &&
               (event.type == logic::EventType::EntityDied || event.type == logic::EventType::PlayerDamaged)) {
        deathAnimationActive = true;
        deathStart = std::chrono::steady_clock::now();
        syncTransform();
    }
}

void EntityView::draw()
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

void EntityView::syncTransform()
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
    spritePosition.y += spriteYOffset();
    sprite.setPosition(spritePosition);
    const float scaleX = pixelSize.x / spriteSourceSize.x;
    const float scaleY = pixelSize.y / spriteSourceSize.y;
    sprite.setScale((mirrored ? -scaleX : scaleX) * visualScale, scaleY * visualScale);
}

void EntityView::updateAnimation()
{
    mirrored = false;
    visualScale = 1.0F;
    sprite.setRotation(0.0F);
    sprite.setColor(sf::Color::White);
}

bool EntityView::usesDeathAnimation() const
{
    return false;
}

float EntityView::spriteYOffset() const
{
    return 0.0F;
}

float EntityView::animationSeconds() const
{
    const auto elapsed = std::chrono::steady_clock::now() - animationStart;
    return std::chrono::duration<float>(elapsed).count();
}

bool EntityView::recentlyMoved() const
{
    if (!hasLastPosition) {
        return false;
    }

    const auto elapsed = std::chrono::steady_clock::now() - lastMoveTime;
    return std::chrono::duration<float>(elapsed).count() < 0.12F;
}

CharacterView::CharacterView(std::weak_ptr<logic::Entity> entity,
                             std::shared_ptr<sf::RenderWindow> window,
                             std::shared_ptr<sf::Texture> texture,
                             std::shared_ptr<sf::Texture> tileTexture)
    : EntityView(std::move(entity), std::move(window), std::move(texture), std::move(tileTexture))
{
    initializeView();
}

void CharacterView::configureSprite()
{
    const auto targetEntity = entity.lock();
    if (targetEntity == nullptr) {
        return;
    }

    usesTexture = true;
    sprite.setTexture(*texture);
    spriteSourceSize = {24.0F, 32.0F};
    spriteOrigin = {12.0F, 16.0F};
    sprite.setTextureRect(sf::IntRect(0, 32, 24, 32));
    fallback.setFillColor(targetEntity->getType() == logic::EntityType::Enemy ? sf::Color(190, 60, 70)
                                                                               : sf::Color(245, 245, 245));
}

void CharacterView::updateAnimation()
{
    EntityView::updateAnimation();
    const auto targetEntity = entity.lock();
    if (targetEntity == nullptr) {
        return;
    }

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
        const float clampedProgress = std::min(progress, 1.0F);
        const auto alpha = static_cast<sf::Uint8>(255.0F * (1.0F - clampedProgress * 0.75F));
        visualScale = 1.0F + std::sin(clampedProgress * 3.14159F) * 0.45F;
        sprite.setRotation(std::sin(clampedProgress * 3.14159F * 2.0F) * 18.0F);
        sprite.setColor(targetEntity->getType() == logic::EntityType::Enemy ? sf::Color(255, 115, 115, alpha)
                                                                             : sf::Color(255, 255, 255, alpha));
    }
}

bool CharacterView::usesDeathAnimation() const
{
    return true;
}

float CharacterView::spriteYOffset() const
{
    return recentlyMoved() ? std::sin(animationSeconds() * 18.0F) * 2.0F : 0.0F;
}

int CharacterView::renderLayer() const
{
    return 3;
}

WallView::WallView(std::weak_ptr<logic::Entity> entity,
                     std::shared_ptr<sf::RenderWindow> window,
                     std::shared_ptr<sf::Texture> texture,
                     std::shared_ptr<sf::Texture> tileTexture)
    : EntityView(std::move(entity), std::move(window), std::move(texture), std::move(tileTexture))
{
    initializeView();
}

void WallView::configureSprite()
{
    const auto targetEntity = entity.lock();
    if (targetEntity == nullptr) {
        return;
    }

    usesTexture = true;
    sprite.setTexture(*tileTexture);
    spriteSourceSize = {16.0F, 16.0F};
    spriteOrigin = {8.0F, 8.0F};
    if (targetEntity->getType() == logic::EntityType::Wall) {
        sprite.setTextureRect(sf::IntRect(48, 48, 16, 16));
        fallback.setFillColor(sf::Color(96, 116, 132));
        fallback.setOutlineColor(sf::Color(38, 49, 59));
    } else {
        sprite.setTextureRect(sf::IntRect(80, 48, 16, 16));
        fallback.setFillColor(sf::Color(178, 112, 62));
        fallback.setOutlineColor(sf::Color(99, 56, 34));
    }
}

int WallView::renderLayer() const
{
    return 0;
}

BombView::BombView(std::weak_ptr<logic::Entity> entity,
                   std::shared_ptr<sf::RenderWindow> window,
                   std::shared_ptr<sf::Texture> texture,
                   std::shared_ptr<sf::Texture> tileTexture)
    : EntityView(std::move(entity), std::move(window), std::move(texture), std::move(tileTexture))
{
    initializeView();
}

void BombView::configureSprite()
{
    usesTexture = true;
    sprite.setTexture(*texture);
    spriteSourceSize = {32.0F, 32.0F};
    spriteOrigin = {16.0F, 16.0F};
    sprite.setTextureRect(sf::IntRect(232, 708, 32, 32));
    fallback.setFillColor(sf::Color(24, 24, 28));
}

void BombView::updateAnimation()
{
    EntityView::updateAnimation();
    const float seconds = animationSeconds();
    const auto pulse = static_cast<sf::Uint8>(190 + (static_cast<int>(seconds * 8.0F) % 2) * 65);
    sprite.setColor(sf::Color(255, pulse, pulse));
}

int BombView::renderLayer() const
{
    return 1;
}

ExplosionView::ExplosionView(std::weak_ptr<logic::Entity> entity,
                             std::shared_ptr<sf::RenderWindow> window,
                             std::shared_ptr<sf::Texture> texture,
                             std::shared_ptr<sf::Texture> tileTexture)
    : EntityView(std::move(entity), std::move(window), std::move(texture), std::move(tileTexture))
{
    initializeView();
}

void ExplosionView::configureSprite()
{
    usesTexture = true;
    sprite.setTexture(*texture);
    spriteSourceSize = {24.0F, 24.0F};
    spriteOrigin = {12.0F, 12.0F};
    sprite.setTextureRect(sf::IntRect(352, 704, 24, 24));
    fallback.setFillColor(sf::Color(255, 190, 58, 210));
    fallback.setOutlineColor(sf::Color(255, 95, 32));
}

void ExplosionView::updateAnimation()
{
    EntityView::updateAnimation();
    const float life = std::fmod(animationSeconds(), 0.65F);
    const float progress = std::min(life / 0.65F, 1.0F);
    const float peak = 1.0F - std::abs(progress * 2.0F - 1.0F);
    visualScale = 0.35F + peak * 0.95F;
    const auto alpha = static_cast<sf::Uint8>(255.0F * (1.0F - progress * 0.35F));
    sprite.setColor(sf::Color(255, 255, 255, alpha));
}

int ExplosionView::renderLayer() const
{
    return 2;
}

PowerUpView::PowerUpView(std::weak_ptr<logic::Entity> entity,
                         std::shared_ptr<sf::RenderWindow> window,
                         std::shared_ptr<sf::Texture> texture,
                         std::shared_ptr<sf::Texture> tileTexture)
    : EntityView(std::move(entity), std::move(window), std::move(texture), std::move(tileTexture))
{
    initializeView();
}

void PowerUpView::configureSprite()
{
    const auto targetEntity = entity.lock();
    if (targetEntity == nullptr) {
        return;
    }

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
}

int PowerUpView::renderLayer() const
{
    return 1;
}

ExitView::ExitView(std::weak_ptr<logic::Entity> entity,
                   std::shared_ptr<sf::RenderWindow> window,
                   std::shared_ptr<sf::Texture> texture,
                   std::shared_ptr<sf::Texture> tileTexture)
    : EntityView(std::move(entity), std::move(window), std::move(texture), std::move(tileTexture))
{
    initializeView();
}

void ExitView::configureSprite()
{
    usesTexture = true;
    sprite.setTexture(*texture);
    spriteSourceSize = {24.0F, 24.0F};
    spriteOrigin = {12.0F, 12.0F};
    sprite.setTextureRect(sf::IntRect(160, 712, 24, 24));
    fallback.setFillColor(sf::Color(90, 190, 255));
    fallback.setOutlineColor(sf::Color(210, 245, 255));
}

void ExitView::updateAnimation()
{
    EntityView::updateAnimation();
    const float pulse = 0.9F + std::sin(animationSeconds() * 6.0F) * 0.12F;
    visualScale = pulse;
    sprite.setColor(sf::Color(180, 230, 255));
}

int ExitView::renderLayer() const
{
    return 1;
}

}
