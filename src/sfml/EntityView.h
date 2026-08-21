#ifndef BOMBERMAN_AP_SFML_ENTITY_VIEW_H
#define BOMBERMAN_AP_SFML_ENTITY_VIEW_H

#include "logic/Entity.h"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <chrono>
#include <memory>

namespace bomberman::sfml {

/**
 * @brief Abstract SFML view for one logic entity.
 *
 * EntityView observes a logic entity and owns the shared drawing state.
 * Concrete views decide which sprite, layer, and animation to use.
 */
class EntityView : public logic::Observer {
public:
    /**
     * @brief Creates a view for one entity.
     * @param entity Logic entity to observe and draw.
     * @param window Window where the sprite is drawn.
     * @param texture Main sprite sheet.
     * @param tileTexture Sprite sheet for tiles.
     */
    EntityView(std::weak_ptr<logic::Entity> entity,
               std::shared_ptr<sf::RenderWindow> window,
               std::shared_ptr<sf::Texture> texture,
               std::shared_ptr<sf::Texture> tileTexture);
    ~EntityView() override = default;

    /**
     * @brief Reacts to movement and death events from the entity.
     * @param event Event sent by the observed entity.
     */
    void onNotify(const logic::Event& event) override;

    /**
     * @brief Draws the entity if it should be visible.
     */
    void draw();

    /**
     * @brief Returns the drawing layer used for sorting.
     */
    [[nodiscard]] virtual int renderLayer() const = 0;

protected:
    void initializeView();
    void syncTransform();
    virtual void configureSprite() = 0;
    virtual void updateAnimation();
    [[nodiscard]] virtual bool usesDeathAnimation() const;
    [[nodiscard]] virtual float spriteYOffset() const;
    [[nodiscard]] float animationSeconds() const;
    [[nodiscard]] bool recentlyMoved() const;

    std::weak_ptr<logic::Entity> entity; ///< Entity drawn by this view.
    std::shared_ptr<sf::RenderWindow> window; ///< Window used for drawing.
    std::shared_ptr<sf::Texture> texture; ///< Main sprite sheet.
    std::shared_ptr<sf::Texture> tileTexture; ///< Tile sprite sheet.
    sf::Sprite sprite; ///< Sprite used when a texture is available.
    sf::RectangleShape fallback; ///< Simple shape used as backup drawing.
    sf::Vector2f spriteSourceSize{24.0F, 24.0F}; ///< Size of the selected sprite frame.
    sf::Vector2f spriteOrigin{12.0F, 12.0F}; ///< Origin point of the sprite frame.
    logic::Vec2 lastPosition{}; ///< Last known entity position.
    std::chrono::steady_clock::time_point animationStart{std::chrono::steady_clock::now()}; ///< Animation start time.
    std::chrono::steady_clock::time_point lastMoveTime{}; ///< Last time the entity moved.
    std::chrono::steady_clock::time_point deathStart{}; ///< Start time of the death animation.
    float visualScale{1.0F}; ///< Extra scale used for animations.
    bool usesTexture{false}; ///< True when drawing a sprite instead of fallback.
    bool hasLastPosition{false}; ///< True after lastPosition is initialized.
    bool deathAnimationActive{false}; ///< True while death animation is playing.
    bool mirrored{false}; ///< True when the sprite is flipped horizontally.
};

/**
 * @brief View for player and enemy characters.
 */
class CharacterView final : public EntityView {
public:
    CharacterView(std::weak_ptr<logic::Entity> entity,
                  std::shared_ptr<sf::RenderWindow> window,
                  std::shared_ptr<sf::Texture> texture,
                  std::shared_ptr<sf::Texture> tileTexture);

    void configureSprite() override;
    void updateAnimation() override;
    [[nodiscard]] bool usesDeathAnimation() const override;
    [[nodiscard]] float spriteYOffset() const override;
    [[nodiscard]] int renderLayer() const override;
};

/**
 * @brief View for indestructible and destructible arena blocks.
 */
class WallView final : public EntityView {
public:
    WallView(std::weak_ptr<logic::Entity> entity,
              std::shared_ptr<sf::RenderWindow> window,
              std::shared_ptr<sf::Texture> texture,
              std::shared_ptr<sf::Texture> tileTexture);

    void configureSprite() override;
    [[nodiscard]] int renderLayer() const override;
};

/**
 * @brief View for bombs.
 */
class BombView final : public EntityView {
public:
    BombView(std::weak_ptr<logic::Entity> entity,
             std::shared_ptr<sf::RenderWindow> window,
             std::shared_ptr<sf::Texture> texture,
             std::shared_ptr<sf::Texture> tileTexture);

    void configureSprite() override;
    void updateAnimation() override;
    [[nodiscard]] int renderLayer() const override;
};

/**
 * @brief View for explosion tiles.
 */
class ExplosionView final : public EntityView {
public:
    ExplosionView(std::weak_ptr<logic::Entity> entity,
                  std::shared_ptr<sf::RenderWindow> window,
                  std::shared_ptr<sf::Texture> texture,
                  std::shared_ptr<sf::Texture> tileTexture);

    void configureSprite() override;
    void updateAnimation() override;
    [[nodiscard]] int renderLayer() const override;
};

/**
 * @brief View for power-up and power-down items.
 */
class PowerUpView final : public EntityView {
public:
    PowerUpView(std::weak_ptr<logic::Entity> entity,
                std::shared_ptr<sf::RenderWindow> window,
                std::shared_ptr<sf::Texture> texture,
                std::shared_ptr<sf::Texture> tileTexture);

    void configureSprite() override;
    [[nodiscard]] int renderLayer() const override;
};

/**
 * @brief View for the level exit.
 */
class ExitView final : public EntityView {
public:
    ExitView(std::weak_ptr<logic::Entity> entity,
             std::shared_ptr<sf::RenderWindow> window,
             std::shared_ptr<sf::Texture> texture,
             std::shared_ptr<sf::Texture> tileTexture);

    void configureSprite() override;
    void updateAnimation() override;
    [[nodiscard]] int renderLayer() const override;
};

}

#endif //BOMBERMAN_AP_SFML_ENTITY_VIEW_H
